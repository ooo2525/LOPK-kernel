#ifndef ATA_H
#define ATA_H

#include <stdint.h>

#define ATA_PRIMARY_IO       0x1F0
#define ATA_PRIMARY_CONTROL  0x3F6

#define ATA_MASTER 0x00
#define ATA_SLAVE  0x10

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_STATUS_ERR  0x01

#define ATA_CMD_READ_SECTORS  0x20
#define ATA_CMD_WRITE_SECTORS 0x30
#define ATA_CMD_IDENTIFY      0xEC

void ata_init(void);

int ata_read_sector(
    uint32_t lba,
    void *buffer
);

int ata_write_sector(
    uint32_t lba,
    const void *buffer
);

#endif
