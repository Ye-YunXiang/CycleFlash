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
#include <assert.h>
#include <limits.h>

#include "cfs_system_oc.h"
#include "cfs_port_device_flash.h"
#include "cfs_system_utils.h"

/*数据对象的头指针*/
static cfs_object_linked_list *cfs_system_object_head = NULL;
static cfs_object_linked_list *cfs_system_object_tail = NULL;


// 读取内存中的数据,会验证crc8
cfs_oc_read_data_result \
    cfs_system_oc_read_flash_data(const uint32_t addr, cfs_data_block * buffer)
{
    assert(buffer->data_pointer != NULL || buffer->data_len >= 1);
    
    cfs_oc_read_data_result result = CFS_OC_READ_DATA_RESULT_NULL;

    uint16_t info_block_len = \
        CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN + buffer->data_len;

    // 创建读取整条数据的缓存
    uint8_t *buffer_read = (uint8_t *)CFS_MALLOC(info_block_len);
    volatile uint8_t i = 2;
    while(i--)
    {
        uint32_t temp_id = 0;
        cfs_port_system_flash_read_data(addr, buffer_read, info_block_len);
        uint16_t check_crc_16 = cfs_system_utils_crc16_xmodem_check( \
            buffer_read, (info_block_len - sizeof(buffer->data_crc_16)));
        uint16_t read_crc_16 = *(buffer_read + (info_block_len - sizeof(buffer->data_crc_16)));

        memcpy(&temp_id, buffer_read, sizeof(temp_id));

        if(temp_id == UINT_MAX)
        {
            result = CFS_OC_READ_DATA_RESULT_NULL;
        }
        else if(check_crc_16 == read_crc_16)
        {
            // 读取数据块ID
            memcpy(&(buffer->data_id), buffer_read, sizeof(buffer->data_id));
            // 读取用户数据
            memcpy(buffer->data_pointer, \
                buffer_read + CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, \
                (buffer->data_len));
            buffer->data_crc_16 = read_crc_16;
            result = CFS_OC_READ_DATA_RESULT_DATA_SUCCEED;
            break;
        }
        else
        {
            result = CFS_OC_READ_DATA_RESULT_DATA_ERROE;
        }
    }

    CFS_FREE(buffer_read);
    return result;
}


// 根据ID计算有效数据个数
uint32_t cfs_system_oc_valid_data_number(cfs_object_linked_list *temp_linked_object)
{
    uint32_t result_id = 0;
    uint32_t all_data = 0; 
    uint32_t data_pages = 0;
    uint32_t data_cycle = 0;
    cfs_system *temp_system_object = temp_linked_object->object_handle;

    if(temp_linked_object->data_id <= 2)
    {
        all_data = temp_system_object->sector_size / temp_system_object->data_size;
        data_pages = temp_linked_object->data_id % all_data;
        if(data_pages >= temp_system_object->sector_count)
        {
            result_id = ((temp_linked_object->data_id % all_data) == 0) ? \
                (all_data * temp_system_object->sector_count) : \
                (all_data * (temp_system_object->sector_count - 1) + data_pages);
        }
        else
        {
            result_id = temp_linked_object->data_id;
        }
    }
    else
    {
        all_data = (temp_system_object->sector_size * temp_system_object->sector_count) \
            / temp_system_object->data_size;
        data_cycle = temp_linked_object->data_id % all_data;

        if(data_cycle > 0)
        {
            result_id = all_data - \
                ( temp_system_object->sector_size / temp_system_object->data_size) - 1;
        }
        else
        {
            result_id = temp_linked_object->data_id;
        }
    }

    return result_id;
}

// 验证链有没有表数据数据对象
bool cfs_system_oc_object_linked_crc_16_verify(\
    const cfs_object_linked_list * temp_object, const uint16_t temp_crc_16)
{
    if(temp_object->this_linked_addr_crc_16 == temp_crc_16)
    {
        return true;
    }

    return false;
}

