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

#include <string.h>
#include <limits.h>
#include <assert.h>

#include "cfs_system_oc.h"
#include "cycle_flash_system.h"
#include "cfs_port_device_flash.h" 




bool cfs_filesystem_check_flash_repeat_address(const cfs_system *temp_object)
{
    // 肯定不能超过uint32类型最大值
    assert((temp_object->addr_handle + (temp_object->sector_size * \
        (temp_object->list_sector_count + temp_object->data_sector_count)\
        )) <= UINT_MAX);

    if(cfs_system_oc_flash_repeat_address(temp_object) == true)
    {
        return true;
    }

    return false;
}


bool cfs_filesystem_object_id_init( cfs_system_handle_t temp_handle)
{
    uint32_t temp_object = (uint32_t)(temp_cfs_system_handle >> 1);
    uint8_t temp_crc_8 = (uint8_t)temp_cfs_system_handle;

    assert(cfs_system_oc_object_linked_crc_8_verify(temp_cfs_handle, name) == true);
    
    cfs_system *temp_cfs_system = cfs_system_oc_system_object_get(temp_object);

    if(temp_cfs_system->list_sector == CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_NULL)
    {
        assert(temp_cfs_system->struct_type == \
            CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_LENGTH || \
            temp_cfs_system->data_size != 0);
        cfs_system_oc_traverse_data_page_id_init(temp_object);
    }
    else
    {
        cfs_system_oc_traverse_list_page_id_init(temp_object);
    }


    return true;
}

cfs_system_handle_t cfs_filesystem_object_add_oc_object( \
    cfs_system *temp_object, const char * const name)
{
    assert((strlen(name) + 1) <= CFS_CONFIG_MAX_OBJECT_NAME_LEN || \
        CFS_CONFIG_MAX_OBJECT_NAME_LEN <= UCHAR_MAX);

    cfs_object_linked_list *temp_object = cfs_system_oc_add_object(temp_object, name);
    if(temp_object == NULL)
    {
        return false;
    }

    cfs_system_handle_t cfs_object_handle = \
        ((uint32_t)(temp_object) << 1) & temp_object->this_linked_addr_crc_8;
    
    return cfs_object_handle;
}

cfs_system_handle_t cfs_filesystem_object_init( \
    cfs_system *temp_object, const char * const name)
{
    // 判断参数有效性
    assert(temp_object->sector_size % 64 == 0);
    assert((temp_object->struct_type != CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_NULL) ||\
        (temp_object->data_sector_count != 0) || \
        (temp_object->addr_handle != 0) || \
        (temp_object->sector_size != 0));
    if(temp_object->struct_type == CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_LENGTH)
    {
        /*定长的情况下，数据大小肯定不能为0*/
        assert(temp_object->data_size != 0);
        return false;
    }

    // 在判断地址有没有重复
    bool verify_addr = cfs_filesystem_check_flash_repeat_address(temp_object);
    if(verify_addr == true)
    {
        /*内存参数交叉了！*/
        assert(verify_addr != true);
        return false;
    }

    // 初始化本地flash
    if(cfs_port_system_flash_init() == false)
    {
        return false;
    }
    
    /*开始初始化*/
    cfs_system_handle_t new_cfs_object_handle = \
        cfs_filesystem_object_add_oc_object(temp_object, name);
    if(new_cfs_object_handle == false)
    {
        return false;
    }

    cfs_filesystem_object_id_init(new_cfs_object_handle, name);



    

    return 0;
}

