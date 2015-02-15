#include "mempool.h"

extern "C" {
#include <sys/mman.h>
}

MemPool::MemPool(unsigned long long size, unsigned long objsize)
{
	poolSize = size;
	objSize = objsize;
	next = NULL;
	if (!newmap()) {
		used = 0;
		poolSize = 0;
	}
}

MemPool::~MemPool()
{
	int i;
	int len = exhaustList.size();
	for (i = 0; i < len; i++) {
		munmap(exhaustList[i], poolSize);
	}
}
