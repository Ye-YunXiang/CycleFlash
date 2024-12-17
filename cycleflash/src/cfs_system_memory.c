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
#include "cfs_system_memory.h"

#include "cfs_port_device_flash.h"
#include "cfs_system_utils.h"

// XXX:这里负责缓存管理。。。。。。。。。。。。。
#if CFS_FLASH_READ_MODE != 0
static struct
{
    cfs_read_buffer_t data_read_buffer;
}_this = {
    .data_read_buffer = {
        .buffer_ptr = NULL,
        .buffer_size = 0,
    }
};
#endif // CFS_FLASH_READ_MODE

// ************************************************************************
//@def 写入和读取数据 —— 内部处理

#if CFS_FLASH_READ_MODE == 1
static bool _read_flash_data(
    const uint32_t address, uint8_t *buffer, uint16_t read_len)
{
    return cfs_port_system_flash_read(address, buffer, read_len);
}
#endif

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
    //XXX: 请处理好内存对齐问题后在放进来。

    cfs_port_system_flash_lock_enable();
    
    while(len != 0)
    {

#if CFS_WRITE_PORT_DOUBLE_WORD != 0
        if ((addr % 8 == 0) && (len >= 8))
        {
            cfs_port_system_flash_write_double_word(addr, buffer, (len / 8));
            buffer += (len / 8) * 8;
            addr += (len / 8) * 8;
            len -= (len / 8) * 8;
            continue;
        }
#endif // CFS_WRITE_PORT_DOUBLE_WORD

#if CFS_WRITE_PORT_ONE_WORD != 0
        if ((addr % 4 == 0) && (len >= 4))
        {
            cfs_port_system_flash_write_word(addr, buffer, (len / 4));
            buffer += (len / 4) * 4;
            addr += (len / 4) * 4;
            len -= (len / 4) * 4;
            continue;
        }
#endif // CFS_WRITE_PORT_ONE_WORD

#if CFS_WRITE_PORT_HALF_WORD != 0
        if ((addr % 2 == 0) && (len >= 2))
        {
            cfs_port_system_flash_write_half_word(addr, buffer, (len / 2));
            buffer += (len / 2) * 2;
            addr += (len / 2) * 2;
            len -= (len / 2) * 2;
            continue;
        }
#endif // CFS_WRITE_PORT_HALF_WORD

#if CFS_WRITE_PORT_ONE_BYTE != 0
    #if (CFS_WRITE_PORT_DOUBLE_WORD != 0 \
        || CFS_WRITE_PORT_ONE_WORD != 0 \
        || CFS_WRITE_PORT_HALF_WORD != 0)

        cfs_port_system_flash_write_byte(addr, buffer, 1);
        buffer += 1;
        addr += 1;
        len -= 1;

    #else

        cfs_port_system_flash_write_byte(addr, buffer, len);
        break;

    #endif
#endif // CFS_WRITE_PORT_ONE_BYTE

    }

    cfs_port_system_flash_lock_disable();

    return true;
}


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

    return result;
}


