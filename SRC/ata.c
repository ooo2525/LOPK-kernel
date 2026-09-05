#include "ata.h"
#include "portio.h"

static void ata_delay(void)
{
    /*
     * Reading the alternate status register
     * four times gives approximately 400 ns.
     */
    pin(ATA_PRIMARY_CONTROL);
    pin(ATA_PRIMARY_CONTROL);
    pin(ATA_PRIMARY_CONTROL);
    pin(ATA_PRIMARY_CONTROL);
}

static int ata_wait_bsy(void)
{
    unsigned char status;

    for (;;) {
        status = pin(ATA_PRIMARY_IO + 7);

        if (!(status & ATA_STATUS_BSY))
            break;
    }

    return 0;
}

static int ata_wait_drq(void)
{
    unsigned char status;

    for (;;) {
        status = pin(ATA_PRIMARY_IO + 7);

        if (status & ATA_STATUS_ERR)
            return -1;

        if (status & ATA_STATUS_DRQ)
            return 0;
    }
}

void ata_init(void)
{
    /*
     * Select primary master.
     */
    pout(ATA_PRIMARY_IO + 6, 0xA0);

    ata_delay();

    /*
     * Software reset / initial status read.
     */
    pin(ATA_PRIMARY_IO + 7);
}

int ata_read_sector(
    uint32_t lba,
    void *buffer
)
{
    uint16_t *dest = (uint16_t *)buffer;
    unsigned char status;
    int i;

    if (!buffer)
        return -1;

    /*
     * LBA28 supports up to 128 GiB with 512-byte sectors.
     */
    if (lba > 0x0FFFFFFF)
        return -1;

    /*
     * Select master + high 4 bits of LBA.
     */
    pout(
        ATA_PRIMARY_IO + 6,
        0xE0 | ((lba >> 24) & 0x0F)
    );

    /*
     * Sector count = 1.
     */
    pout(ATA_PRIMARY_IO + 2, 1);

    /*
     * LBA bits 0-7.
     */
    pout(
        ATA_PRIMARY_IO + 3,
        (unsigned char)(lba & 0xFF)
    );

    /*
     * LBA bits 8-15.
     */
    pout(
        ATA_PRIMARY_IO + 4,
        (unsigned char)((lba >> 8) & 0xFF)
    );

    /*
     * LBA bits 16-23.
     */
    pout(
        ATA_PRIMARY_IO + 5,
        (unsigned char)((lba >> 16) & 0xFF)
    );

    /*
     * Read one sector.
     */
    pout(
        ATA_PRIMARY_IO + 7,
        ATA_CMD_READ_SECTORS
    );

    ata_delay();

    /*
     * Wait until the device is ready.
     */
    ata_wait_bsy();

    status = pin(ATA_PRIMARY_IO + 7);

    if (status & ATA_STATUS_ERR)
        return -1;

    /*
     * Wait for data request.
     */
    if (ata_wait_drq() != 0)
        return -1;

    /*
     * Read 256 words = 512 bytes.
     */
    for (i = 0; i < 256; i++)
        dest[i] = pin16(ATA_PRIMARY_IO);

    return 0;
}

int ata_write_sector(
    uint32_t lba,
    const void *buffer
)
{
    const uint16_t *src = (const uint16_t *)buffer;
    unsigned char status;
    int i;

    if (!buffer)
        return -1;

    if (lba > 0x0FFFFFFF)
        return -1;

    /*
     * Select master + high 4 bits of LBA.
     */
    pout(
        ATA_PRIMARY_IO + 6,
        0xE0 | ((lba >> 24) & 0x0F)
    );

    /*
     * One sector.
     */
    pout(ATA_PRIMARY_IO + 2, 1);

    pout(
        ATA_PRIMARY_IO + 3,
        (unsigned char)(lba & 0xFF)
    );

    pout(
        ATA_PRIMARY_IO + 4,
        (unsigned char)((lba >> 8) & 0xFF)
    );

    pout(
        ATA_PRIMARY_IO + 5,
        (unsigned char)((lba >> 16) & 0xFF)
    );

    /*
     * Write command.
     */
    pout(
        ATA_PRIMARY_IO + 7,
        ATA_CMD_WRITE_SECTORS
    );

    ata_delay();

    ata_wait_bsy();

    if (ata_wait_drq() != 0)
        return -1;

    /*
     * Write 512 bytes.
     */
    for (i = 0; i < 256; i++)
        pout16(ATA_PRIMARY_IO, src[i]);

    /*
     * Wait until the drive finishes.
     */
    ata_wait_bsy();

    status = pin(ATA_PRIMARY_IO + 7);

    if (status & ATA_STATUS_ERR)
        return -1;

    return 0;
}
