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

#include "cfs_DEMO.h"
#include "cycle_flash_system.h"

#define FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING		(0x00012200u)	

static cfs_system_handle_t product_filesystem = NULL;

static bool filesystem_success_init = false;
// 内存初始化
bool flash_filesystem_init(void)
{	
	cfs_system cfs_system_init;
	
    if (filesystem_success_init == true)
    {
        return true;
    }

    // 循环存储数据，product信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH;
    cfs_system_init.addr_handle = FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING;
    cfs_system_init.sector_size = 512;
    cfs_system_init.sector_count = 2;
    cfs_system_init.data_size = 27;
    product_filesystem = cfs_nv_object_init(&cfs_system_init);
    if(product_filesystem == false)
    {
        return false;
    }

    filesystem_success_init = true;
    return true;
}

// *write*功能使用的通用代码
static bool _internal_flash_filesystem_write_nv( \
    cfs_system_handle_t fs, uint32_t id, void* data, uint32_t len)
{
    if (!filesystem_success_init)
    {
        return false;
    }
    if (len < 1)
    {
        return false;
    }

    assert(fs != NULL);
    const uint32_t bytes_written = cfs_nv_write(fs, id, data, len);

    if (bytes_written == len)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// *read*功能使用的通用代码
static bool _internal_flash_filesystem_read_nv( \
    cfs_system_handle_t fs, uint32_t id, void* data, uint32_t len)
{
    if (!filesystem_success_init)
    {
        return false;
    }
    if (len < 1)
    {
        return false;
    }

    assert(fs != NULL);
    const uint32_t bytes_read = cfs_nv_read(fs, id, (uint8_t *)data, len);

    if (bytes_read == len)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int flash_filesystem_write_product_nv(uint32_t id, void* data, uint32_t len)
{
    const bool success = \
        _internal_flash_filesystem_write_nv(product_filesystem, id, data, len);
    if (success)
    {
        return len;
    }
    return 0;
}


int flash_filesystem_read_product_nv(uint32_t id, void* data, uint32_t len)
{
    const bool success = \
        _internal_flash_filesystem_read_nv(product_filesystem, id, data, len);
    if (success)
    {
        return len;
    }
    return 0;
}


// 擦除内存中的设备信息
bool flash_filesystem_erase_product_nv(void)
{
    return cfs_nv_clear(product_filesystem);
}


uint32_t flash_filesystem_product_current_id_get(void)
{
    return cfs_nv_get_current_id(product_filesystem);
}

uint32_t flash_filesystem_product_current_valid_id_get(void)
{
    return cfs_nv_get_current_valid_id(product_filesystem);
}

bool flash_filesystem_product_delete_object(void)
{
    return cfs_nv_object_delete(product_filesystem);
}
