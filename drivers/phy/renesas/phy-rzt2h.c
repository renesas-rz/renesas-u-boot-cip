#include <common.h>
#include <dm.h>
#include <generic-phy.h>

static int rzt2h_usb2_phy_init(struct phy *phy)
{
    return 0;
}

static const struct udevice_id rzt2h_usb2_phy_ids[] = {
    { .compatible = "renesas,rzt2h-usb2-phy" },
    { .compatible = "renesas,rzn2h-usb2-phy" },
    { }
};

static struct phy_ops rzt2h_usb2_phy_ops = {
    .init = rzt2h_usb2_phy_init,
};

U_BOOT_DRIVER(rzt2h_usb2_phy) = {
    .name = "rzt2h_usb2_phy",
    .id = UCLASS_PHY,
    .of_match = rzt2h_usb2_phy_ids,
    .ops = &rzt2h_usb2_phy_ops,
};
