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

struct wtf_attribute {
	struct attribute attr;
	ssize_t (*show)(struct ramufs *ru, struct wtf_attribute *attr, char *buf);
	ssize_t (*store)(struct ramufs *ru, struct wtf_attribute *attr, const char *buf, size_t cnt);
};

#define attr_to_wtf(x) container_of(x, struct wtf_attribute, attr)

static ssize_t wtf_attr_show(struct kobject *kobj,
				struct attribute *attr, char *buf) 
{
	struct wtf_attribute *attribute;
	struct ramufs *ru;

	attribute = attr_to_wtf(attr);
	ru = kobj_to_ramufs(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(ru, attribute, buf);
}

static ssize_t wtf_attr_store(struct kobject *kobj,
				struct attribute *attr,
				const char *buf, size_t len)
{
	struct wtf_attribute *attribute;
	struct ramufs *ru;

	attribute = attr_to_wtf(attr);
	ru = kobj_to_ramufs(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(ru, attribute, buf, len);
}

static const struct sysfs_ops wtf_sysfs_ops = {
	.show = wtf_attr_show,
	.store = wtf_attr_store,
};

static void wtf_release(struct kobject *kobj)
{
	struct ramufs *ru;

	ru = kobj_to_ramufs(kobj);
	kfree(ru);
}

static ssize_t wtf_show(struct ramufs *ru, struct wtf_attribute *attr，
				char *buf)
{
	return sprintf(buf, "%d\n", ru->wtf);
}

static ssize_t wtf_store(struct ramufs *ru, struct attribute *attr,
				const char *buf, size_t cnt)
{
	int ret;

	ret = kstrtoint(buf, 10, &ru->wtf);
	if (ret < 0)
		return ret;
	return cnt;
}

static struct wtf_attribute wtf_attribute = 
	__ATTR(wtf, 0664, wtf_show, wtf_store);

static struct attribute *wtf_default_attrs[] = {
	&wtf_attribute.attr,
	NULL,
};

static struct kobj_type wtf_ktype = {
	.sysfs_ops = &wtf_sysfs_ops,
	.release = wtf_release,
	.default_attrs = wtf_default_attrs,
};

static struct kset *ramufs_kset;
static struct ramufs *ru;

static struct ramufs *create_wtf_obj(const char* name)
{
	struct ramufs *ru;
	int retval;
	
	ru = kzalloc(sizeof(*ru), GFP_KERNEL);
	if (!ru) 
		return NULL;

	ru->kobj.kset = ramufs_kset;
	retval = kobject_init_and_add(&ru->kobj, &wtf_ktype, NULL, "%s", name);
	if (retval) {
		kobject_put(&ru->kobj);
		return NULL;
	}
	kobject_uevent(&ru->kobj, KOBJ_ADD);
	
	return ru;
}

static void destroy_ramufs_obj(struct ramufs *ru)
{
	kobject_put(&ru->kobj);
}

static int __init ramufs_init(void)
{
	ramufs_kset = kset_create_and_add("ramufs_kset", NULL, kernel_kobj);
	if (!ramufs_kset)
		return -ENOMEM;

	foo_obj = create_wtf_obj("wtf");
	if (!foo_obj)
		goto foo_error;

	return 0;

foo_error:
	kset_unregister(ramufs_kset);
	return -EINVAL;
}

static void __exit ramufs_exit(void)
{
	destroy_ramufs_obj(ru);
	kset_unregister(ramufs_kset);
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
