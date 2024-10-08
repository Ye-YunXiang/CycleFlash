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


// flash写入设备状态地址起始
#define FLASH_ADDR_PRODUCT_NV_ID_MANAGER						(0x00012000u)	//62 sector
// 写入设备剩余信用量地址起始
#define FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING		(0x00012200u)	
// 写入nexus状态起始
#define FLASH_ADDR_NEXUS_NV_START								(0x0001C000u)

// 用于管理存储nexus的信息，专门用与Nexus的NV读写操作的空间。
static cfs_system_handle_t nexus_filesystem = NULL;
// 用于管理存储信用量的信息，比如剩余信用量，和总共信用量之类的。
static cfs_system_handle_t product_filesystem = NULL;
// 用于管理存储设备的身份信息，专门用与存储设备ID之类。
static cfs_system_handle_t identity_filesystem = NULL;

// 检查Flash系统中的条目是否损坏
#define FLASH_FILESYSTEM_READ_NUMBER_OF_PAST_ENTRIES_TO_EXAMINE 10

static bool filesystem_success_init = false;
// 内存初始化
bool flash_filesystem_init(void)
{	
	cfs_system cfs_system_init;
	
    if (filesystem_success_init == true)
    {
        return true;
    }

    // 循环存储数据，nexus信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH;
    cfs_system_init.addr_handle = FLASH_ADDR_NEXUS_NV_START;
    cfs_system_init.sector_size = 512;
    cfs_system_init.sector_count = 4;
    cfs_system_init.data_size = 40;
    nexus_filesystem = cfs_nv_object_init(&cfs_system_init);
    if(nexus_filesystem == false)
    {
        return false;
    }
    
    // 循环存储数据，product信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH;
    cfs_system_init.addr_handle = FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING;
    cfs_system_init.sector_size = 512;
    cfs_system_init.sector_count = 4;
    cfs_system_init.data_size = 4;
    product_filesystem = cfs_nv_object_init(&cfs_system_init);
    if(product_filesystem == false)
    {
        return false;
    }

    // 存储定长identity的信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE;
    cfs_system_init.addr_handle = FLASH_ADDR_PRODUCT_NV_ID_MANAGER;
    cfs_system_init.sector_size = 512;
    cfs_system_init.sector_count = 1;
    cfs_system_init.data_size = 20;
    identity_filesystem = cfs_nv_object_init(&cfs_system_init);
    if(identity_filesystem == false)
    {
        return false;
    }

    filesystem_success_init = true;
    return true;
}

// 产品和Nexus NV *write*功能使用的通用代码
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

// 产品和Nexus NV *read*功能使用的通用代码
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

// 写入信用量到内存
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

// 写入nexus的信息到内存
int flash_filesystem_write_nexus_nv(uint16_t id, void* data, uint32_t len)
{
    const bool success =
        _internal_flash_filesystem_write_nv(nexus_filesystem, id, data, len);
    if (success)
    {
        return len;
    }
    return 0;
}

// 写入设备的ID数据
int flash_filesystem_write_identity_nv( \
    enum flash_filesystem_product_nv_id id, void* data, uint32_t len)
{
    const bool success = _internal_flash_filesystem_write_nv( \
        identity_filesystem, (uint32_t)id, data, len);
    if (success)
    {
        return len;
    }
    return 0;
}

// 读取内存中的设备数据
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

// 读取内存中的nexus的数据
int flash_filesystem_read_nexus_nv(uint16_t id, void* data, uint32_t len)
{
    const bool success =
        _internal_flash_filesystem_read_nv(nexus_filesystem, (uint32_t)id, data, len);
    if (success)
    {
        return len;
    }
    return 0;
}

// 读取设备的ID数据
int flash_filesystem_read_identity_nv( \
    enum flash_filesystem_product_nv_id id, void* data, uint32_t len)
{
    const bool success = _internal_flash_filesystem_read_nv( \
        identity_filesystem, (uint32_t)id, data, len);
    if (success)
    {
        return len;
    }
    return 0;
}


// 擦除内存中的nexus信息。
bool flash_filesystem_erase_nexus_nv(void)
{
    return cfs_nv_clear(nexus_filesystem);
}

// 擦除内存中的设备信息
bool flash_filesystem_erase_product_nv(void)
{
    return cfs_nv_clear(product_filesystem);
}

// 擦除内存中关于ID的信息
bool flash_filesystem_erase_identity_nv(void)
{
    return cfs_nv_clear(identity_filesystem);
}

