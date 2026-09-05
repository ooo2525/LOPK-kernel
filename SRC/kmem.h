#ifndef KMEM_H
#define KMEM_H

#include <efi.h>

extern UINTN memory_map_size;
extern EFI_MEMORY_DESCRIPTOR *memory_map;
extern UINTN map_key;
extern UINTN descriptor_size;
extern UINT32 descriptor_version;

#endif
