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
#include <limits.h>

#include "cfs_system_memory.h"

#include "cfs_port_device_flash.h"
#include "cfs_system_utils.h"


// ************************************************************************
//@def 写入和读取数据 —— 内部处理

// static bool __read_flash_data_block(
//     volatile uint32_t addr, cfs_data_block * block, cfs_system *temp_cfs)
// {
//     cfs_port_system_flash_read(
//         addr, (uint8_t *)(&block->data_id), sizeof(block->data_id));

//     addr += sizeof(block->data_id);
//     cfs_port_system_flash_read(
//         addr, block->data_pointer, block->data_len);
        
//     addr += temp_cfs->data_size;
//     cfs_port_system_flash_read(
//         addr, (uint8_t *)(&block->data_crc_16), sizeof(block->data_crc_16));

//     return true;
// }

// HACK: 新
static bool _erasing_flash_page( volatile uint32_t addr, uint16_t page)
{
    cfs_port_system_flash_lock_enable();

    cfs_port_system_flash_erasing_page(addr, page);

    cfs_port_system_flash_lock_disable();

    return true;
}

static bool _write_flash_data(
    volatile uint32_t addr, uint8_t * buffer, uint16_t len)
{
    uint16_t write_byte_all_len = 0;

#ifdef CFS_WRITE_PORT_DOUBLE_WORD
    const uint16_t WRITE_DOUBLR_WORD_LEN = (len - write_byte_all_len) / 8;
    write_byte_all_len += WRITE_DOUBLR_WORD_LEN * 8;
#endif // CFS_WRITE_PORT_DOUBLE_WORD

#ifdef CFS_WRITE_PORT_ONE_WORD
    const uint16_t WRITE_WORD_LEN = (len - write_byte_all_len) / 4;
    write_byte_all_len += WRITE_WORD_LEN * 4;
#endif // CFS_WRITE_PORT_ONE_WORD

#ifdef CFS_WRITE_PORT_HALF_WORD
    const uint16_t WRITE_HALF_WORD_LEN = (len - write_byte_all_len) / 2;
    write_byte_all_len += WRITE_HALF_WORD_LEN * 2;
#endif // CFS_WRITE_PORT_HALF_WORD

#ifdef CFS_WRITE_PORT_ONE_BYTE
    const uint16_t WRITE_BYTE_LEN = len - write_byte_all_len;
    write_byte_all_len += WRITE_BYTE_LEN;
#endif // CFS_WRITE_PORT_ONE_BYTE


    if (write_byte_all_len != len)
    {
        return false;
    }

    cfs_port_system_flash_lock_enable();

#ifdef CFS_WRITE_PORT_ONE_BYTE
    if (WRITE_BYTE_LEN != 0)
    {
        cfs_port_system_flash_write_byte(addr, buffer, WRITE_BYTE_LEN);
        addr += WRITE_BYTE_LEN;
        buffer += WRITE_BYTE_LEN;
    }
#endif // CFS_WRITE_PORT_ONE_BYTE

#ifdef CFS_WRITE_PORT_HALF_WORD
    if (WRITE_HALF_WORD_LEN != 0)
    {
        cfs_port_system_flash_write_byte(addr, buffer, WRITE_HALF_WORD_LEN);
        addr += WRITE_HALF_WORD_LEN * 2;
        buffer += WRITE_HALF_WORD_LEN * 2;
    }
#endif // CFS_WRITE_PORT_HALF_WORD

#ifdef CFS_WRITE_PORT_ONE_WORD
    if (WRITE_WORD_LEN != 0)
    {
        cfs_port_system_flash_write_byte(addr, buffer, WRITE_WORD_LEN);
        addr += WRITE_WORD_LEN * 4;
        buffer += WRITE_WORD_LEN * 4;
    }
#endif // CFS_WRITE_PORT_ONE_WORD

#ifdef CFS_WRITE_PORT_DOUBLE_WORD
    if (WRITE_DOUBLR_WORD_LEN != 0)
    {
        cfs_port_system_flash_write_byte(addr, buffer, WRITE_DOUBLR_WORD_LEN);
    }
#endif // CFS_WRITE_PORT_DOUBLE_WORD

    cfs_port_system_flash_lock_disable();

    return true;
}


