#include "config.h"
#include "procfs.h"
#include "tm_util.h"

#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/uaccess.h>

#define BUF_SIZE (32)


struct node_fops {
	struct proc_dir_entry *file;
	struct file_operations ops;
};

struct procfs_nodes {
	struct proc_dir_entry *dir;
	struct node_fops sslr;
	struct node_fops tslr;
	struct node_fops sv;
};

typedef ssize_t (*node_read_callback_t)(struct file *fp, char __user *buffer, size_t size, loff_t *offset);


static struct procfs_nodes *nodes;


static ssize_t sslr_read(struct file *fp, char __user *buffer, size_t size, loff_t *offset)
{
	char buf[BUF_SIZE];
	int res = 0;

	if (*offset)
		return 0;

	res = val_to_str(get_seconds_since_last_read(), buf, BUF_SIZE);
	if (res < 1) {
		printk(KERN_ERR MODULE_NAME ": failed to prepare result\n");
		return res;
	}

	if (copy_to_user(buffer, buf, (size < res) ? size : res)) {
		printk(KERN_ERR MODULE_NAME ": failed to copy result to user\n");
		return -EIO;
	}

	*offset = res;
	return res;
}


static ssize_t tslr_read(struct file *fp, char __user *buffer, size_t size, loff_t *offset)
{
	struct tm at;
	int res = -1;

	char buf[BUF_SIZE] = {'0'};

	if (*offset)
		return 0;

	res = get_absolute_time_since_last_read(&at);
	if (res)
		return res;

	res = date_to_str(&at, buf, BUF_SIZE);

	if ( res < 1) {
		printk(KERN_ERR MODULE_NAME ": failed to prepare date\n");
		return -EIO;
	}

	if (copy_to_user(buffer, buf, (size < res) ? size : res)) {
		printk(KERN_ERR MODULE_NAME ": failed to copy result to user\n");
		return -EIO;
	}

	*offset = 1;
	return res;
}


static ssize_t sv_read(struct file *fp, char __user *buffer, size_t size, loff_t *offset)
{
	char buf[BUF_SIZE];
	int res = 0;

	if (*offset)
		return 0;

	res = val_to_str(get_next_in_sequence(), buf, BUF_SIZE);
	if (res < 1) {
		printk(KERN_ERR MODULE_NAME ": failed to prepare result\n");
		return res;
	}

	if (copy_to_user(buffer, buf, (size < res ) ? size : res)) {
		printk(KERN_ERR MODULE_NAME ": failed to copy result to user\n");
		return -EIO;
	}

	*offset = res;
	return res;
}


static int init_node_fops(const char* name, node_read_callback_t cb, struct node_fops *fops)
{
	fops->ops.read = cb;
	fops->file = proc_create(name, S_IFREG | S_IRUGO, nodes->dir, &fops->ops);

	return (NULL == fops->file)  ? -ENOMEM : 0;
}


static void cleanup_nodes(void)
{
	if (nodes) {
		if (nodes->sslr.file)
			remove_proc_entry(SSLR_NAME, nodes->dir);

		if (nodes->tslr.file)
			remove_proc_entry(TSLR_NAME, nodes->dir);

		if (nodes->sv.file)
			remove_proc_entry(SV_NAME, nodes->dir);

		if (nodes->dir)
			remove_proc_entry(MODULE_NAME, NULL);

		kfree(nodes);
		nodes = NULL;
	}
}


static int init_nodes(void)
{
	int err = 0;

	do {
		WARN_ON(nodes);
		nodes = kzalloc(sizeof(struct procfs_nodes), GFP_KERNEL);

		if (NULL == nodes)
			return -ENOMEM;

		nodes->dir = proc_mkdir(MODULE_NAME, NULL);

		if (NULL == nodes->dir) {
			err = -ENOMEM;
			break;
		}

		err = init_node_fops(SSLR_NAME, &sslr_read, &nodes->sslr);
		if (err)
			break;

		err = init_node_fops(TSLR_NAME, &tslr_read, &nodes->tslr);
		if (err)
			break;

		err = init_node_fops(SV_NAME, &sv_read, &nodes->sv);
		if (err)
			break;

		return 0;
	} while (0);

	cleanup_nodes();
	return err;
}


int create_procfs_records(void)
{
	return init_nodes();
}


void remove_procfs_records(void)
{
	WARN_ON(!nodes);
	cleanup_nodes();
}
