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

#include "cycle_flash_system.h"
#include "cfs_system_middle.h"


//*********************************************************************************
//-- 对外接口  
//*********************************************************************************

#if CFS_FILESYSTEM_TYPE_VARIABLE_DATA_MODEL != 0
// 包含数据类型初始化
cfs_object_handle_ptr cfs_nv_object_init(char *name,
                                         uint32_t address,
                                         uint32_t sector_count,
                                         uint16_t data_size,
                                         cfs_object_type_t data_tpye)
#else
// 不包含数据类型初始化
cfs_object_handle_ptr cfs_nv_object_init(char *name,
                                         uint32_t address,
                                         uint32_t sector_count,
                                         uint16_t data_size)
#endif // CFS_FILESYSTEM_TYPE_VARIABLE_DATA_MODEL

{

#if CFS_FILESYSTEM_TYPE_VARIABLE_DATA_MODEL == 0
    cfs_object_type_t data_tpye = CFS_OBJECT_TYPE_FIXED_DATA_STORAGE;
#endif // CFS_FILESYSTEM_TYPE_VARIABLE_DATA_MODEL

    #ifdef CFS_DEBUG == (1u)
        //@def 判断参数有效性
        //@def 判断名称长度
        CFS_ASSERT((name!=NULL && strlen(name) < CFS_NAME_LEN_MAX), 
            "name is not exist or too long");
    #endif  // CFS_DEBUG

    //@def 在判断地址有没有重复，没通过返回false
    if (false == cfs_middle_check_address(address, sector_count))
    {
        #ifdef CFS_DEBUG == (1u)
            /*内存参数交叉了！*/
            CFS_ASSERT(cfs_middle_check_address(address, sector_count), 
                "address is not exist or too long");
        #endif  // CFS_DEBUG

        return false;
    }

    //@def 开始初始化，新建一个内存对象
    cfs_object_handle_ptr object_handle = 
        cfs_middle_add_object_init(name, address, sector_count, data_size, data_tpye);
    if(object_handle == false)
    {
        #ifdef CFS_DEBUG == (1u)
            CFS_ASSERT(object_handle, "object init fail");
        #endif  // CFS_DEBUG

        return false;
    }

    //@def 初始化对象的各种ID
    cfs_middle_object_id_init(object_handle);

    /*初始化工作结束，返回初始化的句柄*/
    return object_handle;
}


//@def 根据id往内存中写入数据
int cfs_nv_write(cfs_object_handle_ptr object,
                 uint8_t *data,
                 uint16_t len,
                 uint8_t error_retry)
{
    cfs_object_list_t *object_list = cfs_middle_find_object(object);
    if (object==NULL || data==NULL || len==0 || object_list==NULL)
    {
        return CFS_RETURN_ERROR;
    }

    int result_len = CFS_RETURN_ERROR;
    
    do{
        result_len = cfs_middle_add_data_write(object_list, data, len);
    }
    while(result_len == CFS_RETURN_ERROR && error_retry-- > 0);
    
    return result_len;
}


//@def 根据ID读取内存中的数据
int cfs_nv_read(cfs_object_handle_ptr object,
                uint8_t *data,
                uint16_t len,
                cfs_data_id_t read_in_past)
{
    cfs_object_list_t *object_list = cfs_middle_find_object(object);
    if (object==NULL || data==NULL || len==0 || object_list==NULL)
    {
        return CFS_RETURN_ERROR;
    }

    int result_len = 
        cfs_middle_data_read(object_list, read_in_past, data, len);
    
    return result_len;
}


//@def 清除指定对象的存储空间
bool cfs_nv_clear(cfs_object_handle_ptr object)
{
    cfs_object_list_t *object_list = cfs_middle_find_object(object);
    if(object_list == NULL)
    {
        return false;
    }

    cfs_system_oc_flash_data_clear(object_list);
    
    return true;
}


//@def 返回目前存储对象的ID
cfs_data_id_t cfs_nv_get_current_id(cfs_object_handle_ptr object)
{
    cfs_object_list_t *object_list = cfs_middle_find_object(object);
    if(object_list == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    return cfs_middle_get_current_id(object_list);
}


//@def 返回目前存储对象的可用ID
cfs_data_id_t cfs_nv_get_current_valid_id(cfs_object_handle_ptr object)
{
    cfs_object_list_t *object_list = cfs_middle_find_object(object);
    if(object_list == NULL)
    {
        return CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }

    return cfs_middle_get_current_valid_id(object_list);
}


