#include "allocator.h"

#include <linux/slab.h>


static void* kmalloc_ator(size_t size)
{
	return kmalloc(size, GFP_KERNEL);
}

static void* kzalloc_ator(size_t size)
{
	return kzalloc(size, GFP_KERNEL);
}

static void kfree_dtor(const void* ptr)
{
	kfree(ptr);
}


static void* vmalloc_ator(size_t size)
{
	return vmalloc(size);
}

static void vfree_dtor(const void* ptr)
{
	vfree(ptr);
}


static void* kmem_cache_ator(size_t size)
{
	return kmem_cache_create("kmem_cache_test", size, 0, SLAB_HWCACHE_ALIGN, NULL);
}

static void kmem_cache_dtor(const void *ptr)
{
	kmem_cache_destroy((struct kmem_cache*) ptr);
}


static void kmalloc_factory(struct allocator * ator)
{
	ator->ator = &kmalloc_ator;
	ator->dtor = &kfree_dtor;
}


static void kzalloc_factory(struct allocator * ator)
{
	ator->ator = &kzalloc_ator;
	ator->dtor = &kfree_dtor;
}


static void vmalloc_factory(struct allocator *ator)
{
	ator->ator = &vmalloc_ator;
	ator->dtor = &vfree_dtor;
}


static void kmem_cache_factory(struct allocator *ator)
{
	ator->ator = &kmem_cache_ator;
	ator->dtor = &kmem_cache_dtor;
}


static struct ator_factory factories[] = {
	{"kmalloc", &kmalloc_factory},
	{"kmem_cache", &kmem_cache_factory},
	{"kzalloc", &kzalloc_factory},
	{"vmalloc", &vmalloc_factory},
};


factory_t find_factory(const char* name)
{
	const size_t fsize = sizeof(factories) / sizeof(struct ator_factory);
	size_t step = fsize / 2;
	int index = step - 1;
	int cmp = 0;

	WARN_ON(!fsize);

	while (step) {
		cmp = strcmp(name, factories[index].name);

		if (cmp > 0)
			index += step;
		else if (cmp > 0)
			index -= step;
		else
			return factories[index].factory;

		step /= 2;
	}

	return NULL;
}
