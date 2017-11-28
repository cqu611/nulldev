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

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", foo);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &foo);
	if (ret < 0)
		return ret;

	pr_info("RESTART: RAMUFS");
	

	return count;
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0664, foo_show, foo_store);

static struct attribute *attrs[] = {
	&foo_attribute.attr,
	NULL,	
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};




static ssize_t ramufs_group_features_show(struct config_item *item, char *page)
{
	return snprintf(page, PAGE_SIZE, "memory_backed,discard,bandwidth,cache,badblocks\n");
}

CONFIGFS_ATTR_RO(ramufs_group_, features);

static struct configfs_attribute *ramufs_group_attrs[] = {
	&ramufs_group_attr_features,
	NULL,
};

static struct config_item_type ramufs_group_type = {
	.ct_attrs	= ramufs_group_attrs,
	.ct_owner	= THIS_MODULE,
};

static struct configfs_subsystem ramufs_subsys = {
	.su_group = {
		.cg_item = {
			.ci_namebuf = "ramufs",
			.ci_type = &ramufs_group_type,
		},
	},
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
	ret = sysfs_create_group(ufs->kobj, &attr_group);
	if (ret) {
		pr_err("RAMUFS: sysfs_create_group failed\n");
		kobject_put(ufs->kobj);
		goto free_ufs;
	}


	config_group_init(&ramufs_subsys.su_group);
	mutex_init(&ramufs_subsys.su_mutex);
	ret = configfs_register_subsystem(&ramufs_subsys);
	if (ret) {
		pr_err("RAMUFS: ramufs_init: configfs_register_subsystem() failed\n");
		goto out;
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
	kobject_put(ufs->kobj);
	configfs_unregister_subsystem(&ramufs_subsys);
}

module_init(ramufs_init);
module_exit(ramufs_exit);

MODULE_AUTHOR("Gnekiah Hsiung <gnekiah@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UFS host controller ram-disk driver");
