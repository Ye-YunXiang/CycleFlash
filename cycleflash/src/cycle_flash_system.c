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
#include "cycle_flash_system.h"
#include "cfs_port_device_flash.h" 
#include "cfs_system_utils.h"

// 测试-------
cfs_object_linked_list *temp_cfs_object_linked_list;
// ------------


//@def 紧密存储遍历内存ID初始化
//@def 偷懒，后续在优化吧
static uint32_t cfs_filesystem_tight_data_page_id_init( \
    cfs_object_linked_list *temp_linked_object)
{
    uint32_t temp_data_MAX_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    cfs_system *temp_cfs_handle = cfs_system_oc_system_object_get(temp_linked_object);

    //@def 初始化缓冲数据块
    cfs_data_block data_block;
    memset(&data_block, NULL, sizeof(cfs_data_block));   
    cfs_system_oc_object_block_buffer_set(temp_linked_object, &data_block);
    
    //@def 计算数据块大小
    const uint32_t data_block_size = \
        temp_cfs_handle->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;
    
    //@def 存储数据的起始地址，和记录最大ID
    volatile uint32_t data_start_id = 0;
    volatile uint32_t data_max_addr = NULL;
    volatile uint16_t data_max_i = 0;
    //@def 遍历每一页第一位的值，考虑到如果第一位数据存储错误的情况
    for(uint16_t i = 0; i < temp_cfs_handle->sector_count; i++)
    {
        uint8_t temp_count = 0;
        data_max_addr = \
            (temp_cfs_handle->addr_handle + (i+1) * temp_cfs_handle->sector_size);
        cfs_oc_action_data_result read_result = \
            CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;

        while(read_result != CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
        {
            //@def 因为ID从0开始，所以这里计算出来的天然多1个。
            //@def 然后如果是第0页得出的结果直接为0.
            data_start_id = \
                (i * temp_cfs_handle->sector_size) / data_block_size + temp_count;

            memset(data_block.data_pointer, NULL, temp_cfs_handle->data_size);
            data_block.data_id = data_start_id;
            read_result = cfs_system_oc_read_flash_data(temp_linked_object, &data_block);
            
            //@def 读错就在往后读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED || \
                (read_result != CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED && \
                temp_count == 2))
            {
                data_start_id = data_block.data_id;
                break;
            }
            else if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL)
            {
                break;
            }

            //@def 下方ID+1计算出地址后加上两个数据块的大小后，
            //@def 在减 1 得出在往上加上一个ID长度有没有超过这一页。
            if(data_max_addr > (cfs_system_oc_via_id_calculate_addr( \
                temp_linked_object, data_start_id) + data_block_size * 2 - 1))
            {
                temp_count++;
            }
            else
            {
                break;
            }
        }

        if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
        {
            if(data_block.data_id > temp_data_MAX_id || \
                temp_data_MAX_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
            {
                data_max_i = i;
                temp_data_MAX_id = data_block.data_id;
            }
        }
    }

    //@def 如果读出来的结果是有ID的，遍历ID最大的这一页，寻找ID的最大值
    if(temp_data_MAX_id != CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        data_start_id = temp_data_MAX_id;
        data_max_addr = temp_cfs_handle->addr_handle + \
            (data_max_i + 1) * temp_cfs_handle->sector_size;
        cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED;
        while(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
        {
            memset(data_block.data_pointer, NULL, temp_cfs_handle->data_size);
            data_block.data_id = data_start_id;
            read_result = cfs_system_oc_read_flash_data( \
                temp_linked_object, &data_block);
            
            //@def 读错就在往前读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED && \
                data_block.data_id > temp_data_MAX_id)
            {
                temp_data_MAX_id = data_block.data_id;
            }
            else if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL)
            {
                break;
            }

            //@def 下方ID+1计算出地址后加上两个数据块的大小后，
            //@def 在减 1 得出在往上加上一个ID长度有没有超过这一页。
            if(data_max_addr > (cfs_system_oc_via_id_calculate_addr( \
                temp_linked_object, data_start_id) + data_block_size * 2 - 1))
            {
                data_start_id++;
            }
            else
            {
                break;
            }
        }
    }

    return  temp_data_MAX_id;
}


//@def 检查重复地址
static bool cfs_filesystem_check_flash_repeat_address(const cfs_system *temp_object)
{
    //@def 肯定不能超过uint32类型最大值
    assert((temp_object->addr_handle + \
        (temp_object->sector_size * temp_object->sector_count)) <= UINT_MAX);

    if(cfs_system_oc_flash_repeat_address(temp_object) == true)
    {
        return true;
    }

    return false;
}


