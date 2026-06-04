/*
 * mock_port.c -- virtual flash port implementation
 *
 * Simulates a real NOR flash:
 *   - Write  = AND mask (only clears bits, 1->0)
 *   - Erase  = fill with 0xFF (0->1)
 *   - Read   = memcpy
 */

#include <string.h>
#include <stdbool.h>
#include "mock_port.h"
#include "../cycleflash/cycle_flash_system.h"
#include "../cycleflash/cfs_port.h"

static uint8_t vflash[VFLASH_SIZE];

/* ---- helpers ---- */

void vflash_reset(void)
{
    memset(vflash, 0xFF, VFLASH_SIZE);
}

void vflash_fill(uint32_t addr, uint8_t byte, uint32_t len)
{
    uint32_t off = addr - VFLASH_BASE;
    for (uint32_t i = 0; i < len; i++)
        vflash[off + i] = byte;
}

uint8_t* vflash_ptr(uint32_t addr)
{
    return &vflash[addr - VFLASH_BASE];
}

uint32_t vflash_get_base(void)  { return VFLASH_BASE; }
uint32_t vflash_get_size(void)  { return VFLASH_SIZE; }

/* fault injection: flip a bit in the data area (safe: 1->0 via AND) */
void vflash_corrupt_byte(uint32_t addr)
{
    uint32_t off = addr - VFLASH_BASE;
    /* flip LSB of the first data byte (offset 8 = after header) */
    vflash[off + 8] &= 0xFEu;
}

/* fault injection: mark the block header as bad (all zeros in ID field) */
void vflash_mark_bad_block(uint32_t addr)
{
    uint32_t off = addr - VFLASH_BASE;
    /* write zeros to first 8 bytes (whole header) -- this is 1->0 so valid */
    for (int i = 0; i < 8; i++)
        vflash[off + i] = 0x00u;
}

/* ---- port write functions (len = number of UNITS, not bytes) ---- */

bool cfs_port_write_byte(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    uint32_t off = addr - VFLASH_BASE;
    for (uint16_t i = 0; i < len; i++)
        vflash[off + i] &= data[i];
    return true;
}

bool cfs_port_write_half_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    uint32_t off = addr - VFLASH_BASE;
    uint16_t bytes = len * 2u;
    for (uint16_t i = 0; i < bytes; i++)
        vflash[off + i] &= data[i];
    return true;
}

bool cfs_port_write_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    uint32_t off = addr - VFLASH_BASE;
    uint16_t bytes = len * 4u;
    for (uint16_t i = 0; i < bytes; i++)
        vflash[off + i] &= data[i];
    return true;
}

bool cfs_port_write_double_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    uint32_t off = addr - VFLASH_BASE;
    uint16_t bytes = len * 8u;
    for (uint16_t i = 0; i < bytes; i++)
        vflash[off + i] &= data[i];
    return true;
}

/* ---- port read ---- */

bool cfs_port_read(volatile uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint32_t off = addr - VFLASH_BASE;
    memcpy(buf, &vflash[off], len);
    return true;
}

/* ---- lock / unlock (no-op on PC) ---- */

bool cfs_port_lock_enable(void)  { return true; }
bool cfs_port_lock_disable(void) { return true; }

/* ---- erase ---- */

bool cfs_port_erase_page(volatile uint32_t addr, uint16_t pages)
{
    uint32_t off = addr - VFLASH_BASE;
    uint32_t bytes = (uint32_t)pages * CFS_FLASH_SECTOR_SIZE;
    memset(&vflash[off], 0xFF, bytes);
    return true;
}
