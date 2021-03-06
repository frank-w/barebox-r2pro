// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * MTD oob device
 *
 * Copyright (C) 2011 Sascha Hauer
 *
 * Adds a character devices :
 *  - mtdoob<N>
 */

#include <common.h>
#include <init.h>
#include <malloc.h>
#include <ioctl.h>
#include <errno.h>
#include <linux/mtd/mtd.h>

#include "mtd.h"

struct mtdoob {
	struct cdev cdev;
	struct mtd_info *mtd;
};

static struct mtd_info *to_mtd(struct cdev *cdev)
{
	struct mtdoob *mtdoob = cdev->priv;
	return mtdoob->mtd;
}

static ssize_t mtd_op_read_oob(struct cdev *cdev, void *buf, size_t count,
			     loff_t _offset, ulong flags)
{
	struct mtd_info *mtd = to_mtd(cdev);
	struct mtd_oob_ops ops;
	int ret;
	unsigned long offset = _offset;

	if (count < mtd->oobsize)
		return -EINVAL;

	ops.mode = MTD_OPS_RAW;
	ops.ooboffs = 0;
	ops.ooblen = mtd->oobsize;
	ops.oobbuf = buf;
	ops.datbuf = NULL;
	ops.len = mtd->oobsize;

	offset /= mtd->oobsize;
	ret = mtd_read_oob(mtd, offset * mtd->writesize, &ops);
	if (ret)
		return ret;

	return mtd->oobsize;
}

static struct cdev_operations mtd_ops_oob = {
	.read   = mtd_op_read_oob,
	.ioctl  = mtd_ioctl,
};

static int add_mtdoob_device(struct mtd_info *mtd, const char *devname, void **priv)
{
	struct mtdoob *mtdoob;

	if (mtd->parent || mtd->oobsize == 0)
		return 0;

	mtdoob = xzalloc(sizeof(*mtdoob));
	mtdoob->cdev.ops = &mtd_ops_oob;
	mtdoob->cdev.size = mtd_div_by_wb(mtd->size, mtd) * (loff_t)mtd->oobsize;
	mtdoob->cdev.name = basprintf("%s.oob", mtd->cdev.name);
	mtdoob->cdev.priv = mtdoob;
	mtdoob->cdev.dev = &mtd->dev;
	mtdoob->mtd = mtd;
	*priv = mtdoob;
	devfs_create(&mtdoob->cdev);

	return 0;
}

static int del_mtdoob_device(struct mtd_info *mtd, void **priv)
{
	struct mtdoob *mtdoob;

	if (mtd->parent || mtd->oobsize == 0)
		return 0;

	mtdoob = *priv;
	devfs_remove(&mtdoob->cdev);
	free(mtdoob);

	return 0;
}

static struct mtddev_hook mtdoob_hook = {
	.add_mtd_device = add_mtdoob_device,
	.del_mtd_device = del_mtdoob_device,
};

static int __init register_mtdoob(void)
{
	mtdcore_add_hook(&mtdoob_hook);
	return 0;
}

coredevice_initcall(register_mtdoob);
