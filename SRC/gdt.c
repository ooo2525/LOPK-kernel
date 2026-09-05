#include "gdt.h"

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_mid;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed));

/*
 * GDT layout:
 *
 * 0x00  Null
 * 0x08  Kernel code
 * 0x10  Kernel data
 * 0x18  User code
 * 0x20  User data
 *
 * 0x28+ Reserved for TSS
 */
static struct gdt_entry gdt[5];
static struct gdt_ptr gdtr;

extern void gdt_load(struct gdt_ptr *gdtr);

static void gdt_set_entry(
    int index,
    unsigned int base,
    unsigned int limit,
    unsigned char access,
    unsigned char granularity
)
{
    gdt[index].base_low = base & 0xFFFF;
    gdt[index].base_mid = (base >> 16) & 0xFF;
    gdt[index].base_high = (base >> 24) & 0xFF;

    gdt[index].limit_low = limit & 0xFFFF;

    gdt[index].granularity =
        ((limit >> 16) & 0x0F) |
        (granularity & 0xF0);

    gdt[index].access = access;
}

void gdt_init(void)
{
    /* Null descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);

    /* Kernel code: 0x08 */
    gdt_set_entry(
        1,
        0,
        0xFFFFF,
        0x9A,
        0xA0
    );

    /* Kernel data: 0x10 */
    gdt_set_entry(
        2,
        0,
        0xFFFFF,
        0x92,
        0xC0
    );

    /* User code: 0x18 */
    gdt_set_entry(
        3,
        0,
        0xFFFFF,
        0xFA,
        0xA0
    );

    /* User data: 0x20 */
    gdt_set_entry(
        4,
        0,
        0xFFFFF,
        0xF2,
        0xC0
    );

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (unsigned long long)&gdt;

    gdt_load(&gdtr);
}
