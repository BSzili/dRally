#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drmemory.h"

void ___58b20h(unsigned int err_n, ...);

__POINTER__ dRMemory_alloc(size_t size){

    __POINTER__          alloc;

	//kprintf("%s(%ld)\n", __FUNCTION__, size);

    if((alloc = malloc(size)) == (__POINTER__)0){

        ___58b20h(0xd);
    }

    return alloc;
}

void dRMemory_free(__POINTER__ mem){

	if (mem) {
		free(mem);
	}
}

__POINTER__ dRMemory_resize(__POINTER__ mem, size_t size){

    return mem;
}

void dRally_Memory_init(void){

}

void dRally_Memory_clean(void){

}
