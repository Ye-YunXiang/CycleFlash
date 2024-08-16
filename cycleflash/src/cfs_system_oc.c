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

// *****************************************************************************************************
// 其他接口 —— 接口

// 如果为空值返回true
bool cfs_system_oc_flash_checking_null_values( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer)
{
    uint32_t addr = cfs_system_oc_via_id_calculate_addr(temp_object, buffer->data_id);
    uint32_t data_block_len = \
        buffer->data_len + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;
    
    return cfs_port_system_flash_read_checking_null_values(addr, data_block_len);
}

// 根据ID计算有效数据个数
uint32_t cfs_system_oc_valid_data_number( \
    const cfs_object_linked_list *temp_linked_object)
{
    uint32_t result_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;

    cfs_system *temp_cfs_object = temp_linked_object->object_handle;
    if(temp_linked_object->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        return CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }

	uint32_t data_size = \
        temp_cfs_object->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;
	uint32_t all_data = \
        (temp_cfs_object->sector_size * temp_cfs_object->sector_count) / data_size;
	uint16_t data_cycle = (temp_linked_object->data_id + 1) / all_data;
	uint16_t data_cycle_int = (temp_linked_object->data_id + 1) % all_data;
	
	if(data_cycle < 1 || (data_cycle == 1 && data_cycle_int == 0))
	{
		result_id = temp_linked_object->data_id + 1;
	}
	else
	{
        if(temp_cfs_object->sector_count > 1)
        {
            // result_id = all_data - (temp_cfs_object->sector_size / data_size) - 1;
            uint32_t temp_id_end_addr = cfs_system_oc_via_id_calculate_addr( \
                temp_linked_object, temp_linked_object->data_id) + data_size;
            if((temp_linked_object->data_id + 1) % all_data == 0)
            {
                // 刚好满
                result_id = all_data;
            }
            else
            {
                // 还没满
                result_id = \
                    (all_data - ((((temp_id_end_addr / temp_cfs_object->sector_size) + 1)\
                    * temp_cfs_object->sector_size - temp_cfs_object->addr_handle) / \
                    data_size) - 1) + (temp_linked_object->data_id + 1) % all_data;
            }
        }
        else
        {
            result_id = data_cycle_int == 0 ? all_data : data_cycle_int;
        }
	}

    return result_id;
}

