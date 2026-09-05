#include "idt.h"

struct idt_entry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char  ist;
    unsigned char  type_attr;
    unsigned short offset_mid;
    unsigned int   offset_high;
    unsigned int   reserved;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtr;

extern void idt_load(struct idt_ptr *idtr);
extern void dummy_interrupt(void);
extern void dummy_exception(void);
extern void int50_handler(void);
extern void keyb_handler(void);

static void idt_set_entry(
    int vector,
    unsigned long long handler,
    unsigned char type_attr
)
{
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

void idt_init(void)
{
    for (int i = 0; i < 256; i++) {
        idt_set_entry(
            i,
            (unsigned long long)dummy_interrupt,
            0x8E
        );
        idt_set_entry(
            0x50,
            (unsigned long long)int50_handler,
            0xEE
        );
        idt_set_entry(
            0x21,
            (unsigned long long)keyb_handler,
            0x8E
        );
    }

    /*
     * CPU exceptions.
     * These use a separate handler because some exceptions
     * push an error code onto the stack.
     */
    idt_set_entry(0, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(1, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(2, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(3, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(4, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(5, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(6, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(7, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(8, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(9, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(10, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(11, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(12, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(13, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(14, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(15, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(16, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(17, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(18, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(19, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(20, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(21, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(22, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(23, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(24, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(25, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(26, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(27, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(28, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(29, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(30, (unsigned long long)dummy_exception, 0x8E);
    idt_set_entry(31, (unsigned long long)dummy_exception, 0x8E);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (unsigned long long)&idt;

    idt_load(&idtr);
}
