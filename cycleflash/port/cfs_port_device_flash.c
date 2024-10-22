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
 *
 * Function: It is the definitions head file for this library.
 * Created on: 2024-7-26
 */
// Encoding:UTF-8

#include "cfs_system_define.h"
#include "cfs_port_device_flash.h"
#include "flash.h"


// 下方代码是根据HC32L176写的案例，仅供参考

static bool cfs_port_flash_init_flag = false;


bool cfs_port_system_flash_init(void)
{
    if(cfs_port_flash_init_flag == true)
    {
        return true;
    }

    /*User initialization code*/

    cfs_port_flash_init_flag = true;
    return true;
}

bool cfs_port_system_flash_write_byte( \
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    uint16_t i;
    for(i=0; i< len; i++)
    {
        Flash_WriteByte(addr, *data);
        addr += 1u;
        data += 1u;
    }

    return true;
}

bool cfs_port_system_flash_write_half_word( \
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    uint16_t i;
    volatile uint16_t write_u16data = 0;
    for(i=0; i< len; i++)
    {
        write_u16data = (data[0]) | (data[1] << 8);
        Flash_WriteHalfWord(addr, write_u16data);
        addr += 2u;
        data += 2u;
    }

    return true;
}

bool cfs_port_system_flash_write_word( \
    volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    uint16_t i;
    volatile uint32_t write_u32data = 0;
    for(i=0; i< len; i++)
    {
        write_u32data = (data[0]) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        Flash_WriteWord(addr, write_u32data);
        addr += 4u;
        data += 4u;
    }

    return true;
}

bool cfs_port_system_flash_read( \
    volatile uint32_t addr, uint8_t * buffer, uint16_t len)
{
    memcpy(buffer, (uint8_t *)addr, len);
    return true;
}

//@def 如果相同返回true
bool cfs_port_system_flash_read_contrast( \
    volatile uint32_t addr, uint8_t * buffer, uint16_t len)
{
    if(memcmp(buffer, (uint8_t *)addr, len) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//@def 如果是空值返回true
bool cfs_port_system_flash_read_checking_null_values( \
    volatile uint32_t addr, uint16_t len)
{
    uint16_t i;
    
    for(i=0; i< len; i++)
    {
        if(0xff != *((uint8_t *)addr))
        {
            // 不为空
            return false;
        }
        addr += 0x01;
    }

    return true;
}

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
    uint16_t i = 0;
    for(i = 0; i < page; i++)
    {
        addr = addr + (i * 512);
        while(Ok != Flash_SectorErase(addr)){};
    }

    return true;
}
