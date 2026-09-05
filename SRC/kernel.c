#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include "video.h"
#include "gop.h"
#include "gdt.h"
#include "idt.h"
#include "kmem.h"
#include "memory.h"
#include "pic.h"
#include "uefi.h"
#include "ata.h"
#include "fat32.h"

//structs

//assembly functions

//memory variables
UINTN memory_map_size = 0;
EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
UINTN map_key;
UINTN descriptor_size;
UINT32 descriptor_version;

EFI_STATUS
efi_main(EFI_HANDLE image, EFI_SYSTEM_TABLE *st)
{
    InitializeLib(image, st);

    //for fs and uefi
    efi_image = image;
    efi_st = st;
    efi_bs = st->BootServices;

    //check GOP
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    EFI_STATUS status = uefi_call_wrapper(
        st->BootServices->LocateProtocol,
        3,
        &gop_guid,
        NULL,
        (void **)&gop
    );

    if (EFI_ERROR(status)) {
        printf("GOP ERROR", 0xFFFFFFFF, 0, 0, 2); //not sure if this works if GOP is bad because its too early
        while (1);
    }

    //assign video functions here so compiling works
    framebuffer = (UINT32 *)gop->Mode->FrameBufferBase;

    width = gop->Mode->Info->HorizontalResolution;
    height = gop->Mode->Info->VerticalResolution;
    pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

    //clear
    clear(framebuffer, width, height, 0x00000000,pixels_per_scanline);

    printf("initializing ...", 0xFFFFFFFF, 0, 0, 2);

    //memory map
    clear(framebuffer, width, height, 0x00000000,pixels_per_scanline);

    status = uefi_call_wrapper(
        st->BootServices->GetMemoryMap,
        5,
        &memory_map_size,
        memory_map,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );

    if(status == EFI_BUFFER_TOO_SMALL)
    {
        printf("buffer init", 0xFFFFFFFF, 0, 0, 2);
    }
    
    memory_map_size += 2 * sizeof(EFI_MEMORY_DESCRIPTOR);
        status = uefi_call_wrapper(
        st->BootServices->AllocatePool,
        3,
        EfiLoaderData,
        memory_map_size,
        (void **)&memory_map
    );

    if (EFI_ERROR(status))
    {
        printf("ALLOCATION ERROR", 0xFFFFFFFF, 0, 0, 2);
    }

    status = uefi_call_wrapper(
        st->BootServices->GetMemoryMap,
        5,
        &memory_map_size,
        memory_map,
        &map_key,
        &descriptor_size,
        &descriptor_version
    );

    if(status == EFI_BUFFER_TOO_SMALL)
    {
        printf("buffer init error", 0xFFFFFFFF, 0, 0, 2);
    }
    else
    {
        clear(framebuffer, width, height, 0x00000000,pixels_per_scanline);
    }

    if (EFI_ERROR(status))
    {
        printf("memory map error", 0xFFFFFFFF, 0, 0, 2);
    }
    else
    {
        printf("memory map ok", 0xFFFFFFFF, 0, 0, 2);
    }

    // gdt
    clear(framebuffer, width, height, 0x00000000, pixels_per_scanline);
    printf("gdt init", 0xFFFFFFFF, 0, 0, 2);
    gdt_init();

    //idt 
    clear(framebuffer, width, height, 0x00000000, pixels_per_scanline);
    printf("idt init", 0xFFFFFFFF, 0, 0, 2);
    idt_init();

    //memory
    clear(framebuffer, width, height, 0x00000000, pixels_per_scanline);
    printf("memory init", 0xFFFFFFFF, 0, 0, 2);
    cdesc = memory_map;

    //pic
    clear(framebuffer, width, height, 0x00000000, pixels_per_scanline);
    printf("pic init", 0xFFFFFFFF, 0, 0, 2);
    pic_init();


    // fat 32
    clear(framebuffer, width, height, 0x00000000, pixels_per_scanline);
    printf("filesystem and disk init", 0xFFFFFFFF, 0, 0, 2);

    FAT32_FS fs;

    ata_init();

    if (fat32_init(&fs) != 0) {
        printf("fs error", 0xFFFFFFFF, 50, 50, 2);
        while (1) {
            __asm__ volatile ("hlt");
        }
    }

    uint32_t file_size;
    uint32_t first_cluster;

/*
 * FAT32 uses the 8.3 directory representation:
 *
 * "TEST    BIN"
 */
    if (fat32_find_file(
        &fs,
        "TEST    BIN",
        &first_cluster,
        &file_size
    ) != 0)
{
    printf("file not found", 0xFFFFFFFF, 50, 100, 2);

    while (1) {
        __asm__ volatile ("hlt");
    }
}

/*
 * Allocate enough pages for the file.
 */
    UINTN pages =
        (file_size + 0xFFF) / 0x1000;

    EFI_PHYSICAL_ADDRESS address =
        0xFFFFFFFFFFFFFFFFULL;

    status =
        uefi_call_wrapper(
            efi_bs->AllocatePages,
            4,
            AllocateMaxAddress,
            EfiLoaderCode,
            pages,
            &address
        );

    if (EFI_ERROR(status)) {
        printf("alloc error", 0xFFFFFFFF, 50, 150, 2);

        while (1) {
            __asm__ volatile ("hlt");
        }
    }

    /*
    * Read the file directly into the allocated memory.
    */
    if (fat32_read_file(
        &fs,
        "TEST    BIN",
        (void *)(UINTN)address,
        file_size
    ) < 0)
{
    printf("read error", 0xFFFFFFFF, 50, 200, 2);

    while (1) {
        __asm__ volatile ("hlt");
    }
}

    printf("running test", 0xFFFFFFFF, 50, 250, 2);

/*
 * Jump directly to the raw binary.
 */
    __asm__ volatile (
        "jmp *%0"
        :
        : "r"((UINTN)address)
    );

__builtin_unreachable();

    //sti
     __asm__ volatile ("sti");

    //infinite loop
    while (1) {
        __asm__ volatile ("hlt");
    }
}
