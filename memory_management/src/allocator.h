#ifndef __MM_ALLOCATOR_H__
#define __MM_ALLOCATOR_H__

#include <linux/types.h>


struct allocator {
	void* (*ator)(size_t size);
	void (*dtor)(const void* ptr);
};

typedef void (*factory_t)(struct allocator* ator);

struct ator_factory {
	const char* const name;
	factory_t factory;
};


factory_t find_factory(const char* name);

#endif // __MM_ALLOCATOR_H__
