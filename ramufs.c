#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/configfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/init.h>


struct ramufs {
	struct gendisk *gd;
	struct kobject kobj;
	struct configfs_subsystem ramufs_subsys;
	int wtf;
};
#define kobj_to_ramufs(x) container_of(x, struct ramufs, kobj)


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

static struct kobject *example_kobj;

static int __init ramufs_init(void)
{
	int retval;

	example_kobj = kobject_create_and_add("kobject_example", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(example_kobj, &attr_group);
	if (retval)
		kobject_put(example_kobj);

	return retval;
}

static void __exit ramufs_exit(void)
{
	kobject_put(example_kobj);
}



/*
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
	configfs_unregister_subsystem(&ramufs_subsys);
}
*/
module_init(ramufs_init);
module_exit(ramufs_exit);

MODULE_AUTHOR("Gnekiah Hsiung <gnekiah@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UFS host controller ram-disk driver");
