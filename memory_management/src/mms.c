#include "allocator.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergii Pisklov");
MODULE_DESCRIPTION("Allocation max size test");
MODULE_VERSION("0.1");


static char* allocator = "kmalloc";

module_param(allocator, charp, 0);
MODULE_PARM_DESC(allocator, " kmalloc, kzalloc, vmalloc or kmem_cache");


#define MODULE_NAME KBUILD_MODNAME


static int __init mms_init(void)
{
	void *mem;
	struct allocator a;
	size_t max_size = 0;
	size_t last_size = 0;
	u64 size = 1024 * 1024 * 1024; //GB
	size_t step = size;

	factory_t factory = find_factory(allocator);
	if (!factory)
		return -ENODATA;

	factory(&a);

	while (step >= 128) {
		mem = a.ator(size);
		last_size = size;

		if (mem) {
			printk(KERN_INFO MODULE_NAME ": alloc size is %llu\n", size);
			a.dtor(mem);
			max_size = size;
			size += step;

			if (size >= SIZE_MAX) {
				printk(KERN_INFO MODULE_NAME ": %u bytes reached\n", SIZE_MAX);
				return 0;
			}
		} else {
			printk(KERN_INFO MODULE_NAME ": cant alloc mem of size %llu\n", size);
			step /= 2;
			size -= step;
		}
	}

	printk(KERN_INFO MODULE_NAME ": max allocated size is %u bytes, allocator '%s'\n", max_size, allocator);
	return 0;
}


static void __exit mms_exit(void)
{}


module_init(mms_init);
module_exit(mms_exit);
