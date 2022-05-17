#define DEBUG 1
#include <common.h>
#include <init.h>
#include <io.h>
#include <of.h>
#include <regulator.h>
#include <linux/err.h>
#include <errno.h>
#include <mfd/syscon.h>

#include <i2c/i2c.h>
#include <restart.h>

#define RK817_ID_MSB			0xed
#define RK817_ID_LSB			0xee

struct rk808 {
	struct device_d		*dev;
	struct i2c_client		*i2c;
	struct regmap_irq_chip_data	*irq_data;
	struct regmap			*regmap;
	long				variant;
	const struct regmap_config	*regmap_cfg;
	const struct regmap_irq_chip	*regmap_irq_chip;
};

struct rk808_reg_data {
	int addr;
	int mask;
	int value;
};

/*static struct regmap_bus regmap_rk808_i2c_bus = {
	.reg_write = pfuze_i2c_reg_write,
	.reg_read = pfuze_i2c_reg_read,
};

static const struct regmap_config rk808_regmap_i2c_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 127,
};*/

//maybe needs CONFIG_I2C_RK3X
static int rk808_probe(struct device_d *dev)
{
	struct rk808 *rk808;
	struct device_node *np = dev->device_node;
	unsigned char pmic_id_msb, pmic_id_lsb;

	struct device_d *parent;
	int i,ret;

	rk808 = xzalloc(sizeof(*rk808));
	if (!rk808)
		return -ENOMEM;

	rk808->dev = dev;
	rk808->i2c = to_i2c_client(dev);
	//rk808->regmap = regmap_init(dev, &regmap_rk808_i2c_bus,
	//			     rk808, &rk808_regmap_i2c_config);
	dev_info(dev, "DEBUG %s:%d\n",__FUNCTION__,__LINE__);

	if (of_device_is_compatible(np, "rockchip,rk817") ||
	    of_device_is_compatible(np, "rockchip,rk809")) {
		pmic_id_msb = RK817_ID_MSB;
		pmic_id_lsb = RK817_ID_LSB;
	}

	if (IS_ENABLED(CONFIG_OFDEVICE) && dev->device_node)
		return of_platform_populate(dev->device_node, NULL, dev);

	return 0;
}

/*static const struct mfd_cell rk817s[] = {
	{ .name = "rk808-clkout",},
	{ .name = "rk808-regulator",},
};*/

static __maybe_unused struct of_device_id rk808_compatible[] = {
	{
		.compatible = "rockchip,rk809",
	}, {
		.compatible = "rockchip,rk817",
	}, {
		/* sentinel */
	}
};

static struct driver_d rk808_driver = {
	.name	= "rk808",
	.probe = rk808_probe,
	.of_compatible = DRV_OF_COMPAT(rk808_compatible),
};
device_i2c_driver(rk808_driver);
