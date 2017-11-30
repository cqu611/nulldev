#include "ramufs.h"

static struct ufs_geo geo = {
	.version 		= 0,
	.vnvmt 			= 0,
	.cgrps 			= 1,
	.cap 			= 0,
	.dom 			= 0,
	.ppaf = {
		.ch_off 	= 0,
		.ch_len 	= 0,
		.lun_off 	= 0,
		.lun_len 	= 0,
		.pln_off 	= 0,
		.pln_len 	= 0,
		.blk_off 	= 0,
		.blk_len 	= 0,
		.pg_off 	= 0,
		.pg_len 	= 0,
		.sect_off 	= 0,
		.sect_len 	= 0,
	},
	.ggrp = {
		.mtype 		= 0,
		.fmtype 	= 0,
		.num_ch 	= 0,
		.num_lun 	= 0,
		.num_pln 	= 0,
		.num_blk 	= 0,
		.num_pg 	= 0,
		.fpg_sz 	= 0,
		.csecs 		= 0,
		.sos 		= 0,
		.trdt 		= 0,
		.trdm 		= 0,
		.tprt 		= 0,
		.tprm 		= 0,
		.tbet 		= 0,
		.tbem 		= 0,
		.mpos 		= 0,
		.mccap 		= 0,
		.cpar 		= 0,
		.l2ptbl = {
			/* .id[8], */
			.mlc = {
				.num_pairs = 0,
				/* .pairs[886], */
			},
		},
	},
};

struct ramufs *ufs;

static ssize_t __show_ufs_geo(char *buf)
{
	return sprintf(buf, "\
	Version                         =%#8x\n\
	Vendor NVM Opcode Command Set   =%#8x\n\
	Configuration Groups            =%#8x\n\
	Capabilities                    =%#32x\n\
	Device Op Mode                  =%#32x\n", 
	geo.version, geo.vnvmt, geo.cgrps, geo.cap, geo.dom);
}

static ssize_t __show_ppa_fmt(char *buf)
{
	return sprintf(buf, "\
	Channel bit start       =%#x\n\
	Channel bit length      =%#x\n\
	LUN bit start           =%#x\n\
	LUN bit length          =%#x\n\
	Plane bit start         =%#x\n\
	Plane bit length        =%#x\n\
	Block bit start         =%#x\n\
	Block bit length        =%#x\n\
	Page bit start          =%#x\n\
	Page bit length         =%#x\n\
	Sector bit start        =%#x\n\
	Sector bit length       =%#x\n",
	geo.ppaf.ch_off, geo.ppaf.ch_len, geo.ppaf.lun_off,
	geo.ppaf.lun_len, geo.ppaf.pln_off, geo.ppaf.pln_len,
	geo.ppaf.blk_off, geo.ppaf.blk_len, geo.ppaf.pg_off, 
	geo.ppaf.pg_len, geo.ppaf.sect_off, geo.ppaf.sect_len);
}

static ssize_t __show_cfg_grp(char *buf)
{
	return sprintf(buf, "\
	Media Type                          =%#x\n\
	Flash Media Type                    =%#x\n\
	Number of Channels                  =%#x\n\
	Number of LUNs                      =%#x\n\
	Number of Planes                    =%#x\n\
	Number of Blocks                    =%#x\n\
	Number of Pages                     =%#x\n\
	Page Size                           =%#x\n\
	Controller Sector Size              =%#x\n\
	Sector OOB Size                     =%#x\n\
	tRD Typical                         =%#x\n\
	tRD Max                             =%#x\n\
	tPROG Typical                       =%#x\n\
	tPROG Max                           =%#x\n\
	tBERS Typical                       =%#x\n\
	tBERS Max                           =%#x\n\
	Multi-plane Operation Support       =%#x\n\
	Media and Controller Capabilities   =%#x\n\
	Channel Parallelism                 =%#x\n",
	geo.ggrp.mtype, geo.ggrp.fmtype, geo.ggrp.num_ch,
	geo.ggrp.num_lun, geo.ggrp.num_pln, geo.ggrp.num_blk,
	geo.ggrp.num_pg, geo.ggrp.fpg_sz, geo.ggrp.csecs,
	geo.ggrp.sos, geo.ggrp.trdt, geo.ggrp.trdm, geo.ggrp.tprt, 
	geo.ggrp.tprm, geo.ggrp.tbet, geo.ggrp.tbem, geo.ggrp.mpos, 
	geo.ggrp.mccap, geo.ggrp.cpar);
}

