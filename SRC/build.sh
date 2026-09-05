#!/bin/bash

set -e

rm -f *.o BOOTX64.EFI kernel.so test.img

CFLAGS="
    -ffreestanding
    -fno-stack-protector
    -fpic
    -fshort-wchar
    -mno-red-zone
    -mno-sse
    -mno-mmx
    -DEFI_FUNCTION_WRAPPER
    -I/usr/include/efi
    -I/usr/include/efi/x86_64
    -fexec-charset=ASCII
"

echo "[1/5] Compiling C..."

gcc $CFLAGS -c kernel.c  -o kernel.o
gcc $CFLAGS -c gop.c     -o gop.o
gcc $CFLAGS -c video.c   -o video.o
gcc $CFLAGS -c gdt.c     -o gdt.o
gcc $CFLAGS -c idt.c     -o idt.o
gcc $CFLAGS -c int50.c   -o int50.o
gcc $CFLAGS -c memory.c  -o memory.o
gcc $CFLAGS -c portio.c  -o portio.o
gcc $CFLAGS -c pic.c     -o pic.o
gcc $CFLAGS -c keybh.c   -o keybh.o
gcc $CFLAGS -c fat32.c   -o fat32.o
gcc $CFLAGS -c ata.c     -o ata.o

echo "[2/5] Assembling..."

nasm -f elf64 gdt.asm -o gdtasm.o
nasm -f elf64 idt.asm -o idtasm.o

echo "[3/5] Linking..."

ld -nostdlib -znocombreloc \
    -T /usr/lib/elf_x86_64_efi.lds \
    /usr/lib/crt0-efi-x86_64.o \
    kernel.o \
    gop.o \
    video.o \
    gdtasm.o \
    gdt.o \
    idtasm.o \
    idt.o \
    int50.o \
    memory.o \
    keybh.o \
    portio.o \
    pic.o \
    ata.o \
    fat32.o \
    -shared -Bsymbolic \
    -L/usr/lib \
    -lefi \
    -lgnuefi \
    -o kernel.so

echo "[4/6] Assembling test program..."

nasm -f bin test.asm -o test.bin

echo "[5/6] Creating EFI..."

objcopy \
    -j .text \
    -j .sdata \
    -j .data \
    -j .rodata \
    -j .dynamic \
    -j .dynsym \
    -j .rel \
    -j .rela \
    -j .reloc \
    --target=efi-app-x86_64 \
    kernel.so \
    BOOTX64.EFI

echo "[6/6] Creating FAT32 image..."

dd if=/dev/zero of=test.img bs=1M count=64 status=none

mkfs.fat -F 32 test.img >/dev/null

sudo mkdir -p mnt
sudo mount -o loop test.img mnt

sudo mkdir -p mnt/EFI/BOOT

sudo cp BOOTX64.EFI mnt/EFI/BOOT/BOOTX64.EFI

sudo cp test.bin mnt/test.bin

sudo umount mnt

echo ""
echo "Build complete!"
echo "BOOTX64.EFI created."
echo "test.bin created and copied to the FAT32 root."
