/*
 * This file is part of the cycle_flash_system Library.
 *
 * Copyright (c) 2026, YeYunXiang, <poetrycloud@foxmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// Encoding:UTF-8

/*
 * cfs_port.c — Flash HAL 空桩
 * 替换为真实 MCU 驱动实现
 */

#include "cfs_port.h"

bool cfs_port_write_byte(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    (void)addr; (void)data; (void)len;
    // Flash_WriteByte(addr, *data);
    return true;
}

bool cfs_port_write_half_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    (void)addr; (void)data; (void)len;
    // Flash_WriteHalfWord(addr, data);
    return true;
}

bool cfs_port_write_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    (void)addr; (void)data; (void)len;
    // Flash_WriteWord(addr, data);
    return true;
}

bool cfs_port_write_double_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    (void)addr; (void)data; (void)len;
    return true;
}

bool cfs_port_read(volatile uint32_t addr, uint8_t *buf, uint16_t len)
{
    (void)addr; (void)buf; (void)len;
    // memcpy(buf, (uint8_t *)addr, len);
    return true;
}

bool cfs_port_lock_enable(void)
{
    return true;
}

bool cfs_port_lock_disable(void)
{
    return true;
}

bool cfs_port_erase_page(volatile uint32_t addr, uint16_t pages)
{
    (void)addr; (void)pages;
    // Flash_SectorErase(addr);
    return true;
}
