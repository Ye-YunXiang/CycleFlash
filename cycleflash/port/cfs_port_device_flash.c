/*
 * This file is part of the cycle_flash_system Library.
 *
 * Copyright (c) 2024, YeYunXiang, <poetrycloud@foxmail.com>
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
// 实现函数，每个函数的介绍请看 "cfs_user_define.h" 介绍

#include "cfs_user_define.h"
#include "cfs_port_device_flash.h"

#ifdef CFS_WRITE_PORT_ONE_BYTE == (1u)
bool cfs_port_system_flash_write_byte(
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    /*User initialization code*/

    return true;
}
#endif // CFS_WRITE_PORT_ONE_BYTE


#ifdef CFS_WRITE_PORT_ONE_BYTE == (1u)
bool cfs_port_system_flash_write_half_word(
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    /*User initialization code*/

    return true;
}
#endif // CFS_WRITE_PORT_HALF_WORD


#ifdef CFS_WRITE_PORT_ONE_WORD == (1u)
bool cfs_port_system_flash_write_word(
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    /*User initialization code*/

    return true;
}
#endif // CFS_WRITE_PORT_ONE_WORD


#ifdef CFS_WRITE_PORT_DOUBLE_WORD == (1u)
bool cfs_port_system_flash_write_double_word(
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    /*User initialization code*/

    return true;
}
#endif // CFS_WRITE_PORT_DOUBLE_WORD


#ifdef CFS_FLASH_READ_MODE == (1u)
bool cfs_port_system_flash_read(
    volatile uint32_t addr, uint8_t * buffer, uint16_t len)
{
    // memcpy(buffer, (uint8_t *)addr, len);
    return true;
}
#endif // CFS_FLASH_READ_MODE


bool cfs_port_system_flash_lock_enable(void)
{
    /*User initialization code*/

    return true;
}


bool cfs_port_system_flash_lock_disable(void)
{
    /*User initialization code*/

    return true;
}


bool cfs_port_system_flash_erasing_page(volatile uint32_t addr, uint16_t page)
{
    /*User initialization code*/

    return true;
}
