#include "fat32.h"
#include "ata.h"

#include <stdint.h>

static uint16_t read16(const uint8_t *p)
{
    return
        (uint16_t)p[0] |
        ((uint16_t)p[1] << 8);
}

static uint32_t read32(const uint8_t *p)
{
    return
        (uint32_t)p[0] |
        ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) |
        ((uint32_t)p[3] << 24);
}

static int string_equal_11(
    const uint8_t *entry,
    const char *name
)
{
    int i;

    for (i = 0; i < 11; i++) {
        if ((char)entry[i] != name[i])
            return 0;
    }

    return 1;
}

static uint32_t cluster_to_sector(
    FAT32_FS *fs,
    uint32_t cluster
)
{
    return
        fs->data_start +
        ((cluster - 2) * fs->sectors_per_cluster);
}

static uint32_t fat_next_cluster(
    FAT32_FS *fs,
    uint32_t cluster
)
{
    uint8_t sector[512];
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t offset;

    fat_offset = cluster * 4;

    fat_sector =
        fs->fat_start +
        (fat_offset / fs->bytes_per_sector);

    offset =
        fat_offset %
        fs->bytes_per_sector;

    if (ata_read_sector(fat_sector, sector) != 0)
        return 0x0FFFFFFF;

    return read32(&sector[offset]) & 0x0FFFFFFF;
}

int fat32_init(
    FAT32_FS *fs
)
{
    uint8_t sector[512];
    uint8_t *bpb;

    uint32_t partition_start;

    if (!fs)
        return -1;

    /*
     * Read MBR.
     */
    if (ata_read_sector(0, sector) != 0)
        return -1;

    /*
     * MBR signature.
     */
    if (sector[510] != 0x55 ||
        sector[511] != 0xAA)
        return -1;

    /*
     * Use partition 0.
     */
    partition_start =
        read32(&sector[446 + 8]);

    fs->partition_start = partition_start;

    /*
     * Read FAT32 boot sector.
     */
    if (ata_read_sector(partition_start, sector) != 0)
        return -1;

    bpb = sector;

    /*
     * FAT32 normally uses 512-byte sectors.
     */
    fs->bytes_per_sector =
        read16(&bpb[11]);

    fs->sectors_per_cluster =
        bpb[13];

    fs->reserved_sectors =
        read16(&bpb[14]);

    fs->fat_count =
        bpb[16];

    fs->sectors_per_fat =
        read32(&bpb[36]);

    fs->root_cluster =
        read32(&bpb[44]);

    if (fs->bytes_per_sector != 512)
        return -1;

    if (fs->sectors_per_cluster == 0)
        return -1;

    if (fs->fat_count == 0)
        return -1;

    /*
     * FAT starts after reserved sectors.
     */
    fs->fat_start =
        partition_start +
        fs->reserved_sectors;

    /*
     * Data starts after all FATs.
     */
    fs->data_start =
        fs->fat_start +
        (
            fs->fat_count *
            fs->sectors_per_fat
        );

    return 0;
}

int fat32_find_file(
    FAT32_FS *fs,
    const char *name,
    uint32_t *first_cluster,
    uint32_t *file_size
)
{
    uint8_t sector[512];

    uint32_t cluster;
    uint32_t sector_number;

    int i;

    if (!fs ||
        !name ||
        !first_cluster ||
        !file_size)
        return -1;

    cluster = fs->root_cluster;

    while (cluster < 0x0FFFFFF8) {

        sector_number =
            cluster_to_sector(fs, cluster);

        for (i = 0;
             i < fs->sectors_per_cluster;
             i++) {

            uint32_t lba =
                sector_number + i;

            int entry;

            if (ata_read_sector(lba, sector) != 0)
                return -1;

            for (entry = 0;
                 entry < 16;
                 entry++) {

                uint8_t *dir =
                    &sector[entry * 32];

                uint8_t first =
                    dir[0];

                uint8_t attributes =
                    dir[11];

                uint32_t high;
                uint32_t low;

                /*
                 * End of directory.
                 */
                if (first == 0x00)
                    return -1;

                /*
                 * Deleted entry.
                 */
                if (first == 0xE5)
                    continue;

                /*
                 * Long filename entry.
                 */
                if (attributes == 0x0F)
                    continue;

                /*
                 * Directory.
                 */
                if (attributes & 0x10)
                    continue;

                if (!string_equal_11(dir, name))
                    continue;

                high =
                    (uint32_t)read16(&dir[20]);

                low =
                    (uint32_t)read16(&dir[26]);

                *first_cluster =
                    (high << 16) | low;

                *file_size =
                    read32(&dir[28]);

                return 0;
            }
        }

        cluster =
            fat_next_cluster(fs, cluster);
    }

    return -1;
}

int fat32_read_file(
    FAT32_FS *fs,
    const char *name,
    void *buffer,
    uint32_t buffer_size
)
{
    uint32_t cluster;
    uint32_t file_size;

    uint32_t remaining;
    uint32_t offset;

    uint8_t sector[512];

    if (!fs || !name || !buffer)
        return -1;

    if (fat32_find_file(
            fs,
            name,
            &cluster,
            &file_size) != 0)
        return -1;

    if (file_size > buffer_size)
        return -1;

    remaining = file_size;
    offset = 0;

    while (remaining > 0 &&
           cluster < 0x0FFFFFF8) {

        uint32_t first_sector =
            cluster_to_sector(fs, cluster);

        uint32_t s;

        for (s = 0;
             s < fs->sectors_per_cluster &&
             remaining > 0;
             s++) {

            uint32_t copy_size;

            if (ata_read_sector(
                    first_sector + s,
                    sector) != 0)
                return -1;

            copy_size = remaining;

            if (copy_size > 512)
                copy_size = 512;

            {
                uint8_t *dst =
                    (uint8_t *)buffer + offset;

                uint32_t i;

                for (i = 0; i < copy_size; i++)
                    dst[i] = sector[i];
            }

            offset += copy_size;
            remaining -= copy_size;
        }

        if (remaining == 0)
            break;

        cluster =
            fat_next_cluster(fs, cluster);
    }

    if (remaining != 0)
        return -1;

    return (int)file_size;
}
