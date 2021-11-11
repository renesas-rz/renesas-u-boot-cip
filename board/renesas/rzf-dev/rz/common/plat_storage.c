/*
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <lib/mmio.h>
#include <tools_share/firmware_image_package.h>

#include <rzg2l_def.h>
#include <sys.h>
#include <spi_multi.h>

static uintptr_t memdrv_dev_handle;
static uintptr_t fip_dev_handle;

static uintptr_t boot_io_drv_id;

static const io_block_spec_t rzg2l_block_spec = {
	.offset = RZG2L_SPIROM_FIP_BASE,
	.length = RZG2L_SPIROM_FIP_SIZE,
};

static const io_uuid_spec_t bl31_file_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl32_file_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t bl33_file_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t soc_fw_key_cert_file_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_content_cert_file_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t tos_fw_key_cert_file_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t tos_fw_content_cert_file_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t nt_fw_key_cert_file_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t nt_fw_content_cert_file_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif

static int32_t open_memmap(const uintptr_t spec);
static int32_t open_fipdrv(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int32_t (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
			  &memdrv_dev_handle,
			  (uintptr_t) &rzg2l_block_spec,
			  &open_memmap},
	[BL31_IMAGE_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &bl31_file_spec,
			   &open_fipdrv},
	[BL32_IMAGE_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &bl32_file_spec,
			   &open_fipdrv},
	[BL33_IMAGE_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &bl33_file_spec,
			   &open_fipdrv},
#if TRUSTED_BOARD_BOOT
	[SOC_FW_KEY_CERT_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &soc_fw_key_cert_file_spec,
			   &open_fipdrv},
	[SOC_FW_CONTENT_CERT_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &soc_fw_content_cert_file_spec,
			   &open_fipdrv},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &tos_fw_key_cert_file_spec,
			   &open_fipdrv},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &tos_fw_content_cert_file_spec,
			   &open_fipdrv},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &nt_fw_key_cert_file_spec,
			   &open_fipdrv},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
			   &fip_dev_handle,
			   (uintptr_t) &nt_fw_content_cert_file_spec,
			   &open_fipdrv},
#endif
	{ 0, 0, 0}
};

static int32_t open_fipdrv(const uintptr_t spec)
{
	int32_t result;

	result = io_dev_init(fip_dev_handle, boot_io_drv_id);
	if (result != 0)
		return result;

	return result;
}

static int32_t open_memmap(const uintptr_t spec)
{
	uintptr_t handle;
	int32_t result;

	result = io_dev_init(memdrv_dev_handle, 0);
	if (result != 0)
		return result;

	result = io_open(memdrv_dev_handle, spec, &handle);
	if (result == 0)
		io_close(handle);

	return result;
}

void rz_io_setup(void)
{
	const io_dev_connector_t *memmap;
	const io_dev_connector_t *rzg2l;
	uint16_t boot_dev;

	boot_dev = *((uint16_t *)RZG2L_BOOTINFO_BASE) & MASK_BOOTM_DEVICE;

	boot_io_drv_id = FIP_IMAGE_ID;

	register_io_dev_fip(&rzg2l);

	io_dev_open(rzg2l, 0, &fip_dev_handle);

	if (boot_dev == BOOT_MODE_SPI_1_8 ||
		boot_dev == BOOT_MODE_SPI_3_3) {
		spi_multi_setup(SPI_MULTI_ADDR_WIDES_24, SPI_MULTI_DQ_WIDES_1_4_4, SPI_MULTI_DUMMY_10CYCLE);
		register_io_dev_memmap(&memmap);
		io_dev_open(memmap, 0, &memdrv_dev_handle);
	} else {
		panic();
	}
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
				uintptr_t *image_spec)
{
	const struct plat_io_policy *policy;
	int result;

	policy = &policies[image_id];

	result = policy->check(policy->image_spec);
	if (result != 0)
		return result;

	*image_spec = policy->image_spec;
	*dev_handle = *(policy->dev_handle);

	return 0;
}

