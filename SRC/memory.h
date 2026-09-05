#ifndef MEMORY_H
#define MEMORY_H

extern EFI_MEMORY_DESCRIPTOR *cdesc;
void malloc(UINT64 size, UINT64 retaddr);

#endif