//@def 进行ID初始化操作
static uint32_t cfs_filesystem_object_id_init( cfs_system_handle_t temp_cfs_handle)
{
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_cfs_handle);
    assert(temp_object != NULL);

    uint32_t temp_data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;

    if(cfs_system_oc_object_struct_type_get(temp_object) != \
        CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE)
    {
        //@def 紧密存储数据
        temp_data_id = cfs_filesystem_tight_data_page_id_init(temp_object);
    }

	//@def 设置遍历好的ID值
    cfs_system_oc_object_id_set(temp_object, temp_data_id);

    //@def 判断有没有ID
    if(temp_data_id != CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        temp_data_id = cfs_system_oc_valid_data_number(temp_object);
    }
    else
    {
        temp_data_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }
    //@def 设置目前可用的ID数量
    cfs_system_oc_object_valid_id_number_set(temp_object, temp_data_id);

    return true;
}

static cfs_system_handle_t cfs_filesystem_object_add_oc_object(cfs_system *temp_object)
{
    cfs_object_linked_list *temp_linked_object = cfs_system_oc_add_object(temp_object);
    if(temp_linked_object == NULL)
    {
        return 0;
    }
    
    // 测试-------
    temp_cfs_object_linked_list = temp_linked_object;
    // ------------
    
    cfs_system_handle_t return_handle = \
        (((cfs_system_handle_t)temp_linked_object) << 16) + \
        cfs_system_utils_crc16_xmodem_check( \
        (uint8_t *)temp_linked_object, sizeof(temp_linked_object), true);

    return return_handle;
}

//@def 存储固定数据——写入数据,写入成功返回写入的原始数据长度
static uint32_t cfs_filesystem_fixed_data_write( \
    cfs_object_linked_list *temp_object, \
    uint32_t write_id, uint8_t *data, uint16_t len)
{
    cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
    cfs_system *temp_cfs_object = cfs_system_oc_system_object_get(temp_object);
    const uint32_t  temp_max_id = \
        temp_cfs_object->sector_size * temp_cfs_object->sector_count / \
        (temp_cfs_object->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);

    bool flash_data_block_is_null = true;  
    if(write_id >= temp_max_id)
    {
        //@def 固定存储，ID不能超过最大ID数
        assert(write_id < temp_max_id);
        return NULL;
    }

    //@def 填充数据
    cfs_data_block temp_data_block;
    temp_data_block.data_id = write_id;
    cfs_system_oc_object_block_buffer_set(temp_object, &temp_data_block);
    memcpy(temp_data_block.data_pointer, data, len);

    flash_data_block_is_null = \
        cfs_system_oc_flash_checking_null_values(temp_object, &temp_data_block);
    
    if( flash_data_block_is_null == true)
    {
        read_result = cfs_system_oc_add_write_flash_data(temp_object, &temp_data_block);
    }
    else
    {
        read_result = cfs_system_oc_set_write_flash_data(temp_object, &temp_data_block);
    }
    
    if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
    {
        return len;
    }
    
    return NULL;
}

//@def 循环存储数据——写入数据,写入成功返回写入的原始数据长度
static uint32_t cfs_filesystem_cycle_data_write( \
    cfs_object_linked_list *temp_object, \
    uint32_t write_id, uint8_t *data, uint16_t len)
{
    cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
    uint32_t temp_id = cfs_system_oc_object_id_get(temp_object);
    uint16_t temp_valid_id = cfs_system_oc_object_valid_id_number_get(temp_object);

    // 填充数据
    cfs_data_block temp_data_block;
    temp_data_block.data_id = write_id;
    cfs_system_oc_object_block_buffer_set(temp_object, &temp_data_block);
    memcpy(temp_data_block.data_pointer, data, len);

    // 每次增加的ID不能跳，只能一个一个往上加
    if((write_id > temp_id && ((write_id - temp_id) == 1))|| \
        (temp_id == CFS_CONFIG_NOT_LINKED_DATA_ID && write_id == 0))
    {
        read_result = cfs_system_oc_add_write_flash_data(temp_object, &temp_data_block);

        if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
        {
            cfs_system_oc_object_id_set(temp_object, write_id);
            cfs_system_oc_object_valid_id_number_set( \
                temp_object, cfs_system_oc_valid_data_number(temp_object));
        }
    }
    else if((temp_id-temp_valid_id) < write_id && write_id <= temp_id)
    {
        read_result = cfs_system_oc_set_write_flash_data(temp_object, &temp_data_block);
    }

    if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
    {
        return len;
    }

    /*user designation codes*/
    return NULL;
}

//@def 读取数据,读取成功返回读取的原始数据长度
static uint32_t cfs_filesystem_flsh_data_read( \
    cfs_object_linked_list *temp_object, \
    uint32_t read_id, uint8_t *data, uint16_t len)
{
    cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
    
    cfs_data_block temp_data_block;
    temp_data_block.data_id = read_id;
    cfs_system_oc_object_block_buffer_set(temp_object, &temp_data_block);

    //@def 读取内存中的数据,会验证crc8
    read_result = cfs_system_oc_read_flash_data(temp_object, &temp_data_block);

    if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
    {
        memcpy(data, temp_data_block.data_pointer, len);
        return len;
    }
    
    return false;
}

//*******************************************************************************************
//-- 对外接口  
//*******************************************************************************************

