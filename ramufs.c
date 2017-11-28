#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/configfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/init.h>

struct ufs_ppa_format {
	u8	ch_off;
	u8	ch_len;
	u8	lun_off;
	u8	lun_len;
	u8	pln_off;
	u8	pln_len;
	u8	blk_off;
	u8	blk_len;
	u8	pg_off;
	u8	pg_len;
	u8	sect_off;
	u8	sect_len;
};

struct ufs_geo_l2p_mlc {
	u16	num_pairs;
	u8	pairs[886];
};

struct ufs_geo_l2p_tbl {
	__u8	id[8];
	struct ufs_geo_l2p_mlc mlc;
};

struct ufs_geo_group {
	u8	mtype;
	u8	fmtype;
	u8	num_ch;
	u8	num_lun;
	u8	num_pln;
	u16	num_blk;
	u16	num_pg;
	u16	fpg_sz;
	u16	csecs;
	u16	sos;
	u32	trdt;
	u32	trdm;
	u32	tprt;
	u32	tprm;
	u32	tbet;
	u32	tbem;
	u32	mpos;
	u32	mccap;
	u16	cpar;
	struct ufs_geo_l2p_tbl l2ptbl;
};

struct ufs_geo {
	u8	version;
	u8	vnvmt;
	u8	cgrps;
	u32	cap;
	u32	dom;
	struct ufs_ppa_format ppaf;
	struct ufs_geo_group  ggrp;
}__packed;

struct ramufs {
	struct ufs_geo *geo;

	struct gendisk *gd;
	struct kobject *kobj;
	
	struct configfs_subsystem ramufs_subsys;
};
//#define kobj_to_ramufs(x) container_of(x, struct ramufs, kobj)

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




static struct config_item *ramufs_group_make_item(struct config_group *group, const char *name)
{
	return (struct config_item*)NULL;
}

static void ramufs_group_drop_item(struct config_group *group, struct config_item *item)
{
}

static ssize_t ramufs_group_features_show(struct config_item *item, char *page)
{
	return snprintf(page, PAGE_SIZE, "memory_backed,discard,bandwidth,cache,badblocks\n");
}

CONFIGFS_ATTR_RO(ramufs_group_, features);

static struct configfs_attribute *ramufs_group_attrs[] = {
	&ramufs_group_attr_features,
	NULL,
};

static struct configfs_group_operations ramufs_group_ops = {
	.make_item	= ramufs_group_make_item,
	.drop_item	= ramufs_group_drop_item,
};

static struct config_item_type ramufs_group_type = {
	.ct_group_ops	= &ramufs_group_ops,
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
	kobject *kobj;

	ufs = kzalloc(sizeof(struct ramufs), GFP_KERNEL);
	if (!ufs) {

	}

	ufs->kobj = kobject_create_and_add("ram-ufs", kernel_kobj);
	if (!(ufs->kobj)) {
		
	}
	ret = sysfs_create_group(ufs->kobj, &attr_group);
	if (ret) {
		
		kobject_put(ufs->kobj);
	}

	pr_info("RAMUFS: ramufs init\n");
	config_group_init(&ramufs_subsys.su_group);
	mutex_init(&ramufs_subsys.su_mutex);
	ret = configfs_register_subsystem(&ramufs_subsys);
	if (ret) {
		pr_err("RAMUFS: ramufs_init: configfs_register_subsystem() failed\n");
		goto out;
	}

	pr_info("RAMUFS: ramufs loaded\n");

out:
	return ret;
}

static void __exit ramufs_exit(void)
{
	pr_err("RAMUFS: ramufs exit\n");
	kobject_put(ufs->kobj);
	configfs_unregister_subsystem(&ramufs_subsys);
}

module_init(ramufs_init);
module_exit(ramufs_exit);

MODULE_AUTHOR("Gnekiah Hsiung <gnekiah@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UFS host controller ram-disk driver");
