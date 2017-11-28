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
			//.id[8],
			.mlc = {
				.num_pairs = 0,
				//.pairs[886],
			},
		},
	},
};

struct ramufs *ufs;

static int foo;

static ssize_t ramufs_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	const char *name = attr->attr.name;
	if (strcmp(name, "version") == 0) {
		pr_info("RESTART: this is a version");
	}
	return sprintf(buf, "%d\n", foo);
}

static ssize_t ramufs_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &foo);
	if (ret < 0)
		return ret;

	pr_info("RESTART: RAMUFS");
	

	return count;
}


#define RAMUFS_ATTR(_name)			\
static struct kobj_attribute ramufs_attr_##_name =	\
	__ATTR(_name, 0664, ramufs_show, ramufs_store)

/* geometry */
RAMUFS_ATTR(ufs_geo);
RAMUFS_ATTR(ppa_fmt);
RAMUFS_ATTR(cfg_grp);
RAMUFS_ATTR(l2p_tbl);


/*
RAMUFS_ATTR(version);
RAMUFS_ATTR(vnvmt);
RAMUFS_ATTR(cgrps);
RAMUFS_ATTR(cap);
RAMUFS_ATTR(dom);

RAMUFS_ATTR(ch_off);
RAMUFS_ATTR(ch_len);
RAMUFS_ATTR(lun_off);
RAMUFS_ATTR(lun_len);
RAMUFS_ATTR(pln_off);
RAMUFS_ATTR(pln_len);
RAMUFS_ATTR(blk_off);
RAMUFS_ATTR(blk_len);
RAMUFS_ATTR(pg_off);
RAMUFS_ATTR(pg_len);
RAMUFS_ATTR(sect_off);
RAMUFS_ATTR(sect_len);
RAMUFS_ATTR(mtype);
RAMUFS_ATTR(fmtype);
RAMUFS_ATTR(num_ch);
RAMUFS_ATTR(num_lun);
RAMUFS_ATTR(num_pln);
RAMUFS_ATTR(num_blk);
RAMUFS_ATTR(num_pg);
RAMUFS_ATTR(fpg_sz);
RAMUFS_ATTR(csecs);
RAMUFS_ATTR(sos);
RAMUFS_ATTR(trdt);
RAMUFS_ATTR(trdm);
RAMUFS_ATTR(tprt);
RAMUFS_ATTR(tprm);
RAMUFS_ATTR(tbet);
RAMUFS_ATTR(tbem);
RAMUFS_ATTR(mpos);
RAMUFS_ATTR(mccap);
RAMUFS_ATTR(cpar);

RAMUFS_ATTR(l2ptbl);
*/

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
