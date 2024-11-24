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

// 分配失败打算直接死掉在断言里面
#define APPLY_MEMORY_FAIL_DISPOSE(x)  if(x == NULL){assert(x);while(1);}
// 判断字符串的长度，加上\0，这里需要字符串指针
#define STRING_ALL_SIZE(x)  (strlen((char *)x) + (1u))


// XXX:这里负责对象管理。。。。。。。。。。。。。
static struct
{
    
#ifdef CFS_FLASH_SECTOR_BUFFER_DEF
    uint8_t flash_one_sector_buffer[CFS_FLASH_SECTOR_SIZE];
#endif // CFS_FLASH_SECTOR_BUFFER_DEF

    cfs_object_list_t *object_list_head;
    cfs_block_buffer_t data_block_buffer;
}_this = {

#ifdef CFS_FLASH_SECTOR_BUFFER_DEF
    .flash_one_sector_buffer = {0},
#endif // CFS_FLASH_SECTOR_BUFFER_DEF

    .object_list_head = NULL,
    .data_block_buffer = {
        .buffer_ptr = NULL,
        .buffer_size = 0,
        .use_flag = false
    }
};


//*******************************************************************************************
//-- 内部管理接口
//*******************************************************************************************
// 计算需要填充的字节数
static uint8_t _compute_memory_fill_length(uint16_t data_size)
{
    // 判断一下不能为0
    assert(data_size != 0);

    uint8_t data_fill_len = 
        (data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN) 
        % CFS_WRITE_MIN_PARTICLE;

    if (data_fill_len == 0)
    {
        return 0;
    }
    else
    {
        return (CFS_WRITE_MIN_PARTICLE - data_fill_len);
    }
}

// 数据块缓存区初始化
static void _general_block_buffer_init(uint16_t data_buffer_size)
{
    // 判断一下不能为0
    assert(data_buffer_size != 0);

    if (_this.data_block_buffer.buffer_size > data_buffer_size)
    {
        return;
    }

    // malloc*****
    CFS_FREE(_this.data_block_buffer.buffer_ptr);
    _this.data_block_buffer.buffer_ptr = (uint8_t *)CFS_MALLOC(data_buffer_size);
    APPLY_MEMORY_FAIL_DISPOSE(_this.data_block_buffer.buffer_ptr);
}

