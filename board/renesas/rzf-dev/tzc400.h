/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TZC400_H
#define TZC400_H

#include "tzc_common.h"

#define BUILD_CONFIG_OFF			(0x000)
#define GATE_KEEPER_OFF				(0x008)
#define SPECULATION_CTRL_OFF			(0x00c)
#define INT_STATUS				(0x010)
#define INT_CLEAR				(0x014)

#define FAIL_ADDRESS_LOW_OFF			(0x020)
#define FAIL_ADDRESS_HIGH_OFF			(0x024)
#define FAIL_CONTROL_OFF			(0x028)
#define FAIL_ID					(0x02c)

/* ID registers not common across different varieties of TZC */
#define PID5					(0xFD4)
#define PID6					(0xFD8)
#define PID7					(0xFDC)

#define BUILD_CONFIG_NF_SHIFT			24
#define BUILD_CONFIG_NF_MASK			(0x3)
#define BUILD_CONFIG_AW_SHIFT			8
#define BUILD_CONFIG_AW_MASK			(0x3f)
#define BUILD_CONFIG_NR_SHIFT			0
#define BUILD_CONFIG_NR_MASK			(0x1f)

/*
 * Number of gate keepers is implementation defined. But we know the max for
 * this device is 4. Get implementation details from BUILD_CONFIG.
 */
#define GATE_KEEPER_OS_SHIFT			16
#define GATE_KEEPER_OS_MASK			(0xf)
#define GATE_KEEPER_OR_SHIFT			0
#define GATE_KEEPER_OR_MASK			(0xf)
#define GATE_KEEPER_FILTER_MASK			(0x1)

/* Speculation is enabled by default. */
#define SPECULATION_CTRL_WRITE_DISABLE		BIT_32(1)
#define SPECULATION_CTRL_READ_DISABLE		BIT_32(0)

/* Max number of filters allowed is 4. */
#define INT_STATUS_OVERLAP_SHIFT		16
#define INT_STATUS_OVERLAP_MASK			(0xf)
#define INT_STATUS_OVERRUN_SHIFT		8
#define INT_STATUS_OVERRUN_MASK			(0xf)
#define INT_STATUS_STATUS_SHIFT			0
#define INT_STATUS_STATUS_MASK			(0xf)

#define INT_CLEAR_CLEAR_SHIFT			0
#define INT_CLEAR_CLEAR_MASK			(0xf)

#define FAIL_CONTROL_DIR_SHIFT			24
#define FAIL_CONTROL_DIR_READ			(0)
#define FAIL_CONTROL_DIR_WRITE			(1)
#define FAIL_CONTROL_NS_SHIFT			21
#define FAIL_CONTROL_NS_SECURE			(0)
#define FAIL_CONTROL_NS_NONSECURE		(1)
#define FAIL_CONTROL_PRIV_SHIFT			20
#define FAIL_CONTROL_PRIV_UNPRIV		(0)
#define FAIL_CONTROL_PRIV_PRIV			(1)

/*
 * FAIL_ID_ID_MASK depends on AID_WIDTH which is platform specific.
 * Platform should provide the value on initialisation.
 */
#define FAIL_ID_VNET_SHIFT			24
#define FAIL_ID_VNET_MASK			(0xf)
#define FAIL_ID_ID_SHIFT			0

#define TZC_400_PERIPHERAL_ID			(0x460)

/* Filter enable bits in a TZC */
#define TZC_400_REGION_ATTR_F_EN_MASK		(0xf)
#define TZC_400_REGION_ATTR_FILTER_BIT(x)	((1) << (x))
#define TZC_400_REGION_ATTR_FILTER_BIT_ALL	TZC_400_REGION_ATTR_F_EN_MASK

/*
 * All TZC region configuration registers are placed one after another. It
 * depicts size of block of registers for programming each region.
 */
#define TZC_400_REGION_SIZE			(0x20)
#define TZC_400_ACTION_OFF			(0x4)

#define FILTER_OFFSET				(0x10)

#ifndef __ASSEMBLER__

#include <stdint.h>

/*******************************************************************************
 * Function & variable prototypes
 ******************************************************************************/
void tzc400_init(uintptr_t base);
void tzc400_configure_region0(unsigned int sec_attr,
			   unsigned int ns_device_access);
void tzc400_configure_region(unsigned int filters,
			  unsigned int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  unsigned int sec_attr,
			  unsigned int nsaid_permissions);
void tzc400_update_filters(unsigned int region, unsigned int filters);
void tzc400_set_action(unsigned int action);
void tzc400_enable_filters(void);
void tzc400_disable_filters(void);
int tzc400_it_handler(void);

static inline void tzc_init(uintptr_t base)
{
	tzc400_init(base);
}

static inline void tzc_configure_region0(
			unsigned int sec_attr,
			unsigned int ns_device_access)
{
	tzc400_configure_region0(sec_attr, ns_device_access);
}

static inline void tzc_configure_region(
			  unsigned int filters,
			  unsigned int region,
			  unsigned long long region_base,
			  unsigned long long region_top,
			  unsigned int sec_attr,
			  unsigned int ns_device_access)
{
	tzc400_configure_region(filters, region, region_base,
			region_top, sec_attr, ns_device_access);
}

static inline void tzc_set_action(unsigned int action)
{
	tzc400_set_action(action);
}


static inline void tzc_enable_filters(void)
{
	tzc400_enable_filters();
}

static inline void tzc_disable_filters(void)
{
	tzc400_disable_filters();
}

#endif /* __ASSEMBLER__ */

#endif /* TZC400_H */
