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
#include "cycle_flash_system.h"

// flash写入设备状态地址起始
#define FLASH_ADDR_PRODUCT_NV_ID_MANAGER						(0x00012000u)	//62 sector
// 写入设备剩余信用量地址起始
#define FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING		(0x00012200u)	
// 写入nexus状态起始
#define FLASH_ADDR_NEXUS_NV_START									(0x0001C000u)

// 用于管理存储nexus的信息，专门用与Nexus的NV读写操作的空间。
static cfs_system_handle_t nexus_filesystem;
// 用于管理存储信用量的信息，比如剩余信用量，和总共信用量之类的。
static cfs_system_handle_t product_filesystem;
// 用于管理存储设备的身份信息，专门用与存储设备ID之类。
static cfs_system_handle_t identity_filesystem;

static bool filesystem_success_init = false;
// 内存初始化
bool flash_filesystem_init(void)
{	
	cfs_system cfs_system_init;
	
    if (filesystem_success_init == true)
    {
        return true;
    }

    // 循环存储数据，不定长，nexus信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH;
    cfs_system_init.addr_handle = FLASH_ADDR_NEXUS_NV_START;
    cfs_system_init.sector_size = 512;
    cfs_system_init.list_sector_count = 2;
    cfs_system_init.data_sector_count = 4;
    nexus_filesystem = cfs_filesystem_object_init(&cfs_system_init, "nexus");
    if(nexus_filesystem == false)
    {
        return false;
    }
    
    // 循环存储数据，不定长，product信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH;
    cfs_system_init.addr_handle = FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING;
    cfs_system_init.sector_size = 512;
    cfs_system_init.list_sector_count = 2;
    cfs_system_init.data_sector_count = 4;
    product_filesystem = cfs_filesystem_object_init(&cfs_system_init, "product");
    if(product_filesystem == false)
    {
        return false;
    }

    // 存储定长identity的信息对象
    memset(&cfs_system_init, NULL, sizeof(cfs_system));
    cfs_system_init.struct_type = CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE;
    cfs_system_init.addr_handle = FLASH_ADDR_PRODUCT_NV_ID_MANAGER;
    cfs_system_init.sector_size = 512;
    cfs_system_init.data_sector_count = 1;
    cfs_system_init.data_size = 20;
    identity_filesystem = cfs_filesystem_object_init(&cfs_system_init, "identity");
    if(identity_filesystem == false)
    {
        return false;
    }


    return true;
}