//@def 紧密存储遍历内存ID初始化
// TODO：正在构思，里面的内容仅限借鉴
static uint32_t cfs_filesystem_tight_data_page_id_init( \
    cfs_object_list_t *temp_linked_object)
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
            read_result = cfs_system_oc_read_flash_data(
                temp_linked_object, &data_block);
            
            //@def 读错就在往前读一数据块，读空直接退出
            if(read_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED &&
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
            if(data_max_addr > (cfs_system_oc_via_id_calculate_addr(
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





//*******************************************************************************************
//-- 对上层接口  
//*******************************************************************************************

//@def 初始化数据对象
cfs_object_t * cfs_middle_add_object_init(
    const uint8_t *name, 
    const uint32_t address,
    const cfs_data_id_t sector_count, 
    const uint16_t data_size)
{
    // name malloc******
    uint8_t *name_ptr = (uint8_t *)CFS_MALLOC(STRING_ALL_SIZE(name));
    APPLY_MEMORY_FAIL_DISPOSE(name_ptr);
    memcpy(name_ptr, name, STRING_ALL_SIZE(name));

    // cfs_object_t malloc******
    cfs_object_t *cfs_object = (cfs_object_t *)CFS_MALLOC(sizeof(cfs_object_t)); 
    APPLY_MEMORY_FAIL_DISPOSE(cfs_object);
    *(uint8_t *)&cfs_object->name = name_ptr;
    *(uint32_t *)&cfs_object->address = address;
    *(uint32_t *)&cfs_object->sector_count = sector_count;
    *(uint16_t *)&cfs_object->data_size = data_size;
    *(uint8_t *)&cfs_object->data_fill = _compute_memory_fill_length(data_size);

    // cfs_object_list_t malloc*****
    cfs_object_list_t *cfs_list = 
        (cfs_object_list_t *)CFS_MALLOC(sizeof(cfs_object_list_t));
    APPLY_MEMORY_FAIL_DISPOSE(cfs_list);

    // 添加入list中
    cfs_list->next = _this.object_list_head;
    _this.object_list_head = cfs_list;

    cfs_list->name = cfs_object->name;
    cfs_list->object_handle = cfs_object;
    cfs_list->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    cfs_list->valid_id_number = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    cfs_list->data_buffer_size = CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN
        + cfs_object->data_size + cfs_object->data_fill;

    // 重新分配数据块的缓存区
    _general_block_buffer_init(cfs_list->data_buffer_size);

    return cfs_object;
}

//@def 初始化遍历ID
bool cfs_middle_object_id_init(const cfs_object_t *object)
{
    cfs_object_list_t *list_object_ptr = cfs_middle_find_object(object);
    assert(list_object_ptr!=NULL && object!=NULL);

    cfs_data_id_t data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    uint16_t vakud_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;

    /** 
     * 这里判断第一页存储区的前8个字节的状态：
     * 8个字节一半，只要有4byte符合存储区状态，就算通过。
     * value: 0x01010101       初始化过存储区。
     * value: 0x0A0A0A0A       变长数据存储区。TODO: 还未实现
     * value: 0x0F0F0F0F       开始使用内存。
     */
    // 先读取，判断内存状态
    cfs_object_type_t flash_typ = cfs_filesystem_tight_data_page_id_init(object);

    // 根据不同的返回执行对应的操作
    switch (flash_typ)
    {
        case CFS_OBJECT_TYPE_FIXED_DATA_STORAGE:
            /* code */
            break;

        case CFS_OBJECT_TYPE_VARIABLE_DATA_LENGTH:
            /* code */
            break;
        
        case CFS_OBJECT_TYPE_INIT:
            /* code */
            break;

        default:
            break;
    }



	//@def 设置遍历好的ID值
    cfs_system_oc_object_id_set(object, data_id);

    //@def 判断有没有ID
    if(data_id != CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        data_id = cfs_system_oc_valid_data_number(object);
    }
    else
    {
        data_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }
    //@def 设置目前可用的ID数量
    cfs_system_oc_object_valid_id_number_set(object, data_id);

    return true;
}

//@def 查找对象对象
cfs_object_list_t *cfs_middle_find_object(const cfs_object_t *object)
{
    assert(object != NULL);
    assert(object->name != NULL);
    
    if (_this.object_list_head == NULL)
    {
        return NULL;
    }
  
    cfs_object_list_t *find_object = _this.object_list_head;
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
bool cfs_middle_check_address(const uint32_t address, const uint32_t sector_count)
{
    if (_this.object_list_head == NULL)
    {
        return false;
    }

    cfs_object_list_t *list_pointer = _this.object_list_head->next;
    uint32_t current_head = address;
    uint32_t current_tail = address + (sector_count*CFS_FLASH_SECTOR_SIZE) - 1;
    
    // 遍历内存并检查和之前的数据对象是否有交叉
    while (list_pointer != NULL) 
    {
        uint32_t next_head = list_pointer->object_handle->address;
        uint32_t next_tail = list_pointer->object_handle->address +
            (CFS_FLASH_SECTOR_SIZE*list_pointer->object_handle->sector_count) - 1;

        if (((current_head<=next_tail) && (current_tail>=next_head)) 
            || ((next_head<=current_tail) && (next_tail>=current_head)) 
            || ((current_head<=next_head) && (current_tail>=next_tail)) 
            || ((next_head<=current_head) && (next_tail>=current_tail)))
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
    cfs_object_list_t *object_list, cfs_data_id_t read_id, uint8_t *data, uint16_t len)
{
    // TODO
}

//@def 写入数据,写入成功返回写入的原始数据长度
uint32_t cfs_middle_data_write(
    cfs_object_list_t *object_list, uint32_t write_id, uint8_t *data, uint16_t len)
{
// TODO
    
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

