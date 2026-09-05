#include "video.h"
#include "memory.h"
#include "portio.h"
#include "ata.h"
#include "fat32.h"

extern UINT64 PRAX;
extern UINT64 PRDI;
extern UINT64 PRSI;
extern UINT64 PRBX;
extern UINT64 PRCX;
extern UINT64 PRDX;

extern FAT32_FS fs;

void int50(void) {
    //video
    if (PRAX == 0)
    {
        clear(framebuffer, width, height, (UINT32)PRSI, pixels_per_scanline);
    }
    else if (PRAX == 1)
    {
        putp(framebuffer, (UINT32)PRSI, PRBX, PRCX, pixels_per_scanline);
    }
    else if (PRAX == 2)
    {
        print_char(PRDI, (UINT32)PRSI, PRBX, PRCX, PRDX);
    }
    else if (PRAX == 3)
    {
        char *str = (char *)PRDI;
        printf(str, (UINT32)PRSI, PRBX, PRCX, PRDX);
    }
    else if (PRAX == 4)
    {
        PRDI = height;
        PRSI = width;
        PRBX = (UINT64)framebuffer;
        PRCX = pixels_per_scanline;
    }
    //end of video

    //memory
    else if (PRAX == 5)
    {
        malloc(PRDI, PRAX);
    }
    //end of memory

    //port i/o
    else if (PRAX == 6)
    {
        pout((unsigned char)PRDI, (unsigned char)PRSI);
    }
    else if (PRAX == 7)
    {
        PRAX = pin(PRDI);
    }
    else if (PRAX == 8)
    {
        PRAX = pin16(PRDI);
    }
    else if (PRAX == 9)
    {
        pout16((unsigned short)PRDI, (unsigned short)PRSI);
    }
    //end of port i/o

    //ata
    else if (PRAX == 10)
    {
        ata_init();
    }
    else if (PRAX == 11)
    {
        PRAX = ata_read_sector(
            (UINT32)PRDI,
            (void *)PRSI
        );
    }
    else if (PRAX == 12)
    {
        PRAX = ata_write_sector(
            (UINT32)PRDI,
            (void *)PRSI
        );
    }
    //end of ata

    //fat 32
    else if (PRAX == 13)
    {
        PRAX = fat32_init(&fs);
    }
    else if (PRAX == 14)
    {
        PRAX = fat32_find_file(
            &fs,
            (const char *)PRDI,
            (UINT32 *)PRSI,
            (UINT32 *)PRBX
        );
    }
    else if (PRAX == 15)
    {
        PRAX = fat32_read_file(
            &fs,
            (const char *)PRDI,
            (void *)PRSI,
            (UINT32)PRBX
        );
    }
    //end of fat 32
}
