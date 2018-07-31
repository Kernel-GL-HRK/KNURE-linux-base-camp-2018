#include "config.h"
#include "procfs.h"
#include "sysfs.h"
#include "tm.h"

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergii Pisklov");
MODULE_DESCRIPTION("Time management");
MODULE_VERSION("0.1");


static int __init tm_init(void)
{
	int res = create_procfs_records();
	if (0 != res) {
		printk(KERN_ERR MODULE_NAME "failed to create records in procfs\n");
		return res;
	}

	res = create_sysfs_records();

	if (0 != res) {
		printk(KERN_ERR MODULE_NAME "failed to create records in sysfs\n");
		return res;
	}

	res = init_sequence();

	if (0 != res) {
		printk(KERN_ERR MODULE_NAME "failed to initialize sequenece\n");
		return res;
	}

	return 0;
}


static void __exit tm_cleanup(void)
{
	destroy_sequence();
	remove_procfs_records();
	remove_sysfs_records();
}


module_init(tm_init);
module_exit(tm_cleanup);
