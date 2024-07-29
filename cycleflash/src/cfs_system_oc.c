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

#include "cfs_system_oc.h"
#include "cfs_port_device_flash.h"
#include "cfs_system_utils.h"

/*数据对象的头指针*/
static cfs_object_linked_list *cfs_system_object_head = NULL;
static cfs_object_linked_list *cfs_system_object_tail = NULL;


// 遍历数据页，初始化ID值
uint32_t cfs_system_oc_traverse_data_page_id_init(uint32_t temp_cfs_handle)
{
    // 创建和初始化待会要用到的变量
    cfs_object_linked_list * cfs_handle = (cfs_object_linked_list *)temp_cfs_handle;
    cfs_system *object_handle = cfs_handle->object_handle;
    // 存储数据的起始地址
    volatile uint32_t data_addr = object_handle->addr_handle;

    assert(object_handle->data_size != 0);

    // 初始化缓冲数据块
    cfs_data_block data_block;
    // 包头包尾的长度
    uint16_t info_len = sizeof(cfs_data_block) - sizeof(data_block.data_pointer);
    // 检测块大小，因为光包头包尾就很大了
    assert(cfs_handle->object_handle->data_size > info_len);

    memset(&data_block, NULL, sizeof(cfs_data_block));   
    // 数据块内容去除包头包尾的长度，保留数据长度
    data_block.data_len = object_handle->data_size - info_len;
    data_block.data_pointer = (uint8_t *)CFS_MALLOC(object_handle->data_size - info_len);

//    while (/* condition */)
//    {
//        /* code */
//    }
    
        // 开始遍历
        if(cfs_handle->object_handle->data_sector_count <= 2)
        {
            
            // ID遍历算法~~~~~~~~~~~~~~~~~~~~~

            //cfs_system_oc_read_flash_data();
        }
        else
        {
            // ID遍历算法~~~~~~~~~~~~~~~~~~~~~
            //cfs_system_oc_read_flash_data();
        }

    CFS_FREE(data_block.data_pointer);
    return cfs_handle->data_id;
}

// 遍历目录页，初始化ID值
uint32_t cfs_system_oc_traverse_list_page_id_init(uint32_t temp_cfs_handle)
{
	return 0;
}

// 读取内存中的数据,会验证crc8
bool cfs_system_oc_read_flash_data(const uint32_t addr, cfs_data_block * buffer)
{
    assert(buffer->data_pointer != NULL || buffer->data_len > 1);
    
    bool result = false;
    volatile uint8_t i = 2;
    uint16_t info_block_len = \
        sizeof(cfs_data_block) - sizeof(buffer->data_pointer) + buffer->data_len;

    // 创建读取整条数据的缓存
    uint8_t *buffer_read = (uint8_t *)CFS_MALLOC(info_block_len);
    while(i--)
    {
        cfs_port_system_flash_read_data(addr, buffer_read, info_block_len);
        uint8_t check_crc = cfs_system_utils_crc_8_check( \
            buffer_read, (info_block_len - sizeof(buffer->data_crc8)));
        uint8_t read_crc = *(buffer_read + (info_block_len - sizeof(buffer->data_crc8)));

        if(check_crc == read_crc)
        {
            memcpy(buffer->data_pointer, buffer_read, (buffer->data_len));
            memcpy(&(buffer->data_id), buffer_read, sizeof(buffer->data_id));
            buffer->data_crc8 = read_crc;
            result = true;
            break;
        }
    }

    CFS_FREE(buffer_read);
    return result;
}

/*设置数据数据对象的ID*/
bool cfs_system_oc_object_id_set(uint32_t temp_cfs_handle, uint32_t temp_id)
{
    ((cfs_object_linked_list *)temp_cfs_handle)->data_id = temp_id;
    return true;
}


// 得到数据数据对象的ID
uint32_t cfs_system_oc_object_id_get(uint32_t temp_cfs_handle)
{
    return ((cfs_object_linked_list *)temp_cfs_handle)->data_id;
} 


// 得到系统数据对象指针
cfs_system *cfs_system_oc_system_object_get(const uint32_t temp_object)
{
    return ((cfs_object_linked_list *)temp_object)->object_handle;
}


// 验证链有没有表数据数据对象
bool cfs_system_oc_object_linked_crc_8_verify(\
    const uint32_t temp_object, const uint8_t temp_crc_8)
{
    if(((cfs_object_linked_list *)temp_object)->this_linked_addr_crc_8 == temp_crc_8)
    {
        return true;
    }

    return false;
}

// 链表添加一个数据对象
cfs_object_linked_list *cfs_system_oc_add_object(\
    cfs_system *object_pointer, const char * const name) 
{
    cfs_object_linked_list *new_node = 
        (cfs_object_linked_list *)CFS_MALLOC(sizeof(cfs_object_linked_list));
    if (new_node == NULL) 
    {
        /* Allocation failure */
        return NULL;
    }

    /*控制名字长度*/
    uint8_t temp_len = 0;
    if((strlen(name) + 1) > CFS_CONFIG_MAX_OBJECT_NAME_LEN)
    {
        temp_len = CFS_CONFIG_MAX_OBJECT_NAME_LEN;
    }
    else
    {
        temp_len = strlen(name) + 1;
    }

    new_node->object_name = (char *)CFS_MALLOC(temp_len);
    if (new_node->object_name == NULL) 
    {
        CFS_FREE(new_node);
        /* Allocation failure */
        return NULL;
    }
    memcpy(new_node->object_name, name, temp_len);

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
    new_node->this_linked_addr_crc_8  = \
        cfs_system_utils_crc_8_check((uint8_t *)((uint32_t)new_node), sizeof(uint32_t));
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

        uint32_t tail_1 = temp_object->addr_handle + (temp_object->sector_size * \
            (temp_object->list_sector_count + temp_object->data_sector_count));

        cfs_object_linked_list *temp_pointer = cfs_system_object_head->next;
        
        while(temp_pointer != NULL) 
        {
            uint32_t head_2 = temp_pointer->object_handle->addr_handle;
            uint32_t tail_2 = temp_pointer->object_handle->addr_handle + \
                (temp_pointer->object_handle->sector_size * \
                (temp_pointer->object_handle->list_sector_count + \
                temp_pointer->object_handle->data_sector_count));

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

