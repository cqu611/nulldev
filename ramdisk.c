#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/configfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/init.h>


/*
static struct config_item *ramdisk_group_make_item(struct config_group *group, const char *name)
{
	return (struct config_item*)NULL;
}

static void ramdisk_group_drop_item(struct config_group *group, struct config_item *item)
{
}

static ssize_t ramdisk_group_features_show(struct config_item *item, char *page)
{
	return snprintf(page, PAGE_SIZE, "memory_backed,discard,bandwidth,cache,badblocks\n");
}

CONFIGFS_ATTR_RO(ramdisk_group_, features);


static struct configfs_attribute *ramdisk_group_attrs[] = {
	&ramdisk_group_attr_features,
	NULL,
};

static struct configfs_group_operations ramdisk_group_ops = {
	.make_item	= ramdisk_group_make_item,
	.drop_item	= ramdisk_group_drop_item,
};

static struct config_item_type ramdisk_group_type = {
	.ct_group_ops	= &ramdisk_group_ops,
	.ct_attrs	= ramdisk_group_attrs,
	.ct_owner	= THIS_MODULE,
};

static struct configfs_subsystem ramdisk_subsys = {
	.su_group = {
		.cg_item = {
			.ci_namebuf = "ramdisk",
			.ci_type = &ramdisk_group_type,
		},
	},
};


static int __init ramdisk_init(void)
{
	int ret = 0;

	pr_info("RAMDISK: ramdisk init\n");
	config_group_init(&ramdisk_subsys.su_group);
	mutex_init(&ramdisk_subsys.su_mutex);
	ret = configfs_register_subsystem(&ramdisk_subsys);
	if (ret) {
		pr_err("RAMDISK: ramdisk_init: configfs_register_subsystem() failed\n");
		goto out;
	}

	pr_info("RAMDISK: ramdisk loaded\n");

out:
	return ret;
}

static void __exit ramdisk_exit(void)
{
	pr_err("RAMDISK: ramdisk exit\n");
	configfs_unregister_subsystem(&ramdisk_subsys);
}
*/



struct foo_obj {
	struct kobject kobj;
	int foo;
	int baz;
	int bar;
};
#define to_foo_obj(x) container_of(x, struct foo_obj, kobj)

struct foo_attribute {
	struct attribute attr;
	ssize_t (*show)(struct foo_obj *foo, struct foo_attribute *attr, char *buf);
	ssize_t (*store)(struct foo_obj *foo, struct foo_attribute *attr, const char *buf, size_t count);
};
#define to_foo_attr(x) container_of(x, struct foo_attribute, attr)

static ssize_t foo_attr_show(struct kobject *kobj,
			     struct attribute *attr,
			     char *buf)
{
	struct foo_attribute *attribute;
	struct foo_obj *foo;

	attribute = to_foo_attr(attr);
	foo = to_foo_obj(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(foo, attribute, buf);
}

static ssize_t foo_attr_store(struct kobject *kobj,
			      struct attribute *attr,
			      const char *buf, size_t len)
{
	struct foo_attribute *attribute;
	struct foo_obj *foo;

	attribute = to_foo_attr(attr);
	foo = to_foo_obj(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(foo, attribute, buf, len);
}

static const struct sysfs_ops foo_sysfs_ops = {
	.show = foo_attr_show,
	.store = foo_attr_store,
};

static void foo_release(struct kobject *kobj)
{
	struct foo_obj *foo;

	foo = to_foo_obj(kobj);
	kfree(foo);
}

static ssize_t foo_show(struct foo_obj *foo_obj, struct foo_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", foo_obj->foo);
}

static ssize_t foo_store(struct foo_obj *foo_obj, struct foo_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &foo_obj->foo);
	if (ret < 0)
		return ret;

	return count;
}

static struct foo_attribute foo_attribute =
	__ATTR(foo, 0664, foo_show, foo_store);

static ssize_t b_show(struct foo_obj *foo_obj, struct foo_attribute *attr,
		      char *buf)
{
	int var;

	if (strcmp(attr->attr.name, "baz") == 0)
		var = foo_obj->baz;
	else
		var = foo_obj->bar;
	return sprintf(buf, "%d\n", var);
}

static ssize_t b_store(struct foo_obj *foo_obj, struct foo_attribute *attr,
		       const char *buf, size_t count)
{
	int var, ret;

	ret = kstrtoint(buf, 10, &var);
	if (ret < 0)
		return ret;

	if (strcmp(attr->attr.name, "baz") == 0)
		foo_obj->baz = var;
	else
		foo_obj->bar = var;
	return count;
}

static struct foo_attribute baz_attribute =
	__ATTR(baz, 0664, b_show, b_store);
static struct foo_attribute bar_attribute =
	__ATTR(bar, 0664, b_show, b_store);

static struct attribute *foo_default_attrs[] = {
	&foo_attribute.attr,
	&baz_attribute.attr,
	&bar_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct kobj_type foo_ktype = {
	.sysfs_ops = &foo_sysfs_ops,
	.release = foo_release,
	.default_attrs = foo_default_attrs,
};

static struct kset *example_kset;
static struct foo_obj *foo_obj;
static struct foo_obj *bar_obj;
static struct foo_obj *baz_obj;

static struct foo_obj *create_foo_obj(const char *name)
{
	struct foo_obj *foo;
	int retval;

	foo = kzalloc(sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return NULL;

	foo->kobj.kset = example_kset;

	retval = kobject_init_and_add(&foo->kobj, &foo_ktype, NULL, "%s", name);
	if (retval) {
		kobject_put(&foo->kobj);
		return NULL;
	}

	kobject_uevent(&foo->kobj, KOBJ_ADD);

	return foo;
}

static void destroy_foo_obj(struct foo_obj *foo)
{
	kobject_put(&foo->kobj);
}

static int __init ramdisk_init(void)
{
	example_kset = kset_create_and_add("kset_example", NULL, kernel_kobj);
	if (!example_kset)
		return -ENOMEM;

	foo_obj = create_foo_obj("foo");
	if (!foo_obj)
		goto foo_error;

	bar_obj = create_foo_obj("bar");
	if (!bar_obj)
		goto bar_error;

	baz_obj = create_foo_obj("baz");
	if (!baz_obj)
		goto baz_error;

	return 0;

baz_error:
	destroy_foo_obj(bar_obj);
bar_error:
	destroy_foo_obj(foo_obj);
foo_error:
	kset_unregister(example_kset);
	return -EINVAL;
}

static void __exit ramdisk_exit(void)
{
	destroy_foo_obj(baz_obj);
	destroy_foo_obj(bar_obj);
	destroy_foo_obj(foo_obj);
	kset_unregister(example_kset);
}

module_init(ramdisk_init);
module_exit(ramdisk_exit);

MODULE_AUTHOR("Gnekiah Hsiung <gnekiah@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UFS host controller ramdisk driver");