// 链表添加一个数据对象
cfs_object_linked_list *cfs_system_oc_add_object(\
    cfs_system *object_pointer, const uint8_t * const name) 
{
    cfs_object_linked_list *new_node = 
        (cfs_object_linked_list *)CFS_MALLOC(sizeof(cfs_object_linked_list));
    if (new_node == NULL) 
    {
        /* Allocation failure */
        return NULL;
    }

	uint8_t temp_len = strlen((char *)name) + 1;
    /*写入名字*/
    while(temp_len != CFS_CONFIG_MAX_OBJECT_NAME_LEN)
    {
        if(name[temp_len] == '\0' || temp_len >= (CFS_CONFIG_MAX_OBJECT_NAME_LEN - 1))
        {
            new_node->object_name[temp_len] = '0';
            break;
        }
        else
        {
            new_node->object_name[temp_len] = name[temp_len];
        }

        temp_len++;
    }

    if (cfs_system_object_head == NULL)
    {
        cfs_system_object_head = new_node;
    }
    else
    {
        cfs_system_object_tail->next = new_node;
    }

    new_node->object_handle = object_pointer;
    new_node->data_id = 0;
    new_node->this_linked_addr_crc_16  = \
        cfs_system_utils_crc16_xmodem_check((uint8_t *)((uint32_t)new_node), sizeof(uint32_t));
    cfs_system_object_tail = new_node;

    return new_node;
}


// 检查设置的文件的地址和将要存入数据的地址片区有没有重复
// 有交叉返回 true， 没有交叉返回 false
bool cfs_system_oc_flash_repeat_address(const cfs_system *temp_object)
{
    if(cfs_system_object_head != NULL)
    {
        uint32_t head_1 = temp_object->addr_handle;

        uint32_t tail_1 = temp_object->addr_handle + \
            (temp_object->sector_size * temp_object->sector_count);

        cfs_object_linked_list *temp_pointer = cfs_system_object_head->next;
        
        while(temp_pointer != NULL) 
        {
            uint32_t head_2 = temp_pointer->object_handle->addr_handle;
            uint32_t tail_2 = temp_pointer->object_handle->addr_handle + \
                (temp_pointer->object_handle->sector_size * \
                temp_pointer->object_handle->sector_count);

            if((head_1 <= tail_2 && tail_1 >= head_2) || \
                (head_2 <= tail_1 && tail_2 >= head_1) || \
                (head_1 <= head_2 && tail_1 >= tail_2) || \
                (head_2 <= head_1 && tail_2 >= tail_1))
            {
                /*分配的内存地址交叉了*/
                return true;
            }

            temp_pointer = temp_pointer->next;
        }
    }
    /*No object name*/
    return false;
}


// *****************************************************************************************************
// 设置和获取对象接口

/*设置数据数据对象的ID*/
bool cfs_system_oc_object_id_set( \
    cfs_object_linked_list * temp_cfs_handle, uint32_t temp_id)
{
    temp_cfs_handle->data_id = temp_id;
    return true;
}


// 得到数据数据对象的ID
uint32_t cfs_system_oc_object_id_get(cfs_object_linked_list *temp_cfs_handle)
{
    return temp_cfs_handle->data_id;
} 

/*设置数据数据对象的可用ID*/
bool cfs_system_oc_object_valid_id_number_set( \
    cfs_object_linked_list * temp_cfs_handle, uint32_t temp_id)
{
    temp_cfs_handle->valid_id_number = temp_id;
    return true;
}


// 得到数据数据对象的ID
uint32_t cfs_system_oc_object_valid_id_number_get( \
    cfs_object_linked_list *temp_cfs_handle)
{
    return temp_cfs_handle->valid_id_number;
} 


// 得到系统数据对象指针
cfs_system *cfs_system_oc_system_object_get(const cfs_object_linked_list *temp_object)
{
    return temp_object->object_handle;
}

