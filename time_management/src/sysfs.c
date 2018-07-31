#include "config.h"
#include "sysfs.h"
#include "tm_util.h"

#include <linux/device.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>


struct sysfs_nodes {
	struct class *fs_class;
	struct class_attribute sslr_attr;
	struct class_attribute tslr_attr;
	struct class_attribute sv_attr;
};

typedef ssize_t (*show_callback_t)(struct class *class, struct class_attribute *attr, char *buf);


static ssize_t denied_store(struct class *class, struct class_attribute *attr, const char *buf, size_t count)
{
	return -EPERM;
}


static ssize_t show_sslr(struct class *class, struct class_attribute *attr, char *buf)
{
	return val_to_str(get_seconds_since_last_read(), buf, 64);
}


static ssize_t show_tslr(struct class *class, struct class_attribute *attr, char *buf)
{
	struct tm at;
	int err = get_absolute_time_since_last_read(&at);
	return (0 == err) ? date_to_str(&at, buf, 64) : err;
}


static ssize_t show_sv(struct class *class, struct class_attribute *attr, char *buf)
{
	return val_to_str(get_next_in_sequence(), buf, 64);
}


static void init_attr(const char *name, show_callback_t cb, struct class_attribute *attr)
{
	attr->attr.mode = S_IRUGO;
	attr->attr.name = name;
	attr->show = cb;
	attr->store = &denied_store;
}


static struct sysfs_nodes *nodes = NULL;


int create_sysfs_records(void)
{
	int res;
	WARN_ON(nodes);

	do {
		nodes = kmalloc(sizeof(struct sysfs_nodes), GFP_KERNEL);

		if (!nodes) {
			printk(KERN_ERR MODULE_NAME ": sysfs initialization failed\n");
			return -ENOMEM;
		}

		nodes->fs_class = class_create(THIS_MODULE, MODULE_NAME);

		if (IS_ERR(nodes->fs_class)) {
			printk(KERN_ERR MODULE_NAME ":bad sys fs class\n");
			res = -ENOMEM;
			break;
		}

		init_attr(SSLR_NAME, &show_sslr, &nodes->sslr_attr);
		res = class_create_file(nodes->fs_class, &nodes->sslr_attr);
		if (res) {
			printk(KERN_ERR MODULE_NAME ":failed to create sysfs " SSLR_NAME " file\n");
			break;
		}

		init_attr(TSLR_NAME, &show_tslr, &nodes->tslr_attr);
		res = class_create_file(nodes->fs_class, &nodes->tslr_attr);
		if (res) {
			printk(KERN_ERR MODULE_NAME ":failed to create sysfs " TSLR_NAME " file\n");
			break;
		}

		init_attr(SV_NAME, &show_sv, &nodes->sv_attr);
		res = class_create_file(nodes->fs_class, &nodes->sv_attr);
		if (res) {
			printk(KERN_ERR MODULE_NAME ":failed to create sysfs " SV_NAME " file\n");
			break;
		}

		return 0;
	} while (0);

	remove_sysfs_records();
	return res;
}


void remove_sysfs_records(void)
{
	if (!nodes)
		return;

	class_remove_file(nodes->fs_class, &nodes->sv_attr);
	class_remove_file(nodes->fs_class, &nodes->tslr_attr);
	class_remove_file(nodes->fs_class, &nodes->sslr_attr);
	class_destroy(nodes->fs_class);

	kfree(nodes);
	nodes = NULL;
}