static bool _contrast_flash_data_block(
    const uint32_t addr, const cfs_data_block_t *contrast_block, uint8_t *buffer)
{
    for (uint8_t i=0; i<2; i++)
    {

#if CFS_FLASH_READ_MODE == 0
        // XXX：这边是内部为大部分32位MCU可用的方法。
        if (0 != memcmp((uint8_t *)addr,
                        (uint8_t *)contrast_block,
                        CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN))
        {
            continue;
        }

        if (0 == memcmp((uint8_t *)(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
                        contrast_block->data_ptr,
                        contrast_block->data_len))
        {
            return true;
        }
#else
        _read_flash_data(
            addr, buffer, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        if (0 != memcmp((uint8_t *)addr,
                        (uint8_t *)contrast_block,
                        CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN))
        {
            // 防止每次都等待读取完毕
            continue;
        }

        _read_flash_data(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN,
                        buffer,
                        contrast_block->data_len);
        if (0 == memcmp(buffer,
                        contrast_block->data_ptr,
                        contrast_block->data_len))
        {
            return true;
        }
#endif // CFS_FLASH_READ_MODE

    }

    return false;
}


static bool _checking_flash_block_is_null_values(const uint32_t addr,
                                                 uint16_t len,
                                                 uint8_t *buffer)
{
    bool result = false;
    for (uint8_t i=0; i<2; i++)
    {
        result = true;

#if CFS_FLASH_READ_MODE == 0
        for(uint16_t i=0; i<len; i++)
        {
            // XXX：这边是内部为大部分32位MCU可用的方法。
            if(CFS_FLASH_ERASURE != ((uint8_t *)addr)[i])
            {
                // 不为空
                result = false;
                continue;
            }
        }
#else
        _read_flash_data(addr, buffer, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        for(uint16_t i=0; i<CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN; i++)
        {
            if(CFS_FLASH_ERASURE != buffer[i])
            {
                // 不为空
                result = false;
                continue;
            }
        }

        _read_flash_data(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN,
                         buffer,
                         len - CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        for(uint16_t i=0; i < (len-CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN); i++)
        {
            if(CFS_FLASH_ERASURE != buffer[i])
            {
                // 不为空
                result = false;
                continue;
            }
        }
#endif // CFS_FLASH_READ_MODE

    }

    return result;
}


static bool _erasing_page_flash_data(uint32_t addr, uint16_t page)
{
    bool result = false;
    cfs_port_system_flash_lock_enable();

    result = cfs_port_system_flash_erasing_page(addr, page);

    cfs_port_system_flash_lock_disable();

    return result;
}


// *************************************************************************
//@def 其他接口 —— 接口

// 数据块缓存区初始化
void cfs_memory_general_block_buffer_init(uint16_t data_buffer_size)
{
    #if CFS_FLASH_READ_MODE != 0
        // 判断一下不能为0
        assert(data_buffer_size != 0);

        if (_this.data_read_buffer.buffer_size > data_buffer_size)
        {
            return;
        }

        // malloc*****
        CFS_FREE(_this.data_read_buffer.buffer_ptr);
        _this.data_read_buffer.buffer_ptr = (uint8_t *)CFS_MALLOC(data_buffer_size);


        if (_this.data_read_buffer.buffer_ptr == NULL)
        {
            assert(_this.data_read_buffer.buffer_ptr);
            while(1);
        }
    #else
        
        /*A task that has not yet been executed*/

    #endif // CFS_FLASH_READ_MODE
}


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


//@def 通过可用ID计算要存入的地址位置。
/**
 * 
 */
uint32_t cfs_memory_calculate_fixed_id_flash_address(
    const cfs_object_list_t *object_list, cfs_data_id_t id_input)
{
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


//@def 根据ID计算有效数据个数
/**
 * 
 */
cfs_data_id_t cfs_memory_fixe_valid_id_number(
    const cfs_object_list_t *object_list, cfs_data_id_t id_input)
{
    uint32_t result_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;

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

//@def 在遍历ID阶段，遍历指定位置的数据，得到位置id
/**
 * 直接校验对象缓存长度的数据，对比读取出来的数据长度。
 * 返回： 如果数据有效，返回ID，否则返回0
 */
cfs_data_id_t cfs_memory_read_verify_flash_data_id(
    const cfs_object_list_t *object_list, const uint32_t address)
{
    cfs_data_block_t read_block = {0};
    uint16_t get_checkout_value = 0;

    // XXX：这边是内部为大部分32位MCU定制的读取遍历方法。
    for (uint8_t i=0; i<2; i++)
    {
        memset(&read_block, 0, sizeof(cfs_data_block_t));

#if CFS_FLASH_READ_MODE == 0
        memcpy((uint8_t *)&read_block,
               (uint8_t *)address,
               CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
#else
        _read_flash_data(address,
                         (uint8_t *)&read_block,
                         CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
#endif  // CFS_FLASH_READ_MODE

        get_checkout_value = cfs_system_utils_check(
            (uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
        
        if (read_block.data_len > object_list->object_handle->data_size
            || read_block.data_len == 0
            || read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            continue;
        }

#if CFS_FLASH_READ_MODE == 0
        get_checkout_value += cfs_system_utils_check(
            (uint8_t *)(address + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
            read_block.data_len, 
            NULL);
#else
        memset(_this.data_read_buffer.buffer_ptr, 0, read_block.data_len);
        _read_flash_data(address + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, 
                         _this.data_read_buffer.buffer_ptr, 
                         read_block.data_len);
        get_checkout_value += cfs_system_utils_check(
            _this.data_read_buffer.buffer_ptr, read_block.data_len, NULL);
#endif  // CFS_FLASH_READ_MODE
        
        if (get_checkout_value == read_block.data_check)
        {
            return read_block.data_id;
        }
    }
    
    return CFS_CONFIG_NOT_LINKED_DATA_ID;
}


//@def 读取内存中指定的内存大小，经过数据校验正确后返回给中间层解析。
/**
 * 直接校验对象缓存长度的数据，对比读取出来的数据长度。
 *
 */
int cfs_memory_read_flash_fixed_data(const cfs_object_list_t *object_list,
                                     const uint32_t address,
                                     const uint16_t data_max_len,
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

#if CFS_FLASH_READ_MODE == 0
        memcpy((uint8_t *)&read_block,
               (uint8_t *)address,
               CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
#else
        _read_flash_data(address,
                         (uint8_t *)&read_block,
                         CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
#endif  // CFS_FLASH_READ_MODE

        get_checkout_value = cfs_system_utils_check(
            (uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
        
        if (read_block.data_len > data_max_len 
            || read_block.data_len == 0
            || read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            continue;
        }

#if CFS_FLASH_READ_MODE == 0
        get_checkout_value += cfs_system_utils_check(
            (uint8_t *)(address + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
            read_block.data_len, 
            data_buffer);
#else
        _read_flash_data(address + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, 
                         data_buffer, 
                         read_block.data_len);
        get_checkout_value += 
            cfs_system_utils_check(data_buffer, read_block.data_len, NULL);
#endif  // CFS_FLASH_READ_MODE
        
        if (get_checkout_value == read_block.data_check)
        {
            return read_block.data_len;
        }
    }
    
    return CFS_RETURN_ERROR;
}


//@def 添加式写入数据，确认写入正确后返回。
/**
 * 如果写入失败，会检测以下写入区域是否无数据，如果无数据就在尝试写入一次。
 * 以上全部失败，把本区域全部置为初始值的相反值，在返回错误。
 */
int cfs_memory_add_write_flash_fixed_data(const cfs_object_list_t *object_list,
                                          const uint32_t address,
                                          const cfs_data_id_t input_id,
                                          const uint16_t data_len,
                                          uint8_t *data_buffer)
{
    if ((address + object_list->data_buffer_size) > 
        (object_list->object_handle->address 
            + CFS_FLASH_SECTOR_SIZE * object_list->object_handle->sector_count))
    {
        // 数据长度的错误情况判断
        return CFS_RETURN_ERROR;
    }
    
#if CFS_FLASH_READ_MODE == 0
    uint8_t * data_read_buffer_per = NULL;
#else
    uint8_t * data_read_buffer_per = _this.data_read_buffer.buffer_ptr;
#endif // CFS_FLASH_READ_MODE

    int write_result = CFS_RETURN_ERROR;
    cfs_data_block_t write_block = {0};
    write_block.data_ptr = data_buffer;
    write_block.data_id = input_id;
    write_block.data_len = data_len;
    write_block.data_check = cfs_system_utils_check(
        (uint8_t *)&write_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
    write_block.data_check += cfs_system_utils_check(data_buffer, data_len, NULL);

    // 判断是否要擦除页-----------------------
    if(address == object_list->object_handle->address
        || address % CFS_FLASH_SECTOR_SIZE == 0) 
    {
        // 情况一：循环的第一个数据。
        // 情况二：数据小于一页，但是正好在新的一页的第一个。
        _erasing_page_flash_data(address, (
                (address + object_list->data_buffer_size) - address
            ) / CFS_FLASH_SECTOR_SIZE + 1);
    }
    else if(address / CFS_FLASH_SECTOR_SIZE
        < (address + object_list->data_buffer_size) / CFS_FLASH_SECTOR_SIZE)
    {
        // 情况三：存入的数据长度跨页了,这里要确定有出现跨页了才能进来
        const uint32_t CLEAR_START_ADDR = 
            ((address / CFS_FLASH_SECTOR_SIZE) + 1) * CFS_FLASH_SECTOR_SIZE;
        _erasing_page_flash_data(CLEAR_START_ADDR, (
                ((address + object_list->data_buffer_size) - CLEAR_START_ADDR) 
                / CFS_FLASH_SECTOR_SIZE
            ) + 1);
    }

    // 开始写入数据---------------------------
    for (uint8_t i=0; i<2; i++)
    {
        _write_flash_data_block(address, &write_block);

        if(true == _contrast_flash_data_block(
            address, &write_block, data_read_buffer_per))
        {
            write_result = data_len;
            break;
        }
        else if (false == _checking_flash_block_is_null_values(
                address, 
                data_len + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, 
                data_read_buffer_per))
        {
            write_result = CFS_RETURN_ERROR;
            break;
        }
    }

    // 错误处理，方便后期上电遍历----------------------
    if (write_result == CFS_RETURN_ERROR)
    {
        memset(&write_block,
               ~CFS_FLASH_ERASURE,
               CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        _write_flash_data(address,
                          (uint8_t *)&write_block,
                          CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    }

    return write_result;
}


bool cfs_memory_flash_data_clear(const cfs_object_list_t *object_list)
{
    _erasing_flash_page(object_list->object_handle->address,
                        object_list->object_handle->sector_count);
    return true;
}
