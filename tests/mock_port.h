/*
 * mock_port.h -- virtual flash mock for testing CycleFlash
 */
#ifndef MOCK_PORT_H
#define MOCK_PORT_H

#include <stdint.h>

/* configurable virtual flash dimensions */
#define VFLASH_BASE   0x08000000u
#define VFLASH_SIZE   8192u   /* 8 KB, can hold many configs */

/* helpers for test code */
void     vflash_reset(void);
void     vflash_fill(uint32_t addr, uint8_t byte, uint32_t len);
uint8_t* vflash_ptr(uint32_t addr);
uint32_t vflash_get_base(void);
uint32_t vflash_get_size(void);

/* fault injection: corrupt a data byte (safe 1->0 flash transition) */
void     vflash_corrupt_byte(uint32_t addr);

/* fault injection: mark block header as bad (write zeros to data_id area) */
void     vflash_mark_bad_block(uint32_t addr);

#endif
