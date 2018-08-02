#include "allocator.h"

#include <linux/jiffies.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergii Pisklov");
MODULE_DESCRIPTION("Allocation speed test");
MODULE_VERSION("0.1");


static char* allocator = "kmalloc";
static unsigned long alloc_size = 128;

module_param(allocator, charp, 0);
module_param(alloc_size, ulong, 0);
MODULE_PARM_DESC(allocator, " kmalloc, kzalloc, vmalloc or kmem_cache");
MODULE_PARM_DESC(alloc_size, " size in bytes up to 4GB");


#define MODULE_NAME KBUILD_MODNAME


static int __init mmt_init(void)
{
	static const unsigned long count = 10000;
	size_t index = 0;
	unsigned long jiffies_sum = 0 ,jf;
	unsigned long avr;
	void *mem;
	struct allocator a;
	factory_t factory;

	printk(KERN_DEBUG MODULE_NAME ": allocator='%s', alloc_size=%lu\n", allocator, alloc_size);

	factory = find_factory(allocator);
	if (!factory) {
		printk(KERN_DEBUG MODULE_NAME ": factory not found\n");
		return -ENODATA;
	}

	factory(&a);

	for (; index < count; ++index) {
		jf = jiffies;
		mem = a.ator(alloc_size);

		if (!mem) {
			printk(KERN_ERR MODULE_NAME ": can't allocate %lu bytes with %s\n", alloc_size, allocator);
			return -ENOMEM;
		}

		jiffies_sum += (jiffies - jf);
		a.dtor(mem);
	}

	printk(KERN_INFO MODULE_NAME ": total jiffies %lu\n", jiffies_sum);
	avr = jiffies_to_usecs(jiffies_sum) / count;
	printk(KERN_INFO MODULE_NAME ": it takes %lu us per allocation (block size is %lu, %lu allocations)\n", avr, alloc_size, count);

	return 0;
}


static void __exit mmt_exit(void)
{}


module_init(mmt_init);
module_exit(mmt_exit);
