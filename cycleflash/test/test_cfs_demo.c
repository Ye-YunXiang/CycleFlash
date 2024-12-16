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

#include "cfs_demo.h"
#include "cycle_flash_system.h"


// 这里给一页128Byte吧，方便后续测试。
// 创建一个模拟flash的缓存区
uint8_t cfs_buffer[1024 * 128];

static cfs_object_handle_ptr product_filesystem = NULL;

static bool filesystem_success_init = false;
// 内存初始化
bool cfs_demo_init(void)
{	
    if (filesystem_success_init == true)
    {
        return true;
    }

    product_filesystem =
        cfs_nv_object_init((uint8_t *)"0", // 名字
                           (uint32_t)(&cfs_buffer),  // 地址
                           3,   // 页数
                           25,  // 数据大小(不包含元数据)
                           CFS_OBJECT_TYPE_FIXED_DATA_STORAGE); // 数据类型
    if(product_filesystem == false)
    {
        return false;
    }

    filesystem_success_init = true;
    return true;
}

int cfs_demo_write_product_nv(void* data, uint32_t len)
{
    if (len < 1)
    {
        return false;
    }

    const int bytes_written = cfs_nv_write(product_filesystem, data, len);

    // 用于固定长度的DEMO
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

    const int bytes_read = cfs_nv_read(product_filesystem, (uint8_t *)data, len, read_in_past);

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
    return cfs_nv_clear(product_filesystem);
}


uint32_t cfs_demo_product_current_id_get(void)
{
    return cfs_nv_get_current_id(product_filesystem);
}

uint32_t cfs_demo_product_current_valid_id_get(void)
{
    return cfs_nv_get_current_valid_id(product_filesystem);
}