// 链表添加一个数据对象
cfs_object_linked_list *cfs_system_oc_add_object(cfs_system *object_pointer) 
{
    cfs_object_linked_list *new_node = 
        (cfs_object_linked_list *)CFS_MALLOC(sizeof(cfs_object_linked_list));
    if (new_node == NULL) 
    {
        /* Allocation failure */
        return NULL;
    }
    cfs_system *new_cfs_node = (cfs_system *)CFS_MALLOC(sizeof(cfs_system));
    if (new_cfs_node == NULL) 
    {
        CFS_FREE(new_node);
        /* Allocation failure */
        return NULL;
    }
    new_cfs_node->addr_handle = object_pointer->addr_handle;
    new_cfs_node->sector_size = object_pointer->sector_size;
    new_cfs_node->sector_count = object_pointer->sector_count;
    new_cfs_node->data_size = object_pointer->data_size;
    new_cfs_node->struct_type = object_pointer->struct_type; 

    if (cfs_system_object_head == NULL)
    {
        cfs_system_object_head = new_node;
        new_node->prior = NULL;
        new_node->next = NULL;
        cfs_system_object_tail = new_node;
    }
    else
    {
        cfs_system_object_tail->next = new_node;
        new_node->prior = cfs_system_object_tail;
        new_node->next = NULL;
        cfs_system_object_tail = new_node;
    }

    new_node->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    new_node->valid_id_number = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    new_node->object_handle = new_cfs_node;
    new_node->this_linked_addr_crc_16 = \
        cfs_system_utils_crc16_xmodem_check( \
        (uint8_t *)(new_node), sizeof(new_node->object_handle), true);

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
            (temp_object->sector_size * temp_object->sector_count) - 1;

        cfs_object_linked_list *temp_pointer = cfs_system_object_head->next;
        
        while(temp_pointer != NULL) 
        {
            uint32_t head_2 = temp_pointer->object_handle->addr_handle;
            uint32_t tail_2 = temp_pointer->object_handle->addr_handle + \
                (temp_pointer->object_handle->sector_size * \
                temp_pointer->object_handle->sector_count) - 1;

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

// 根据ID得到ID对应的内存地址
uint32_t cfs_system_oc_via_id_calculate_addr( \
    const cfs_object_linked_list *temp_object, uint32_t temp_id)
{
    assert(temp_id != CFS_CONFIG_NOT_LINKED_DATA_ID);
    cfs_system *temp_cfs_object = temp_object->object_handle;

	uint32_t data_size = \
        temp_cfs_object->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;
	uint32_t all_data = \
        (temp_cfs_object->sector_size * temp_cfs_object->sector_count) / data_size;
	uint16_t data_cycle = (temp_id + 1) / all_data;
	uint16_t data_cycle_int = (temp_id + 1) % all_data;
	
	uint32_t result_addr = NULL;
	
	if(data_cycle < 1 || (data_cycle == 1 && data_cycle_int == 0))
	{
		result_addr = temp_id * data_size;
	}
	else if(data_cycle >= 1 && data_cycle_int != 0)
	{
		result_addr = (temp_id - data_cycle * all_data) * data_size;
	}
	else if(data_cycle > 1 && data_cycle_int == 0)
	{
		result_addr = (temp_id - (data_cycle - 1) * all_data) * data_size;
	}

    result_addr = result_addr + temp_cfs_object->addr_handle;

    return result_addr;
}
// *****************************************************************************************************
// 写入和读取数据 —— 内部处理

static bool __read_flash_data_block( \
    volatile uint32_t addr, cfs_data_block * block, cfs_system *temp_cfs)
{
    cfs_port_system_flash_lock_enable();

    cfs_port_system_flash_read(\
        addr, (uint8_t *)(&block->data_id), sizeof(block->data_id));
    addr += sizeof(block->data_id);
    cfs_port_system_flash_read(\
        addr, block->data_pointer, block->data_len);
    addr += temp_cfs->data_size;
    cfs_port_system_flash_read(\
        addr, (uint8_t *)(&block->data_crc_16), sizeof(block->data_crc_16));

    cfs_port_system_flash_lock_disable();

    return true;
}

static bool __write_flash_data( \
    volatile uint32_t addr, uint8_t * buffer, uint16_t len)
{
    cfs_port_system_flash_lock_enable();

    while(len != 0)
    {
        if(addr % 4 == 0 && len >= 4)
        {
            cfs_port_system_flash_write_word(addr, buffer, len / 4);
            buffer += (len / 4) * 4;
            addr += (len / 4) * 4;
            len -= (len / 4) * 4;
        }
        else if(addr % 2 == 0 && len >= 2)
        {
            cfs_port_system_flash_write_half_word(addr, buffer, len / 2);
            buffer += (len / 2) * 2;
            addr += (len / 2) * 2;
            len -= (len / 2) * 2;
        }
        else
        {
            cfs_port_system_flash_write_byte(addr, buffer, 1);
            buffer++;
            addr++;
            len--;
        }
    }

    cfs_port_system_flash_lock_disable();
    return true;
}

static bool __write_flash_data_block( \
    volatile uint32_t addr, cfs_data_block * block, cfs_system *temp_cfs)
{
    // 根据字符大小定制的写入逻辑
    assert(sizeof(block->data_id) == 4 && \
        sizeof(block->data_len) == 2 && sizeof(block->data_crc_16) == 2);
    
    cfs_port_system_flash_lock_enable();

    __write_flash_data(addr, (uint8_t *)(&block->data_id), sizeof(block->data_id));
    addr += sizeof(block->data_id);
    __write_flash_data(addr, block->data_pointer, block->data_len);
    addr += temp_cfs->data_size;
    __write_flash_data(addr, (uint8_t *)(&block->data_crc_16), sizeof(block->data_crc_16));

    cfs_port_system_flash_lock_disable();

    return true;
}

static bool __contrast_flash_data_block( \
    volatile uint32_t addr, const cfs_data_block * block, cfs_system *temp_cfs)
{
    bool result = false;
    cfs_port_system_flash_lock_enable();

    while(true)
    {
        result = cfs_port_system_flash_read_contrast( \
            addr, (uint8_t *)(&block->data_id), sizeof(block->data_id));
        if(result == false)
        {
            break;
        }

        addr += sizeof(block->data_id);
        cfs_port_system_flash_read_contrast( \
            addr, block->data_pointer, block->data_len);
        if(result == false)
        {
            break;
        }

        addr += temp_cfs->data_size;
        cfs_port_system_flash_read_contrast( \
            addr, (uint8_t *)(&block->data_crc_16), sizeof(block->data_crc_16));

        break;
    }
    cfs_port_system_flash_lock_disable();

    return result;
}



// *****************************************************************************************************
// 写入、读取数据、删除 —— 接口

// 读取内存中的数据,会验证crc8
cfs_oc_action_data_result cfs_system_oc_read_flash_data( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer)
{
    assert(buffer->data_pointer != NULL || buffer->data_len >= 1);

    const uint32_t addr = cfs_system_oc_via_id_calculate_addr(temp_object, buffer->data_id);
    cfs_system *temp_cfs = cfs_system_oc_system_object_get(temp_object);

    cfs_oc_action_data_result result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;

    volatile uint8_t i = 2;
    while(i--)
    {
        __read_flash_data_block(addr, buffer, temp_cfs);
        uint16_t check_crc_16 = cfs_system_utils_crc16_xmodem_check_data_block(buffer, true);

        if(buffer->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
        }
        else if(check_crc_16 == buffer->data_crc_16)
        {
            result = CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED;
            break;
        }
        else
        {
            buffer->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
            result = CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE;
        }
    }

    return result;
}

// 往内存中写入新的数据，增加式
cfs_oc_action_data_result cfs_system_oc_add_write_flash_data( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer)
{
    assert(buffer != NULL && \
        buffer->data_len >= 1 && buffer->data_id != CFS_CONFIG_NOT_LINKED_DATA_ID);

    cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
    cfs_system *temp_cfs = cfs_system_oc_system_object_get(temp_object);
    const uint32_t data_addr = \
        cfs_system_oc_via_id_calculate_addr(temp_object, buffer->data_id);
    const uint32_t start_addr = \
        (data_addr / temp_cfs->sector_size) * temp_cfs->sector_size;
    const uint32_t max_addr = start_addr + temp_cfs->sector_size;
    const uint32_t data_block_lent = \
        temp_cfs->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;

    buffer->data_crc_16 = cfs_system_utils_crc16_xmodem_check_data_block(buffer, true);

    if(data_addr == temp_cfs->addr_handle) 
    {
        cfs_port_system_flash_erasing_page(data_addr, 1);
    }
    else if((data_addr + data_block_lent) >= max_addr)
    {
        cfs_port_system_flash_erasing_page(max_addr, 1);
    }
    __write_flash_data_block(data_addr, buffer, temp_cfs);

    if(__contrast_flash_data_block(data_addr, buffer, temp_cfs) == false)
    {
        read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE;
    }
    else
    {
        read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED;
    }

    return read_result;
}

uint8_t data_buffer_temp[1024];

// 修改内存中的数据
cfs_oc_action_data_result cfs_system_oc_set_write_flash_data( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer)
{
    assert(buffer != NULL && \
        buffer->data_len >= 1 && buffer->data_id != CFS_CONFIG_NOT_LINKED_DATA_ID);
        
    cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
    cfs_system *temp_cfs_objecr = cfs_system_oc_system_object_get(temp_object);
    // 数据地址
    const uint32_t data_addr = \
        cfs_system_oc_via_id_calculate_addr(temp_object, buffer->data_id);
    // 数据所在页开始地址
    const uint32_t start_addr = \
        (data_addr / temp_cfs_objecr->sector_size) * temp_cfs_objecr->sector_size;
    // 数据块的长度
    const uint32_t data_block_lent = \
        temp_cfs_objecr->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;

    uint8_t *read_sector_data = NULL;
    uint8_t *temo_read_sector_data = NULL;
    uint16_t read_page_count = 1;
    // 减一得到结束地址
    if((data_addr + data_block_lent - 1) >= start_addr + temp_cfs_objecr->sector_size)
    {
        read_page_count = 2;
    }

    //read_sector_data = \
        (uint8_t *)CFS_MALLOC(temp_cfs_objecr->sector_size * read_page_count);
    read_sector_data = &data_buffer_temp[0];
    temo_read_sector_data = read_sector_data;
    
    uint16_t temp_data_crc_16 = cfs_system_utils_crc16_xmodem_check_data_block(buffer, true);

    cfs_port_system_flash_read(\
        start_addr, read_sector_data, temp_cfs_objecr->sector_size * read_page_count);
    temo_read_sector_data = read_sector_data + data_addr - start_addr;
    memset(temo_read_sector_data, CFS_FLASH_ERASURE, \
        temp_cfs_objecr->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    memcpy(temo_read_sector_data, &buffer->data_id, sizeof(buffer->data_id));
    temo_read_sector_data += sizeof(buffer->data_id);
    memcpy(temo_read_sector_data, buffer->data_pointer, buffer->data_len);
    temo_read_sector_data += temp_cfs_objecr->data_size;
    memcpy(temo_read_sector_data, &temp_data_crc_16, sizeof(buffer->data_crc_16));

    cfs_port_system_flash_lock_enable();
    cfs_port_system_flash_erasing_page(start_addr, read_page_count);
    __write_flash_data(\
        start_addr, read_sector_data, temp_cfs_objecr->sector_size * read_page_count);
    cfs_port_system_flash_lock_disable();

    if(cfs_port_system_flash_read_contrast( \
        start_addr, read_sector_data, \
        temp_cfs_objecr->sector_size * read_page_count) == false)
    {
        read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE;
    }
    else
    {
        read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED;
    }

    //CFS_FREE(read_sector_data);

    return read_result;
}

bool cfs_system_oc_flash_data_clear(const cfs_object_linked_list *temp_object)
{
    cfs_port_system_flash_erasing_page( \
        temp_object->object_handle->addr_handle, \
        temp_object->object_handle->sector_count);
    return true;
}

bool cfs_system_oc_object_delete(cfs_object_linked_list *temp_object)
{
    if(temp_object == NULL)
    {
        return false;
    }

    if(temp_object->object_handle != NULL)
    {
        CFS_FREE(temp_object->object_handle);
        temp_object->object_handle = NULL;
    }

    if(cfs_system_object_head == temp_object)
    {
        if(cfs_system_object_tail == temp_object)
        {
            cfs_system_object_head = NULL;
            cfs_system_object_tail = NULL;
        }
        else
        {
            cfs_system_object_head = temp_object->next;
            temp_object->next->prior = NULL;
        }
    }
    else
    {
        if(cfs_system_object_tail == temp_object)
        {
            cfs_system_object_tail = temp_object->prior;
        }
        else
        {
            temp_object->prior->next = temp_object->next;
            temp_object->next->prior = temp_object->prior;
        }
    }

    CFS_FREE(temp_object);
    return true;
}

// *****************************************************************************************************
// 设置和获取对象 —— 接口

/*设置数据数据对象的ID*/
bool cfs_system_oc_object_id_set( \
    cfs_object_linked_list * temp_cfs_handle, uint32_t temp_id)
{
    temp_cfs_handle->data_id = temp_id;
    return true;
}


// 得到数据数据对象的ID
uint32_t cfs_system_oc_object_id_get(const cfs_object_linked_list *temp_cfs_handle)
{
    return temp_cfs_handle->data_id;
} 

/*设置数据数据对象的可用ID*/
bool cfs_system_oc_object_valid_id_number_set( \
    cfs_object_linked_list * temp_cfs_handle, uint16_t temp_id)
{
    temp_cfs_handle->valid_id_number = temp_id;
    return true;
}


// 得到数据数据对象的可用ID
uint16_t cfs_system_oc_object_valid_id_number_get( \
    const cfs_object_linked_list *temp_cfs_handle)
{
    return temp_cfs_handle->valid_id_number;
} 

// 得到数据对象的类型
uint8_t cfs_system_oc_object_struct_type_get( \
    const cfs_object_linked_list *temp_cfs_handle)
{
    return (uint8_t)temp_cfs_handle->object_handle->struct_type;
} 


// 得到系统数据对象指针
cfs_system *cfs_system_oc_system_object_get(const cfs_object_linked_list *temp_object)
{
    return temp_object->object_handle;
}


/*使用初始化链表对象后返回的句柄，在通过crc-16-xmodem标识验证链表对象是否存在*/
// 存在返回链表对象，不存在返回NULL
cfs_object_linked_list * cfs_system_oc_object_linked_crc_16_verify( \
    cfs_system_handle_t temp_cfs_handle)
{
    cfs_object_linked_list *temp_object = \
        (cfs_object_linked_list *)((uint32_t)(temp_cfs_handle>>16));
    uint16_t temp_crc_16 = (uint16_t)(temp_cfs_handle);

    if(temp_object->this_linked_addr_crc_16 == temp_crc_16)
    {
        return temp_object;
    }

    return NULL;
}
