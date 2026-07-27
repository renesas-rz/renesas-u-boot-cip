// SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause)
/*
 * Clock driver for Renesas RZ/T2N SoCs.
 *
 * Copyright (C) 2024 Renesas Electronics Corporation
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/io.h>
#include <linux/err.h>
#include <dt-bindings/clock/r9a07g076-cpg.h>  // Define R9A07G076_USB_CLK, etc

struct r9a07g076_clk_priv {
    void __iomem *base;
};

#define MSTPCRM			0x80280330
#define MSTPCRM_SDHI0		BIT(12)
#define MSTPCRM_SDHI1		BIT(13)


#define MSTPCRE			0x80280310
#define MSTPCRE_GMAC1		BIT(16)
#define MSTPCRE_GMAC2		BIT(17)
#define MSTPCRE_ETHSS		BIT(3)

#define MRCTLE			0x80280250
#define MRCTLE_GMAC1_PCLKH	BIT(16)
#define MRCTLE_GMAC1_PCLKM	BIT(17)
#define MRCTLE_GMAC2_PCLKH	BIT(18)
#define MRCTLE_GMAC2_PCLKM	BIT(19)

#define MRCTLE_ETHSS		BIT(5)
#define MRCTLE_MII_CONVERTER		BIT(6)

#define PRCRN			0x80294200
#define PRCRN_PRKEY		(0xa5 << 8)
#define PRCRN_WR_EN		0xF
#define PRCRS                   0x81296000
#define PRCRS_PRKEY             (0xa5 << 8)
#define PRCRS_WR_EN             0xF

/* xSPI clock enable*/
#define MRCTLA          0x80280240
#define MRCTLA_XSPI1    BIT(5)

#define SCKCR2			0x81280004
#define SCKCR2_CA55SCLK		BIT(12)

static int r9a07g076_clk_of_xlate(struct clk *clk,
                  struct ofnode_phandle_args *args)
{
    if (args->args_count != 2) {
        printf("CLK: ERROR invalid args_count=%d (expected 2)\n",
               args->args_count);
        return -EINVAL;
    }

    clk->id = args->args[1];  /* args[0] = type, args[1] = ID */

    return 0;
}

static int r9a07g076_clk_enable(struct clk *clk)
{
    /* Disable Write protect to enable writing */
    *(volatile u32 *)PRCRN = PRCRN_PRKEY | PRCRN_WR_EN;
    *(volatile u32 *)PRCRS = PRCRS_PRKEY | PRCRS_WR_EN;
	switch(clk->id)
    {
        case R9A07G076_SDHI0_CLK:
            *(volatile u32 *)MSTPCRM &= ~(MSTPCRM_SDHI0);
            break;
        case R9A07G076_SDHI1_CLK:
            *(volatile u32 *)MSTPCRM &= ~(MSTPCRM_SDHI1);
            break;

        case R9A07G076_GMAC1_CLK:

	        /* Release module stop for GMAC1 */
	        *(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC1);

	        /* Release module reset for GMAC1 */
	        *(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC1_PCLKH | MRCTLE_GMAC1_PCLKM);

            break;

        case R9A07G076_GMAC2_CLK:

	        /* Release module stop for GMAC1 */
	        *(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC1);

	        /* Release module reset for GMAC1 */
	        *(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC1_PCLKH | MRCTLE_GMAC1_PCLKM);

            /* Release module stop for GMAC2 */
            *(volatile u32 *)MSTPCRE &= ~(MSTPCRE_GMAC2);

            /* Release module reset for GMAC2 */
            *(volatile u32 *)MRCTLE &= ~(MRCTLE_GMAC2_PCLKH | MRCTLE_GMAC2_PCLKM);


            /* Release module stop for ETH_SS */
            *(volatile u32 *)MSTPCRE &= ~(MSTPCRE_ETHSS);

            /* Release module reset for ETH_SS, MII_CONV */
            *(volatile u32 *)MRCTLE &= ~(MRCTLE_ETHSS | MRCTLE_MII_CONVERTER);

            /* Set Coretex-A55 DSU clock to 1 GHz */
            *(volatile u32 *)SCKCR2 |= SCKCR2_CA55SCLK;

            break;
        case R9A07G076_xSPI0_CLK:
            /* Release module reset for XSPI0 */
            *(volatile u32 *)MRCTLA &= ~(MRCTLA_XSPI1);
            break;
        default:
            break;
    }
    /* Enable Write protect to disable writing */
    *(volatile u32 *)PRCRN = PRCRN_PRKEY;
    *(volatile u32 *)PRCRS = PRCRS_PRKEY;
    return 0;
}

static int r9a07g076_clk_probe(struct udevice *dev)
{
    struct r9a07g076_clk_priv *priv = dev_get_priv(dev);

    priv->base = dev_read_addr_ptr(dev);

    if (!priv->base) {
        printf("CLK: ERROR base address is NULL\n");
        return -EINVAL;
    }
    return 0;
}

static const struct clk_ops r9a07g076_clk_ops = {
	.of_xlate = r9a07g076_clk_of_xlate,
	.enable = r9a07g076_clk_enable,
};

static const struct udevice_id r9a07g076_clk_ids[] = {
	{ .compatible = "renesas,r9a07g076-cpg" },
	{ }
};

U_BOOT_DRIVER(r9a07g076_clk) = {
	.name = "clk_r9a07g076",
	.id = UCLASS_CLK,
	.of_match = r9a07g076_clk_ids,
	.priv_auto = sizeof(struct r9a07g076_clk_priv),
	.probe = r9a07g076_clk_probe,
	.ops = &r9a07g076_clk_ops,
};

