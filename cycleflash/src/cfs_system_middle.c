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

#include <string.h>
#include <assert.h>

#include "cfs_system_middle.h"
#include "cfs_system_oc.h"

// XXX:这里负责对象管理。。。。。。。。。。。。。

static cfs_object_list_t *object_list_head = NULL;


//*******************************************************************************************
//-- 对上层接口  
//*******************************************************************************************

//@def 初始化数据对象
bool cfs_middle_object_init(const cfs_object_t *object)
{

}

//@def 查找对象对象
cfs_object_list_t cfs_middle_find_object(const cfs_object_t *object)
{
    assert(object != NULL);
    assert(object->name != NULL);
  
    cfs_object_list_t *find_object = object_list_head;
    while (find_object != NULL)
    {
        if (strcmp(find_object->object_handle->name, object->name) == 0)
        {
            break;
        }
        find_object = find_object->next;
    }

    return find_object;
}

//@def 检查重复地址， 通过返回 true， 不通过返回 false
bool cfs_middle_check_address(const cfs_object_t *object)
{
    if(object == NULL)
    {
        return false;
    }

    cfs_object_list_t *list_pointer = cfs_system_object_head->next;
    uint32_t current_head = object->address;
    uint32_t current_tail = 
        object->address + (object->sector_count * CFS_FLASH_SECTOR_SIZE) - 1;
    
    // 遍历内存并检查和之前的数据对象是否有交叉
    while(list_pointer != NULL) 
    {
        uint32_t next_head = list_pointer->object_handle->addr_handle;
        uint32_t next_tail = list_pointer->object_handle->addr_handle + \
            (CFS_FLASH_SECTOR_SIZE * list_pointer->object_handle->sector_count) - 1;

        if((current_head <= next_tail && current_tail >= next_head) || \
            (next_head <= current_tail && next_tail >= current_head) || \
            (current_head <= next_head && current_tail >= next_tail) || \
            (next_head <= current_head && next_tail >= current_tail))
        {
            /*分配的内存地址交叉了*/
            return false;
        }

        list_pointer = list_pointer->next;
    }

    return true;
}

//@def 读取数据,读取成功返回读取的原始数据长度
uint32_t cfs_middle_data_read(
    cfs_object_list_t *object_list, uint32_t read_id, uint8_t *data, uint16_t len)
{
    
}

//@def 写入数据,写入成功返回写入的原始数据长度
uint32_t cfs_middle_data_write(
    cfs_object_list_t *object_list, uint32_t write_id, uint8_t *data, uint16_t len)
{


    
    return false;
}

//@def 清除本对象数据
bool cfs_system_oc_flash_data_clear(cfs_object_list_t *object_list)
{
    object_list->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    object_list->valid_id_number = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    memset(object_list->buffer, 0,object_list->object_handle->data_size);

    return true;
}

//@def 返回目前存储对象的ID
uint32_t cfs_middle_get_current_id(const cfs_object_list_t *object_list)
{
    if (object_list == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    // return cfs_system_oc_object_id_get(object_list);
    return object_list->data_id;
}

//@def 返回目前存储对象的可用ID
uint32_t cfs_middle_get_current_valid_id(const cfs_object_list_t *object_list)
{
    if (object_list == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }

    //return cfs_system_oc_object_valid_id_number_get(object_list);
    return object_list->valid_id_number;
}

