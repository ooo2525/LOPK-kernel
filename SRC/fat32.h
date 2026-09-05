#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

typedef struct {
    uint32_t partition_start;

    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;

    uint32_t reserved_sectors;
    uint32_t fat_count;
    uint32_t sectors_per_fat;

    uint32_t fat_start;
    uint32_t data_start;

    uint32_t root_cluster;
} FAT32_FS;

int fat32_init(
    FAT32_FS *fs
);

int fat32_find_file(
    FAT32_FS *fs,
    const char *name,
    uint32_t *first_cluster,
    uint32_t *file_size
);

int fat32_read_file(
    FAT32_FS *fs,
    const char *name,
    void *buffer,
    uint32_t buffer_size
);

#endif
