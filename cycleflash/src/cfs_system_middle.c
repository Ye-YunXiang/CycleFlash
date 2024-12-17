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

#include "cfs_system_middle.h"
#include "cfs_system_memory.h"


// 分配失败打算直接死掉在断言里面
#define APPLY_MEMORY_FAIL_DISPOSE(x)  if(x == NULL){assert(x);while(1);}
// 判断字符串的长度，加上\0，这里需要字符串指针
#define STRING_ALL_SIZE(x)  (strlen((char *)x) + (1u))


// XXX:这里负责对象管理。。。。。。。。。。。。。
static struct
{
    cfs_object_list_t *object_list_head;
}_this = {
    .object_list_head = NULL
};


//************************************************************************************
//-- 内部管理接口
//************************************************************************************


// 三层处理接口 ----------------------------------------------------------------------
// 获取内存中的ID，专用函数，用于初始化遍历ID时。
// 参数 read_id[0]:要读取的ID； read_id[1]：读取到的ID。
static cfs_oc_action_data_result _read_fixed_flash_id(
    const cfs_object_list_t *object_list, cfs_data_id_t read_id[2])
{
    // XXX: 这个暂时是用于读取定长设计的
    uint32_t address = 
        cfs_memory_calculate_fixed_id_flash_address(object_list, read_id[0]);

    read_id[1] = 
        cfs_memory_read_verify_flash_data_id(object_list, address);

    if (read_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        return CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED;
    }
    else
    {
        read_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;

        return CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE;
    }
}