cfs_system_handle_t cfs_nv_object_init(cfs_system *temp_object)
{
    //@def 判断参数有效性
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
        //@def 检查参数
        return false;
    }

    //@def 在判断地址有没有重复
    if(true == cfs_filesystem_check_flash_repeat_address(temp_object))
    {
			cfs_filesystem_check_flash_repeat_address(temp_object);
        /*内存参数交叉了！*/
        assert(false);
        return false;
    }

    //@def 初始化本地flash
    if(cfs_port_system_flash_init() != true)
    {
        return false;
    }
    
    /*开始初始化*/
    cfs_system_handle_t new_cfs_object_handle = \
        cfs_filesystem_object_add_oc_object(temp_object);		
    if(new_cfs_object_handle == false)
    {
        return false;
    }

    //@def 初始化对象的各种ID
    cfs_filesystem_object_id_init(new_cfs_object_handle);

    /*初始化工作结束，返回初始化的句柄*/
    return new_cfs_object_handle;
}

bool cfs_nv_object_delete(cfs_system_handle_t temp_object_handle)
{
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_object_handle);
    if(temp_object == NULL)
    {
        return false;
    }

    return cfs_system_oc_object_delete(temp_object);
}

//@def 根据id往内存中写入数据
uint32_t cfs_nv_write(cfs_system_handle_t temp_object_handle, \
	uint32_t temp_id, uint8_t *data, uint16_t len)
{
    uint32_t result_len = NULL;
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_object_handle);
    cfs_system *temp_cfs_object = cfs_system_oc_system_object_get(temp_object);
    if(temp_object == NULL || temp_id == CFS_CONFIG_NOT_LINKED_DATA_ID || \
        len > temp_cfs_object->data_size)
    {
        return result_len;
    }

    switch (cfs_system_oc_object_struct_type_get(temp_object))
    {
        case CFS_FILESYSTEM_OBJECT_TYPE_NULL:
            //@def 不应该出现这种情况
            assert(false);
            return result_len;
        
        case CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE:
            result_len = \
                cfs_filesystem_fixed_data_write(temp_object, temp_id, data, len);
            break;
        
        case CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH:
            result_len = \
                cfs_filesystem_cycle_data_write(temp_object, temp_id, data, len);
            break;
        
        default:
            break;
    }
    
    return result_len;
}

//@def 根据ID读取内存中的数据
uint32_t cfs_nv_read(cfs_system_handle_t temp_object_handle, \
	uint32_t read_id, uint8_t *data, uint32_t len)
{
    uint32_t result_len = NULL;
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_object_handle);
    cfs_system *temp_cfs_object = cfs_system_oc_system_object_get(temp_object);
    if(temp_object == NULL || read_id == CFS_CONFIG_NOT_LINKED_DATA_ID || \
        len > temp_cfs_object->data_size)
    {
        return false;
    }

    
    const uint32_t  temp_max_id = \
        temp_cfs_object->sector_size * temp_cfs_object->sector_count / \
        (temp_cfs_object->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    uint32_t temp_id = cfs_system_oc_object_id_get(temp_object);
    uint16_t temp_valid_id = cfs_system_oc_object_valid_id_number_get(temp_object);
    
    switch (cfs_system_oc_object_struct_type_get(temp_object))
    {
        case CFS_FILESYSTEM_OBJECT_TYPE_NULL:
            //@def 不应该出现这种情况
            assert(false);
            return result_len;
        
        case CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE:
            if(read_id >= temp_max_id)
            {
                //@def 读取数据，ID不能超过最大ID数
                assert(read_id < temp_max_id);
                return result_len;
            }
            break;
        
        case CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH:
            if((temp_id-temp_valid_id) >= read_id && read_id > temp_id)
            {
                return result_len;
            }
            break;
        
        default:
            break;
    }

    result_len = \
        cfs_filesystem_flsh_data_read(temp_object, read_id, data, len);
    
    return result_len;
}

//@def 清除指定对象的存储空间
bool cfs_nv_clear(cfs_system_handle_t temp_object_handle)
{
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_object_handle);
    if(temp_object == NULL)
    {
        return false;
    }

    if(cfs_system_oc_flash_data_clear(temp_object) == true)
    {
        temp_object->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
        temp_object->valid_id_number = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }
    else
    {
        //@def 不应该到这里
        assert(false);
        return false;
    }
    
    return true;
}

//@def 返回目前存储对象的ID
uint32_t cfs_nv_get_current_id(cfs_system_handle_t temp_object_handle)
{
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_object_handle);
    if(temp_object == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    return cfs_system_oc_object_id_get(temp_object);
}

//@def 返回目前存储对象的可用ID
uint32_t cfs_nv_get_current_valid_id(cfs_system_handle_t temp_object_handle)
{
    cfs_object_linked_list *temp_object = \
        cfs_system_oc_object_linked_crc_16_verify(temp_object_handle);
    if(temp_object == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    return cfs_system_oc_object_valid_id_number_get(temp_object);
}
