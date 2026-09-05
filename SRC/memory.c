#include <efi.h>
#include "kmem.h"
#include "memory.h"

EFI_MEMORY_DESCRIPTOR *cdesc;
void malloc(UINT64 size, UINT64 retaddr) {
    UINT64 ffree;
    for (int i = 0; i < memory_map_size / descriptor_size; i++)
    {
        if (cdesc->Type == EfiConventionalMemory && cdesc->NumberOfPages * 4096 >= size)
        {
            ffree = cdesc->PhysicalStart;
            retaddr = ffree;
            ffree += size;
        }
        cdesc++;
    }
    return;
}