// HACK: 新
static bool _write_flash_data_block(
    volatile uint32_t addr, const cfs_data_block_t *write_block)
{   
    const uint16_t DATA_FILL = cfs_memory_compute_memory_fill_length(write_block->data_len);
    bool result = false;

    cfs_port_system_flash_lock_enable();

    result = _write_flash_data(addr,
                      (uint8_t *)write_block,
                      CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    addr += CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;

    if (DATA_FILL == 0)
    {
        // 不需要填充
        result = _write_flash_data(addr, write_block->data_ptr, write_block->data_len);
    }
    else
    {
        // 计算填充字节并存入
        uint8_t fill_data[CFS_WRITE_MIN_PARTICLE] = {0};
        const uint16_t SUBSECTION_DATA_LEM =
            write_block->data_len - (CFS_WRITE_MIN_PARTICLE - DATA_FILL);
        result = _write_flash_data(addr, write_block->data_ptr, SUBSECTION_DATA_LEM);

        memcpy(fill_data,
               &write_block->data_ptr[SUBSECTION_DATA_LEM],
               write_block->data_len - SUBSECTION_DATA_LEM);
        result = _write_flash_data(addr += SUBSECTION_DATA_LEM,
                          write_block->data_ptr,
                          SUBSECTION_DATA_LEM);
    }

    cfs_port_system_flash_lock_disable();

    return true;
}

// HACK: 新
static bool _contrast_flash_data_block(
    const uint32_t addr, const cfs_data_block_t *contrast_block)
{
    bool result = false;

    // XXX：这边是内部为大部分32位MCU可用的方法。
    for (uint8_t i=0; i<2; i++)
    {
        if (0 != memcmp((uint8_t *)addr,
                        (uint8_t *)contrast_block,
                        CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN))
        {
            continue;
        }

        if (0 != memcmp((uint8_t *)(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
                        contrast_block->data_ptr,
                        contrast_block->data_len))
        {
            return true;
        }
    }

    return false;
}

// HACK: 新
static bool _checking_flash_block_is_null_values(const uint32_t addr, uint16_t len)
{
    for(uint16_t i=0; i<len; i++)
    {
        // XXX：这边是内部为大部分32位MCU可用的方法。
        if(CFS_FLASH_ERASURE != ((uint8_t *)addr)[i])
        {
            // 不为空
            return false;
        }
    }

    return true;
}



// // HACK: 新
// static bool _read_flash_fixed_data(
//     const uint32_t address, uint8_t *buffer, uint16_t read_len)
// {
//     return cfs_port_system_flash_read(address, buffer, read_len);
// }


// *************************************************************************
//@def 其他接口 —— 接口

// HACK: 新
// 计算需要填充的字节数
uint8_t cfs_memory_compute_memory_fill_length(uint16_t data_size)
{
    // 判断一下不能为0
    assert(data_size != 0);
    uint8_t data_fill_len = data_size % CFS_WRITE_MIN_PARTICLE;
    if (data_fill_len == 0)
    {
        return 0;
    }
    else
    {
        return (CFS_WRITE_MIN_PARTICLE - data_fill_len);
    }
}


// HACK: 新
//@def 通过可用ID计算要存入的地址位置。
/**
 * 
 */
uint32_t cfs_memory_calculate_fixed_id_flash_address(
    const cfs_object_list_t *object_list, cfs_data_id_t id_input)
{
    // 不能让无ID的情况到这里，刚开始就要被滤掉
    // if (object_list->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
    // {
    //     return 0;
    // }

    uint32_t result_addr = NULL;

    const cfs_data_id_t FLASH_MAX_ID_COUNT = 
        (object_list->object_handle->sector_count * CFS_FLASH_SECTOR_SIZE)
        / object_list->data_buffer_size;

	cfs_data_id_t data_cycle = (id_input + 1) / FLASH_MAX_ID_COUNT;
	cfs_data_id_t data_cycle_int = (id_input + 1) % FLASH_MAX_ID_COUNT;
	
	if(data_cycle < 1 || (data_cycle == 1 && data_cycle_int == 0))
	{
		result_addr = id_input * object_list->data_buffer_size;
	}
	else if(data_cycle >= 1 && data_cycle_int != 0)
	{
		result_addr = (id_input - data_cycle*FLASH_MAX_ID_COUNT) 
            * object_list->data_buffer_size;
	}
	else if(data_cycle > 1 && data_cycle_int == 0)
	{
		result_addr = (id_input - (data_cycle-1)*FLASH_MAX_ID_COUNT) 
            * object_list->data_buffer_size;
	}

    return result_addr + object_list->object_handle->address;
}


// HACK: 新
//@def 根据ID计算有效数据个数
/**
 * 
 */
cfs_data_id_t cfs_memory_fixe_valid_id_number(
    const cfs_object_list_t *object_list, cfs_data_id_t id_input)
{
    uint32_t result_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;

    // cfs_system *temp_cfs_object = temp_linked_object->object_handle;
    if (id_input == CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        return CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }

    const cfs_data_id_t FLASH_MAX_ID_COUNT = 
        (object_list->object_handle->sector_count * CFS_FLASH_SECTOR_SIZE)
        / object_list->data_buffer_size;

    // 计算循环存储了几次后，剩余几个ID
	cfs_data_id_t data_cycle_int = (id_input + 1) % FLASH_MAX_ID_COUNT;
	
    // XXX: 这里需要注意，只要正好存满或者还未开始循环，都直接进入返回
    if (((id_input+1)/FLASH_MAX_ID_COUNT)<1 || data_cycle_int==0)
	{
		return (id_input + 1);
	}

    // 判断页数多余一页
    if(object_list->object_handle->sector_count > 1)
    {
        uint32_t data_id_end_addr = 
            cfs_memory_calculate_fixed_id_flash_address(object_list, id_input) 
            + object_list->data_buffer_size;

        result_id =
            (FLASH_MAX_ID_COUNT - ((((data_id_end_addr / CFS_FLASH_SECTOR_SIZE) + 1)
            * CFS_FLASH_SECTOR_SIZE - object_list->object_handle->address)
            / object_list->data_buffer_size) - 1) 
            + (id_input + 1) % FLASH_MAX_ID_COUNT;
    }
    else
    {
        // 如果存储区只有一页
        result_id = data_cycle_int;
    }

    return result_id;
}


// **************************************************************************
//@def 写入、读取数据、删除 —— 接口

// HACK: 新
//@def 在遍历ID阶段，遍历指定位置的数据，得到位置id
/**
 * 直接校验对象缓存长度的数据，对比读取出来的数据长度。
 * 返回： 如果数据有效，返回数据长度，否则返回0
 */
// TODO: 这里后面需要兼容自定义读取函数，后面在做
int cfs_memory_read_verify_flash_data_id(const cfs_object_list_t *object_list,
                                         const uint32_t address,
                                         cfs_data_id_t get_id)
{
    cfs_data_block_t read_block = {0};
    uint16_t get_checkout_value = 0;

    // XXX：这边是内部为大部分32位MCU定制的读取遍历方法。
    for (uint8_t i=0; i<2; i++)
    {
        memset(&read_block, 0, sizeof(cfs_data_block_t));
        memcpy((uint8_t *)&read_block,
               (uint8_t *)address,
               CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        get_checkout_value = cfs_system_utils_check(
            (uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
        
        if (read_block.data_len > object_list->object_handle->data_size
            || read_block.data_len == 0
            || read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            continue;
        }

        get_checkout_value += cfs_system_utils_check(
            (uint8_t *)(address + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
            read_block.data_len, 
            NULL);
        
        if (get_checkout_value == read_block.data_check)
        {
            get_id = read_block.data_id;
            return read_block.data_len;
        }
    }
    
    return CFS_RETURN_ERROR;
}


// HACK: 新
//@def 读取内存中指定的内存大小，经过数据校验正确后返回给中间层解析。
/**
 * 直接校验对象缓存长度的数据，对比读取出来的数据长度。
 *
 */
int cfs_memory_read_flash_fixed_data(const cfs_object_list_t *object_list,
                                     const uint32_t address,
                                     const data_max_len,
                                     uint8_t *data_buffer)
{
    cfs_data_block_t read_block = {0};
    // 在传入参数的时候，就要把参数滤干净
    uint16_t get_checkout_value = 0;

    // XXX：这边是内部为大部分32位MCU定制的读取遍历方法。
    for (uint8_t i=0; i<2; i++)
    {
        memset(data_buffer, 0, data_max_len);
        memset(&read_block, 0, sizeof(cfs_data_block_t));

        memcpy((uint8_t *)&read_block,
               (uint8_t *)address,
               CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        get_checkout_value = cfs_system_utils_check(
            (uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
        
        if (read_block.data_len > data_max_len 
            || read_block.data_len == 0
            || read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            continue;
        }

        get_checkout_value += cfs_system_utils_check(
            (uint8_t *)(address + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
            read_block.data_len, 
            data_buffer);
        
        if (get_checkout_value == read_block.data_check)
        {
            return read_block.data_len;
        }
    }
    
    return CFS_RETURN_ERROR;
}


// HACK: 新
//@def 添加式写入数据，确认写入正确后返回。
/**
 * 如果写入失败，会检测以下写入区域是否无数据，如果无数据就在尝试写入一次。
 * 以上全部失败，把本区域全部置为初始值的相反值，在返回错误。
 */
int cfs_memory_add_write_flash_fixed_data(const cfs_object_list_t *object_list,
                                          const uint32_t address,
                                          const uint16_t data_len,
                                          uint8_t *data_buffer)
{
    int write_result = CFS_RETURN_ERROR;
    cfs_data_block_t write_block = {0};
    write_block.data_ptr = data_buffer;
    write_block.data_len = data_len;
    write_block.data_check = cfs_system_utils_check(
        (uint8_t *)&write_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
    write_block.data_check += cfs_system_utils_check(data_buffer, data_len, NULL);
    

    // 判断是否要擦除页-----------------------
    const uint32_t MAX_ADDR = 
        (address / CFS_FLASH_SECTOR_SIZE) 
        * CFS_FLASH_SECTOR_SIZE + CFS_FLASH_SECTOR_SIZE;
    if(address == object_list->object_handle->address) 
    {
        _erasing_page_flash_data(address, 1);
    }
    else if((address + object_list->data_buffer_size) >= MAX_ADDR 
        && MAX_ADDR < (object_list->object_handle->address 
            + CFS_FLASH_SECTOR_SIZE * object_list->object_handle->sector_count))
    {
        _erasing_page_flash_data(MAX_ADDR, 1);
    }

    // 开始写入数据---------------------------
    for (uint8_t i=0; i<2; i++)
    {
        if (false == _checking_flash_block_is_null_values(
            address, data_len + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN))
        {
            continue;
        }

        _write_flash_data_block(address, &write_block);

        if(true == _contrast_flash_data_block(address, &write_block))
        {
            write_result = data_len;
        }
    }

    // 错误处理，方便后期上电遍历----------------------
    if (write_result == CFS_RETURN_ERROR)
    {
        memset(&write_block,
               ~CFS_FLASH_ERASURE,
               CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        _write_flash_data(address,
                          &write_block,
                          CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    }

    return write_result;
}


// HACK: 新
bool cfs_memory_flash_data_clear(const cfs_object_list_t *object_list)
{
    _erasing_flash_page(object_list->object_handle->address,
                        object_list->object_handle->sector_count);
    return true;
}



// //@def 修改内存中的数据
// cfs_oc_action_data_result cfs_system_oc_set_write_flash_data( 
//     const cfs_object_list_t *temp_object, cfs_data_block * buffer)
// {
//     assert(buffer != NULL && 
//         buffer->data_len >= 1 && buffer->data_id != CFS_CONFIG_NOT_LINKED_DATA_ID);
        
//     cfs_oc_action_data_result read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
//     cfs_system *temp_cfs_objecr = cfs_system_oc_system_object_get(temp_object);
//     //@def 数据地址
//     const uint32_t data_addr = 
//         cfs_system_oc_via_id_calculate_addr(temp_object, buffer->data_id);
//     //@def 数据所在页开始地址
//     const uint32_t start_addr = 
//         (data_addr / temp_cfs_objecr->sector_size) * temp_cfs_objecr->sector_size;
//     //@def 数据块的长度
//     const uint32_t data_block_lent = 
//         temp_cfs_objecr->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;

//     uint8_t *read_sector_data = NULL;
//     uint8_t *temo_read_sector_data = NULL;
//     uint16_t read_page_count = 1;
 
//     read_sector_data = &data_buffer_temp[0];
//     temo_read_sector_data = read_sector_data;
//     buffer->data_crc_16 = cfs_system_utils_crc16_xmodem_check_data_block(buffer, true);

//     //@def 减一得到结束地址
//     if((data_addr + data_block_lent - 1) >= start_addr + temp_cfs_objecr->sector_size)
//     {
//         cfs_port_system_flash_read(start_addr, read_sector_data, temp_cfs_objecr->sector_size);
//         memset(read_sector_data + data_addr - start_addr, CFS_FLASH_SECTOR_SIZE, 
//             start_addr + temp_cfs_objecr->sector_size - data_addr);

//         __erasing_flash_page(start_addr, 1);
//         __write_flash_data(\
//             start_addr, read_sector_data, temp_cfs_objecr->sector_size);
//         cfs_port_system_flash_lock_disable();

//         cfs_port_system_flash_read(start_addr + temp_cfs_objecr->sector_size, read_sector_data, temp_cfs_objecr->sector_size);
//         memset(read_sector_data, CFS_FLASH_SECTOR_SIZE, data_addr - start_addr + data_block_lent - temp_cfs_objecr->sector_size);

//         __erasing_flash_page(start_addr + temp_cfs_objecr->sector_size, 1);
//         __write_flash_data( start_addr + temp_cfs_objecr->sector_size, read_sector_data, temp_cfs_objecr->sector_size);

//         __write_flash_data_block(data_addr, buffer, temp_cfs_objecr);
//         cfs_port_system_flash_lock_disable();
//     }
//     else
//     {
//         cfs_port_system_flash_read(start_addr, read_sector_data, \
//             temp_cfs_objecr->sector_size * read_page_count);
//         temo_read_sector_data = read_sector_data + data_addr - start_addr;
//         memset(temo_read_sector_data, CFS_FLASH_SECTOR_SIZE, data_block_lent);
//         memcpy(temo_read_sector_data, &buffer->data_id, sizeof(buffer->data_id));
//         temo_read_sector_data += sizeof(buffer->data_id);
//         memcpy(temo_read_sector_data, buffer->data_pointer, buffer->data_len);
//         temo_read_sector_data += temp_cfs_objecr->data_size;
//         memcpy(temo_read_sector_data, &buffer->data_crc_16, sizeof(buffer->data_crc_16));

//         __erasing_flash_page(start_addr, read_page_count);
//         __write_flash_data(\
//             start_addr, read_sector_data, temp_cfs_objecr->sector_size);

//     }

//     if(cfs_port_system_flash_read_contrast( \
//         start_addr, read_sector_data, \
//         temp_cfs_objecr->sector_size * read_page_count) == false)
//     {
//         read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE;
//     }
//     else
//     {
//         read_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED;
//     }

//     return read_result;
// }

