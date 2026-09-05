void pout(unsigned short port, unsigned char value) {
    __asm__ volatile (
        ".intel_syntax noprefix\n"
        "out dx, al\n"
        ".att_syntax\n"
        :
        : "d"(port), "a"(value)
        :
    );
}

unsigned char pin(unsigned short port) {
    unsigned char value;

    __asm__ volatile (
        ".intel_syntax noprefix\n"
        "in al, dx\n"
        ".att_syntax\n"
        : "=a"(value)
        : "d"(port)
        :
    );

    return value;
}

void pout16(unsigned short port, unsigned short value) {
    __asm__ volatile (
        ".intel_syntax noprefix\n"
        "out dx, ax\n"
        ".att_syntax\n"
        :
        : "d"(port), "a"(value)
        :
    );
}

unsigned short pin16(unsigned short port) {
    unsigned short value;

    __asm__ volatile (
        ".intel_syntax noprefix\n"
        "in ax, dx\n"
        ".att_syntax\n"
        : "=a"(value)
        : "d"(port)
        :
    );

    return value;
}
