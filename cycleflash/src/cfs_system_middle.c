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

// 计算需要填充的字节数
#define COMPUTE_MEMORY_FILL_LENGTH(x)    ((x + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN) % CFS_WRITE_MIN_PARTICLE)

// 内存分配三部曲，分配失败打算直接死掉在断言里面
#define APPLY_MEMORY_FAIL_DISPOSE(x)  if(x == NULL){asster(x);while(1);}

// 判断字符串的长度，加上\0，这里需要字符串指针
#define STRING_ALL_SIZE(x)  (strlen(x) + 1)


// XXX:这里负责对象管理。。。。。。。。。。。。。

static cfs_object_list_t *object_list_head = NULL;

//*******************************************************************************************
//-- 内部管理接口
//*******************************************************************************************
// 计算需要填充的字节数
static uint8_t _compute_memory_fill_length(cfs_data_size_t data_size)
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

//*******************************************************************************************
//-- 对上层接口  
//*******************************************************************************************

//@def 初始化数据对象
cfs_object_t * cfs_middle_add_object_init(
    const uint8_t *name, 
    const uint32_t address,
    const uint16_t sector_count, 
    const uint16_t data_size,
    const enum cycle_object_type type)
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
    *(uint16_t *)&cfs_object->sector_count = sector_count;
    *(cfs_data_size_t *)&cfs_object->data_size = COMPUTE_MEMORY_LENGTH(data_size);
    *(cycle_object_type_t *)&cfs_object->type = type;

    // cfs_object_list_t malloc*****
    cfs_object_list_t *cfs_list = 
        (cfs_object_list_t *)CFS_MALLOC(sizeof(cfs_object_list_t));
    APPLY_MEMORY_FAIL_DISPOSE(cfs_list);

    // buffer malloc******
    cfs_list->buffer = (uint8_t *)CFS_MALLOC(
        cfs_object->data_size + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN); 
    APPLY_MEMORY_FAIL_DISPOSE(cfs_list->buffer);

    // 添加入list中
    cfs_list->next = object_list_head;
    object_list_head = cfs_list;

    cfs_list->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    cfs_list->valid_id_number = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    cfs_list->name = cfs_object->name;
    cfs_list->object_handle = cfs_object;

    return cfs_list;
}

//@def 查找对象对象
cfs_object_list_t cfs_middle_find_object(const cfs_object_t *object)
{
    assert(object != NULL);
    assert(object->name != NULL);
    assert(object_list_head != NULL);
  
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
bool cfs_middle_check_address(const uint32_t address, const uint16_t sector_count)
{
    cfs_object_list_t *list_pointer = cfs_system_object_head->next;
    uint32_t current_head = address;
    uint32_t current_tail = address + (sector_count*CFS_FLASH_SECTOR_SIZE) - 1;
    
    // 遍历内存并检查和之前的数据对象是否有交叉
    while(list_pointer != NULL) 
    {
        uint32_t next_head = list_pointer->object_handle->addr_handle;
        uint32_t next_tail = list_pointer->object_handle->addr_handle + \
            (CFS_FLASH_SECTOR_SIZE * list_pointer->object_handle->sector_count) - 1;

        if(((current_head<=next_tail) && (current_tail>=next_head)) 
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
    cfs_object_list_t *object_list, uint32_t read_id, uint8_t *data, uint16_t len)
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