static ssize_t __show_l2p_tbl(char *buf)
{
	return snprintf(buf, sizeof(struct ufs_geo_l2p_tbl), "%s\n", geo.ggrp.l2ptbl);
}

	/* .l2ptbl = {
			.id[8], 
			.mlc = {
				.num_pairs = 0,
				.pairs[886],
			},
		}, */

static ssize_t ramufs_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	const char *name = attr->attr.name;
	
	if (strcmp(name, "ufs_geo") == 0) {
		pr_info("RAMUFS: show geometry\n");
		return __show_ufs_geo(buf);
	} else if (strcmp(name, "ppa_fmt") == 0) {
		pr_info("RAMUFS: show ppa format\n");
		return __show_ppa_fmt(buf);
	} else if (strcmp(name, "cfg_grp") == 0) {
		pr_info("RAMUFS: show configuration group\n");
		return __show_cfg_grp(buf);
	} else if (strcmp(name, "l2p_tbl") == 0) {
		pr_info("RAMUFS: show l2p table\n");
		return __show_l2p_tbl(buf);
	}
	return sprintf(buf, "Unhandled attr(%s) in `ramufs_show`\n", name);
}

static ssize_t ramufs_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;
	const char *name = attr->attr.name;
	
	if (strcmp(name, "ufs_geo") == 0) {
		pr_info("RAMUFS: set geometry\n");
	} else if (strcmp(name, "ppa_fmt") == 0) {
		pr_info("RAMUFS: set ppa format\n");
	} else if (strcmp(name, "cfg_grp") == 0) {
		pr_info("RAMUFS: set configuration group\n");
	} else if (strcmp(name, "l2p_tbl") == 0) {
		pr_info("RAMUFS: set l2p table\n");
	}
	return -EINVAL;
/*
	ret = kstrtoint(buf, 10, &foo);
	if (ret < 0)
		return ret;

	return count;
*/
}

#define RAMUFS_ATTR(_name)			\
static struct kobj_attribute ramufs_attr_##_name =	\
	__ATTR(_name, 0664, ramufs_show, ramufs_store)

RAMUFS_ATTR(ufs_geo);
RAMUFS_ATTR(ppa_fmt);
RAMUFS_ATTR(cfg_grp);
RAMUFS_ATTR(l2p_tbl);

static struct attribute *ramufs_attrs[] = {
	&ramufs_attr_ufs_geo.attr,
	&ramufs_attr_ppa_fmt.attr,
	&ramufs_attr_cfg_grp.attr,
	&ramufs_attr_l2p_tbl.attr,
	NULL,	
};

static struct attribute_group ramufs_attr_group = {
	.attrs = ramufs_attrs,
};

static int __init ramufs_init(void)
{
	int ret = 0;

	pr_info("RAMUFS: ramufs init\n");
	ufs = kzalloc(sizeof(struct ramufs), GFP_KERNEL);
	if (!ufs) {
		pr_err("RAMUFS: kzalloc failed, out of memory\n");
		ret = -ENOMEM;
		goto out;
	}

	ufs->kobj = kobject_create_and_add("ramufs", kernel_kobj);
	if (!(ufs->kobj)) {
		pr_err("RAMUFS: kobject_create_and_add failed, out of memory\n");
		ret = -ENOMEM;
		goto free_ufs;
	}
	ret = sysfs_create_group(ufs->kobj, &ramufs_attr_group);
	if (ret) {
		pr_err("RAMUFS: sysfs_create_group failed\n");
		kobject_put(ufs->kobj);
		goto free_ufs;
	}

	pr_info("RAMUFS: ramufs loaded\n");
	return ret;

free_ufs:
	kfree(ufs);
out:
	return ret;
}

static void __exit ramufs_exit(void)
{
	pr_info("RAMUFS: ramufs exit\n");
	sysfs_remove_group(ufs->kobj, &ramufs_attr_group);
	kobject_put(ufs->kobj);
	kfree(ufs);
}

module_init(ramufs_init);
module_exit(ramufs_exit);

MODULE_AUTHOR("Gnekiah Hsiung <gnekiah@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UFS host controller ram-disk driver");
