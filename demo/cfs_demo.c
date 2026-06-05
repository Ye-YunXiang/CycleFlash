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

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "cfs_demo.h"
#include "cycle_flash_system.h"

#include "cfs_port.h"
#include "storage_hal.h"
#include "flash.h"


static cfs_object_t product_filesystem = {0};

// 内存初始化
bool cfs_demo_init(void)
{	
    // 写入地址
    bool state = cfs_nv_object_init(
        &product_filesystem, 
        (0x8010000),     // 写入地址 
        3,              // 几页
        25              // 数据大小
    );

    return state;
}

int cfs_demo_write_product_nv(void* data, uint32_t len)
{
    if (len < 1)
    {
        return false;
    }

    const int bytes_written = cfs_nv_write(&product_filesystem, data, len, 0);

    if (bytes_written != len)
    {
        return false;
    }
    
    return bytes_written;
}


int cfs_demo_read_product_nv(
    void* data, uint32_t len, uint32_t read_in_past)
{
    if (len < 1)
    {
        return false;
    }

    const int bytes_read = cfs_nv_read(&product_filesystem, (uint8_t *)data, len, read_in_past);

    // 用于固定长度的DEMO
    if (bytes_read != len)
    {
        return false;
    }

    return bytes_read;
}


// 擦除内存中的设备信息
bool cfs_demo_erase_product_nv(void)
{
    return cfs_nv_clear(&product_filesystem);
}


uint32_t cfs_demo_product_current_id_get(void)
{
    return cfs_nv_get_current_id(&product_filesystem);
}

uint32_t cfs_demo_product_current_valid_id_get(void)
{
    return cfs_nv_get_current_valid_id(&product_filesystem);
}




// 根据CycleFlash的定义，初始化相关写入函数和擦除函数-------------------------------------------
// 没用到的就不管 
bool cfs_port_write_byte(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
	ProgramPage(addr, len, data);
    return true;
}


bool cfs_port_write_half_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
	ProgramPage(addr, (len * 2), data);
	
    return true;
}


bool cfs_port_write_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    ProgramPage(addr, (len * 4), data);

    return true;
}


bool cfs_port_write_double_word(volatile uint32_t addr, const uint8_t *data, uint16_t len)
{
    return true;
}


// bool cfs_port_read(volatile uint32_t addr, uint8_t *buf, uint16_t len)
// {
//     memcpy(buf, (uint8_t *)addr, len);
//     return true;
// }

bool cfs_port_erase_page(volatile uint32_t addr, uint16_t page)
{
    uint16_t i = 0;
    for(i = 0; i < page; i++)
    {
        while(0 != EraseSector(addr + (i * 512))){};
    }

    return true;
}