// 二层处理接口 -------------------------------------------------------------------
//@def 固定长度存储遍历内存ID初始化
static cfs_data_id_t 
    _fixed_data_storage_id_search(const cfs_object_list_t *object_list)
{
    // 计算本存储区能存储的ID总数，这里指的是能存几个
    // ID是从0开始的，所以得小于它。
    const cfs_data_id_t FLASH_MAX_ID_COUNT = 
        ((object_list->object_handle->sector_count * CFS_FLASH_SECTOR_SIZE)
        / object_list->data_buffer_size) - 1;
    // 创建数组，数组结构 { 遍历时的检索ID，遍历到的最大ID }
    cfs_data_id_t data_max_id[2] = {NULL, CFS_CONFIG_NOT_LINKED_DATA_ID};
    cfs_data_id_t data_traversal_id[2] = {NULL, CFS_CONFIG_NOT_LINKED_DATA_ID};

    // 一临时的处理变量
    cfs_data_id_t data_compute_id = 0;
    cfs_oc_action_data_result read_id_result = CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL;
    
    //@def 遍历每一页第一位的值，考虑到如果第一位数据存储错误的情况，往后累加3位
    // 
    for (uint32_t i=0; i<object_list->object_handle->sector_count; i++)
    {
        data_compute_id = ((i*CFS_FLASH_SECTOR_SIZE) / object_list->data_buffer_size);
        if (data_compute_id > data_traversal_id[0] || i == 0)
        {
            data_traversal_id[0] = data_compute_id;
        }
        else
        {
            continue; // ID重复跳过本循环
        }

        // 遍历区，如果错误，在往下遍历一位
        for (uint8_t j=0; j<3; j++)
        {
            if (data_traversal_id[0] > FLASH_MAX_ID_COUNT)
            {
                // 这里表示ID号超过了弹出结束
                break;
            }

            read_id_result = _read_fixed_flash_id(object_list, data_traversal_id);
            if (read_id_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED)
            {
                // 解码成功
                break;
            }
            else
            {
                data_traversal_id[0] += 1;
                data_traversal_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;
            }
        }

        if ((data_max_id[1]<data_traversal_id[1]
                || (data_max_id[1]==CFS_CONFIG_NOT_LINKED_DATA_ID 
                    && data_traversal_id[1]!=CFS_CONFIG_NOT_LINKED_DATA_ID))
            && data_traversal_id[1]!=CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            data_max_id[0] = data_traversal_id[0];
            data_max_id[1] = data_traversal_id[1];
        }

        data_traversal_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    //@def 如果读出来的结果是有ID的，遍历ID最大的这一页，寻找ID的最大值
    if (data_max_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        const cfs_data_id_t LASR_TRAVERSE_MAX_ID =  
            (CFS_FLASH_SECTOR_SIZE / object_list->data_buffer_size) 
            + data_max_id[0] + 1;
        
        data_traversal_id[0] = data_max_id[0] + 1;

        while (data_traversal_id[0] < FLASH_MAX_ID_COUNT
            && data_traversal_id[0] < LASR_TRAVERSE_MAX_ID)
        {
            read_id_result = _read_fixed_flash_id(object_list, data_traversal_id);
            if (read_id_result == CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED
                && data_traversal_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID)
            {
                if (data_traversal_id[1] >= data_max_id[1])
                {
                    data_max_id[1] = data_traversal_id[1];
                }
                else
                {
                    break;
                }
            }
            data_traversal_id[0] += 1;
            data_traversal_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;
        }
    }

    return  data_max_id[1];
}


//*******************************************************************************
//-- 对上层接口  
//*******************************************************************************

// 上层工具层 ------------------------------------------------------------------

//@def 检查重复地址， 通过返回 true， 不通过返回 false
bool cfs_middle_check_address(const uint32_t address, const uint32_t sector_count)
{
    if (_this.object_list_head == NULL)
    {
        return true;
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

// 上层接口层 ------------------------------------------------------------------

//@def 初始化数据对象
cfs_object_t *cfs_middle_add_object_init(char *name,
                                         uint32_t address,
                                         uint16_t sector_count,
                                         uint16_t data_size,
                                         cfs_object_type_t data_tpye)
{
    // name malloc******
    char *name_ptr = (char *)CFS_MALLOC(STRING_ALL_SIZE(name));
    APPLY_MEMORY_FAIL_DISPOSE(name_ptr);
    memcpy(name_ptr, name, STRING_ALL_SIZE(name));

    // cfs_object_t malloc******
    cfs_object_t *cfs_object = (cfs_object_t *)CFS_MALLOC(sizeof(cfs_object_t)); 
    APPLY_MEMORY_FAIL_DISPOSE(cfs_object);
    cfs_object->name = name_ptr;
    *(uint32_t *)&cfs_object->address = address;
    *(uint32_t *)&cfs_object->sector_count = sector_count;
    *(uint16_t *)&cfs_object->data_size = data_size;
    *(cfs_object_type_t *)&cfs_object->data_type = data_tpye;

    // cfs_object_list_t malloc*****
    cfs_object_list_t *cfs_list = 
        (cfs_object_list_t *)CFS_MALLOC(sizeof(cfs_object_list_t));
    APPLY_MEMORY_FAIL_DISPOSE(cfs_list);

    // 添加入list中
    cfs_list->next = _this.object_list_head;
    _this.object_list_head = cfs_list;

    cfs_list->name = name_ptr;
    cfs_list->object_handle = cfs_object;
    cfs_list->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    cfs_list->valid_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    cfs_list->data_buffer_size = CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN
        + cfs_object->data_size + cfs_memory_compute_memory_fill_length(data_size);

    // 判断数据大小不能大于存储区
    assert(cfs_list->data_buffer_size 
        < (cfs_object->sector_count*CFS_FLASH_SECTOR_SIZE));

    // 重新分配数据块的缓存区
    cfs_memory_general_block_buffer_init(cfs_list->data_buffer_size);

    return cfs_object;
}

//@def 初始化遍历ID
bool cfs_middle_object_id_init(const cfs_object_t *object)
{
    cfs_object_list_t *list_object_ptr = cfs_middle_find_object(object);
    assert(list_object_ptr!=NULL && object!=NULL);

    cfs_data_id_t data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    cfs_data_id_t vakud_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;

    if (object->data_type == CFS_OBJECT_TYPE_FIXED_DATA_STORAGE)
    {
        // 这里为定长数据的遍历
        data_id = _fixed_data_storage_id_search(list_object_ptr);
        // 检索可用ID
        vakud_id = cfs_memory_fixe_valid_id_number(list_object_ptr, data_id);
    }
    else
    {
        // 这里为变长数据的定义
        /* TODO: 还未实现   code */
    }

	//@def 设置遍历好的ID值
    list_object_ptr->data_id = data_id;
    list_object_ptr->valid_id = vakud_id;

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

//@def 读取数据,读取成功返回读取的原始数据长度
// 如果有错误数据，比如内存数据长度大于传入缓存长度，或者ID不匹配，就返回错误。
int cfs_middle_data_read(cfs_object_list_t *object_list,
                         cfs_data_id_t read_in_past,
                         uint8_t *data,
                         uint16_t len)
{
    // XXX: 外层要把参数处理干净在传入进来。
    if (object_list->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID
        || read_in_past > object_list->valid_id)
    {
        return CFS_RETURN_ERROR;
    }

    uint32_t address = 
        cfs_memory_calculate_fixed_id_flash_address(
            object_list, object_list->data_id - read_in_past);
    
    int result = 
        cfs_memory_read_flash_fixed_data(object_list, address, len, data);

    if (result == CFS_RETURN_ERROR)
    {
        // 清空给的数据区
        memset(data, 0, len);
    }

    return result;
}


//@def 写入数据
int cfs_middle_add_data_write(cfs_object_list_t *object_list,
                              uint8_t *data,
                              uint16_t len)
{
    // XXX: 外层要把参数处理干净在传入进来。
    if (len > object_list->object_handle->data_size)
    {
        return CFS_RETURN_ERROR;
    }

    int result = CFS_RETURN_ERROR;

    if (object_list->data_id < CFS_CONFIG_DATA_ID_UPPER_LIMIT)
    {
        object_list->data_id++;
    }
    else if (object_list->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
    {
        object_list->data_id = 0;
    }
    else if (object_list->data_id > CFS_CONFIG_DATA_ID_UPPER_LIMIT)
    {
        cfs_memory_flash_data_clear(object_list);
        object_list->data_id = 0;
        object_list->valid_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }

    // TODO: 这里后续需要区分是变长还是定长 
    if (object_list->object_handle->data_type == CFS_OBJECT_TYPE_FIXED_DATA_STORAGE)
    {
        uint32_t address = 
            cfs_memory_calculate_fixed_id_flash_address(
                object_list, object_list->data_id);
        
        result = cfs_memory_add_write_flash_fixed_data(
            object_list, address, object_list->data_id,len, data);

        object_list->valid_id = 
            cfs_memory_fixe_valid_id_number(object_list, object_list->data_id);
    }
    else if (object_list->object_handle->data_type 
            == CFS_OBJECT_TYPE_VARIABLE_DATA_STORAGE)
    {
        // 这里是变长的数据
    }

    return result;
}


//@def 清除本对象数据
bool cfs_system_oc_flash_data_clear(cfs_object_list_t *object_list)
{
    object_list->data_id = CFS_CONFIG_NOT_LINKED_DATA_ID;
    object_list->valid_id = CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    cfs_memory_flash_data_clear(object_list);

    return true;
}

//@def 返回目前存储对象的ID
uint32_t cfs_middle_get_current_id(const cfs_object_list_t *object_list)
{
    if (object_list == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    return object_list->data_id;
}

//@def 返回目前存储对象的可用ID
uint32_t cfs_middle_get_current_valid_id(const cfs_object_list_t *object_list)
{
    if (object_list == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }

    return object_list->valid_id;
}

