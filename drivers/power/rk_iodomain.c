#define DEBUG 1
#include <common.h>
#include <init.h>
#include <io.h>
#include <of.h>
#include <regulator.h>
#include <linux/err.h>
#include <errno.h>
#include <mfd/syscon.h>

#define MAX_SUPPLIES		16
#define MAX_VOLTAGE_1_8		1980000
#define MAX_VOLTAGE_3_3		3600000

#define RK3568_PMU_GRF_IO_VSEL0		(0x0140)
#define RK3568_PMU_GRF_IO_VSEL1		(0x0144)
#define RK3568_PMU_GRF_IO_VSEL2		(0x0148)

struct rockchip_iodomain;

struct rockchip_iodomain_supply {
	struct rockchip_iodomain *iod;
	struct regulator *reg;
	//struct notifier_block nb;
	int idx;
};

struct rockchip_iodomain_soc_data {
	int grf_offset;
	const char *supply_names[MAX_SUPPLIES];
	void (*init)(struct rockchip_iodomain *iod);
	int (*write)(struct rockchip_iodomain_supply *supply, int uV);
};

struct rockchip_iodomain {
	struct device_d *dev;
	struct regmap *grf;
	const struct rockchip_iodomain_soc_data *soc_data;
	struct rockchip_iodomain_supply supplies[MAX_SUPPLIES];
	int (*write)(struct rockchip_iodomain_supply *supply, int uV);
};

static int rk3568_iodomain_write(struct rockchip_iodomain_supply *supply, int uV)
{
	return 0;

	struct rockchip_iodomain *iod = supply->iod;
	u32 is_3v3 = uV > MAX_VOLTAGE_1_8;
	u32 val0, val1;
	int b;

	switch (supply->idx) {
	case 0: /* pmuio1 */
		break;
	case 1: /* pmuio2 */
		b = supply->idx;
		val0 = BIT(16 + b) | (is_3v3 ? 0 : BIT(b));
		b = supply->idx + 4;
		val1 = BIT(16 + b) | (is_3v3 ? BIT(b) : 0);

		regmap_write(iod->grf, RK3568_PMU_GRF_IO_VSEL2, val0);
		regmap_write(iod->grf, RK3568_PMU_GRF_IO_VSEL2, val1);
		break;
	case 3: /* vccio2 */
		break;
	case 2: /* vccio1 */
	case 4: /* vccio3 */
	case 5: /* vccio4 */
	case 6: /* vccio5 */
	case 7: /* vccio6 */
	case 8: /* vccio7 */
		b = supply->idx - 1;
		val0 = BIT(16 + b) | (is_3v3 ? 0 : BIT(b));
		val1 = BIT(16 + b) | (is_3v3 ? BIT(b) : 0);

		regmap_write(iod->grf, RK3568_PMU_GRF_IO_VSEL0, val0);
		regmap_write(iod->grf, RK3568_PMU_GRF_IO_VSEL1, val1);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int rk_iodomain_probe(struct device_d *dev)
{
	//struct device_node *np = dev->device_node;
	struct rockchip_iodomain *iod;

	//const struct rockchip_iodomain_soc_data *soc_data;
	struct device_d *parent;
	int i,ret;

	iod = xzalloc(sizeof(*iod));

	iod->dev = dev;

	iod->soc_data = device_get_match_data(dev);
	if (!iod->soc_data)
		return -EINVAL;

	parent = dev->parent;
	if (parent && parent->device_node) {
		iod->grf = syscon_node_to_regmap(parent->device_node);
	} /*else {
		dev_dbg(&pdev->dev, "falling back to old binding\n");
		iod->grf = syscon_regmap_lookup_by_phandle(np, "rockchip,grf");
	}*/



	for (i = 0; i < MAX_SUPPLIES; i++) {
		const char *supply_name = iod->soc_data->supply_names[i];
		//struct rockchip_iodomain_supply *supply = &iod->supplies[i];
		struct regulator *reg;
		int uV;

		if (!supply_name)
			continue;
		dev_info(dev, "DEBUG %s:%d reg:%s\n",__FUNCTION__,__LINE__,supply_name);
		reg = regulator_get(iod->dev, supply_name); //reg=NULL
		if (reg)
		{
			if (IS_ERR(reg)) {
				ret = PTR_ERR(reg);
				dev_info(dev, "DEBUG %s:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

				/* If a supply wasn't specified, that's OK */
				if (ret == -ENODEV)
					continue;
				else if (ret != -EPROBE_DEFER)
					dev_err(iod->dev, "couldn't get regulator %s\n",
						supply_name);
				dev_info(dev, "DEBUG %s:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
				return ret;
			}
			/* set initial correct value */
			uV = regulator_get_voltage(reg);
			dev_info(iod->dev, "DEBUG reg:%s (%p) uV:%d\n",supply_name,reg,uV);
		}
	}
	dev_info(dev, "DEBUG %s:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	return 0;
}

static const struct rockchip_iodomain_soc_data soc_data_rk3568_pmu = {
	.grf_offset = 0x140,
	.supply_names = {
		"pmuio1",
		"pmuio2",
		"vccio1",
		"vccio2",
		"vccio3",
		"vccio4",
		"vccio5",
		"vccio6",
		"vccio7",
	},
	.write = rk3568_iodomain_write,
};

static __maybe_unused struct of_device_id rk_iodomain_compatible[] = {
	{
		.compatible = "rockchip,rk3568-pmu-io-voltage-domain",
		.data = &soc_data_rk3568_pmu
	}, {
		/* sentinel */
	}
};

static struct driver_d rk_iodomain_driver = {
	.name  = "rk_iodomain",
	.probe = rk_iodomain_probe,
	.of_compatible = DRV_OF_COMPAT(rk_iodomain_compatible),
};
device_platform_driver(rk_iodomain_driver);
