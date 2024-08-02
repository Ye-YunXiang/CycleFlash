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

/**/

// 非紧密存储遍历内存ID初始化  感觉需要在分一层嵌出来，考虑内部可以复用嘛
// 偷懒，后续在优化吧
static uint32_t cfs_filesystem_not_tight_data_page_id_init(cfs_system * temp_cfs_handle)
{
    uint32_t temp_data_MAX_id = 0;

    // 初始化缓冲数据块
    cfs_data_block data_block;
    // // 包头包尾的长度
    // uint16_t read_data_block_total_len = \
    //     CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN + temp_cfs_handle->data_size;

    memset(&data_block, NULL, sizeof(cfs_data_block));   
    // 用户存储数据空间申请
    data_block.data_pointer = (uint8_t *)CFS_MALLOC(data_block.data_len);
    
    // 存储数据的起始地址，和记录最大ID
    volatile uint32_t data_addr = temp_cfs_handle->addr_handle;
    volatile uint32_t data_max_addr = NULL;
    // 遍历每一页第一位的值，考虑到如果第一位数据存储错误的情况
    for(uint16_t i = 0; i < temp_cfs_handle->sector_count; i++)
    {
        uint8_t temp_count = 0;
        cfs_oc_read_data_result read_result = CFS_OC_READ_DATA_RESULT_NULL;

        while(read_result != CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
        {
            memset(data_block.data_pointer, NULL, data_block.data_len);
            read_result = cfs_system_oc_read_flash_data( \
                (temp_cfs_handle->addr_handle + i * temp_cfs_handle->sector_size) + \
                (temp_cfs_handle->data_size * temp_count), &data_block);
            
            // 读错就在往后读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED || \
                (temp_count ==2 && read_result != CFS_OC_READ_DATA_RESULT_DATA_SUCCEED))
            {
                data_addr = \
                    (temp_cfs_handle->addr_handle + i * temp_cfs_handle->sector_size) + \
                    (temp_cfs_handle->data_size * temp_count);
                data_max_addr = \
                    (temp_cfs_handle->addr_handle + (i+1) * temp_cfs_handle->sector_size);
                break;
            }
            else if(read_result == CFS_OC_READ_DATA_RESULT_NULL)
            {
                break;
            }

            if(temp_count <= \
                (temp_cfs_handle->sector_size / temp_cfs_handle->data_size - 1))
            {
                temp_count++;
            }
            else
            {
                break;
            }
        }

        if((data_block.data_id > temp_data_MAX_id) && \
            read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
        {
            temp_data_MAX_id = data_block.data_id;
        }
    }

    // 如果读出来的结果是有ID的，遍历ID最大的这一页，寻找ID的最大值
    if(temp_data_MAX_id > 0)
    {
        uint8_t temp_count = 0;
        cfs_oc_read_data_result read_result = CFS_OC_READ_DATA_RESULT_NULL;
        while(read_result != CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
        {
            memset(data_block.data_pointer, NULL, data_block.data_len);
            read_result = cfs_system_oc_read_flash_data( \
                data_addr + (temp_cfs_handle->data_size * temp_count), &data_block);
            
            // 读错就在往前读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
            {
                if((data_block.data_id > temp_data_MAX_id) && \
                    read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
                {
                    temp_data_MAX_id = data_block.data_id;
                } 
            }
            else if(read_result == CFS_OC_READ_DATA_RESULT_NULL)
            {
                break;
            }

            if((data_addr + \
                (temp_cfs_handle->data_size * (temp_count + 1))) <= data_max_addr)
            {
                temp_count++;
            }
            else
            {
                break;
            }
        }
    }

    CFS_FREE(data_block.data_pointer);
    return  temp_data_MAX_id;
}

// 紧密存储遍历内存ID初始化
// 偷懒，后续在优化吧
static uint32_t cfs_filesystem_tight_data_page_id_init(cfs_system * temp_cfs_handle)
{
    uint32_t temp_data_MAX_id = 0;

    // 初始化缓冲数据块
    cfs_data_block data_block;
    // // 包头包尾的长度
    // uint16_t read_data_block_total_len = \
    //     CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN + temp_cfs_handle->data_size;

    memset(&data_block, NULL, sizeof(cfs_data_block));   
    // 用户存储数据空间申请
    data_block.data_pointer = (uint8_t *)CFS_MALLOC(data_block.data_len);
    
    // 存储数据的起始地址，和记录最大ID
    volatile uint32_t data_addr = temp_cfs_handle->addr_handle;
    volatile uint32_t data_max_addr = NULL;
    // 遍历每一页第一位的值，考虑到如果第一位数据存储错误的情况
    for(uint16_t i = 0; i < temp_cfs_handle->sector_count; i++)
    {
        uint8_t temp_count = 0;
        cfs_oc_read_data_result read_result = CFS_OC_READ_DATA_RESULT_NULL;

        while(read_result != CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
        {
            uint32_t temp_addr = \
                (i * temp_cfs_handle->sector_size) / temp_cfs_handle->data_size;
            // 计算要第几个数据的数据地址。
            if(i == 0)
            {
                temp_addr = temp_cfs_handle->addr_handle + \
                    temp_count * temp_cfs_handle->data_size;
            }
            else
            {
                temp_addr = temp_cfs_handle->addr_handle + \
                    (((i * temp_cfs_handle->sector_size) / \
                    temp_cfs_handle->data_size) + 1) * temp_cfs_handle->data_size;
            }
            
            memset(data_block.data_pointer, NULL, data_block.data_len);
            read_result = cfs_system_oc_read_flash_data(temp_addr, &data_block);
            
            // 读错就在往后读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED || \
                (temp_count ==2 && read_result != CFS_OC_READ_DATA_RESULT_DATA_SUCCEED))
            {
                data_addr = temp_addr;
                data_max_addr = \
                    (temp_cfs_handle->addr_handle + (i+1) * temp_cfs_handle->data_size);
                break;
            }
            else if(read_result == CFS_OC_READ_DATA_RESULT_NULL)
            {
                break;
            }

            if(temp_count <= \
                (temp_cfs_handle->sector_size / temp_cfs_handle->data_size - 1))
            {
                temp_count++;
            }
            else
            {
                break;
            }
        }

        if((data_block.data_id > temp_data_MAX_id) && \
            read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
        {
            temp_data_MAX_id = data_block.data_id;
        }
    }

    // 如果读出来的结果是有ID的，遍历ID最大的这一页，寻找ID的最大值
    if(temp_data_MAX_id > 0)
    {
        uint8_t temp_count = 0;
        cfs_oc_read_data_result read_result = CFS_OC_READ_DATA_RESULT_NULL;
        while(read_result != CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
        {
            memset(data_block.data_pointer, NULL, data_block.data_len);
            read_result = cfs_system_oc_read_flash_data( \
                data_addr + (temp_cfs_handle->data_size * temp_count), &data_block);
            
            // 读错就在往前读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
            {
                if((data_block.data_id > temp_data_MAX_id) && \
                    read_result == CFS_OC_READ_DATA_RESULT_DATA_SUCCEED)
                {
                    temp_data_MAX_id = data_block.data_id;
                } 
            }
            else if(read_result == CFS_OC_READ_DATA_RESULT_NULL)
            {
                break;
            }

            if((data_addr + \
                (temp_cfs_handle->data_size * (temp_count + 1))) <= data_max_addr)
            {
                temp_count++;
            }
            else
            {
                break;
            }
        }
    }

    CFS_FREE(data_block.data_pointer);
    return  temp_data_MAX_id;
}


// 检查重复地址
bool cfs_filesystem_check_flash_repeat_address(const cfs_system *temp_object)
{
    // 肯定不能超过uint32类型最大值
    assert((temp_object->addr_handle + \
        (temp_object->sector_size * temp_object->sector_count)) <= UINT_MAX);

    if(cfs_system_oc_flash_repeat_address(temp_object) == true)
    {
        return true;
    }

    return false;
}


bool cfs_filesystem_object_id_init( cfs_system_handle_t temp_cfs_handle)
{
    uint32_t temp_object = (uint32_t)(temp_cfs_handle >> 1);
    uint16_t temp_crc_16 = (uint16_t)temp_cfs_handle;
    uint32_t temp_data_max_id = 0;

    assert(cfs_system_oc_object_linked_crc_16_verify(temp_object, temp_crc_16) == true);
    
    cfs_system *temp_cfs_system = cfs_system_oc_system_object_get(temp_object);

    // 固定数据长度遍历数据
    assert(temp_cfs_system->data_size != 0);

    if(temp_cfs_system->struct_type == CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH && \
        temp_cfs_system->sector_count <= 2)
    {
        // 非紧密存储数据
        temp_data_max_id = cfs_filesystem_not_tight_data_page_id_init(temp_cfs_system);
    }
    else
    {
        // 紧密存储数据
        temp_data_max_id = cfs_filesystem_tight_data_page_id_init(temp_cfs_system);
    }

	// 设置遍历好的ID值
    cfs_system_oc_object_id_set( \
		(cfs_object_linked_list *)temp_cfs_handle, temp_data_max_id);

    return true;
}

cfs_system_handle_t cfs_filesystem_object_add_oc_object( \
    cfs_system *temp_object, const char * const name)
{
    assert((strlen(name) + 1) <= CFS_CONFIG_MAX_OBJECT_NAME_LEN || \
        CFS_CONFIG_MAX_OBJECT_NAME_LEN <= UCHAR_MAX);

    cfs_object_linked_list *temp_linked_object = cfs_system_oc_add_object(temp_object, name);
    if(temp_linked_object == NULL)
    {
        return false;
    }

    cfs_system_handle_t cfs_object_handle = \
        ((uint32_t)(temp_linked_object) << (temp_linked_object->this_linked_addr_crc_16)) & \
        temp_linked_object->this_linked_addr_crc_16;
    
    return cfs_object_handle;
}

cfs_system_handle_t cfs_filesystem_object_init( \
    cfs_system *temp_object, const char * const name)
{
    // 判断参数有效性
    assert(temp_object->sector_size % 64 == 0);

    assert((temp_object->struct_type != CFS_FILESYSTEM_OBJECT_TYPE_NULL) ||\
        (temp_object->sector_count != 0) || \
        (temp_object->addr_handle != 0) || \
        (temp_object->sector_size != 0) || \
        (temp_object->data_size != 0));

    if((temp_object->struct_type == CFS_FILESYSTEM_OBJECT_TYPE_NULL) ||\
        (temp_object->sector_count == 0) || \
        (temp_object->addr_handle == 0) || \
        (temp_object->sector_size == 0) || \
        (temp_object->data_size == 0))
    {
        // 检查参数
        return false;
    }

    if(temp_object->struct_type == CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH || \
        temp_object->sector_count < 2 )
    {
        // 如果使用的循环存储，分配的页数必须要 >= 2
        assert(false);
        return false;
    }

    // 在判断地址有没有重复
    if(true == cfs_filesystem_check_flash_repeat_address(temp_object))
    {
        /*内存参数交叉了！*/
        assert(false);
        return false;
    }

    // 初始化本地flash
    if(cfs_port_system_flash_init() != true)
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

    cfs_filesystem_object_id_init(new_cfs_object_handle);



    

    return 0;
}

