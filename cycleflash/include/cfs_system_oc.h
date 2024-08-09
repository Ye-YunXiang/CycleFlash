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

#ifndef __CFS_SYSTEM_OC_H__
#define __CFS_SYSTEM_OC_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cfs_system_define.h"

typedef enum
{
    // cfs操作的数据为空
    CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL       = 0,
    // cfs数据页非空
    CFS_OC_READ_OR_WRITE_DATA_RESULT_NONEMPTY   = 1,
    // cfs写入或读取数据错误
    CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE      = 2,
    // cfs写入或读取数据有效
    CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED    = 3,
}cfs_oc_action_data_result;

// 检查flash制定地址的block是否有数据
// 如果为空值返回true
bool cfs_system_oc_flash_checking_null_values( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer);

// 根据ID计算有效数据个数
uint32_t cfs_system_oc_valid_data_number(const cfs_object_linked_list *temp_linked_object);

/*遍历数据页，初始化ID值*/
uint32_t cfs_system_oc_traverse_data_page_id_init(uint32_t temp_cfs_handle);

/*遍历目录页，初始化ID值*/
uint32_t cfs_system_oc_traverse_list_page_id_init(uint32_t temp_cfs_handle);

// 链表添加一个数据对象
cfs_object_linked_list *cfs_system_oc_add_object(\
    cfs_system *object_pointer, const uint8_t * const name);

// 检查设置的文件的地址和将要存入数据的地址片区有没有重复
bool cfs_system_oc_flash_repeat_address(const cfs_system *temp_object);


// 根据ID得到本ID对应的内存地址
uint32_t cfs_system_oc_via_id_calculate_addr( \
    const cfs_object_linked_list *temp_object, uint32_t temp_id);

// *****************************************************************************************************
// 写入和读取数据 —— 接口

// 读取内存中的数据
cfs_oc_action_data_result cfs_system_oc_read_flash_data( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer);

// 往内存中写入新的数据，增加式
cfs_oc_action_data_result cfs_system_oc_add_write_flash_data( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer);

// 修改内存中的数据
cfs_oc_action_data_result cfs_system_oc_set_write_flash_data( \
    const cfs_object_linked_list *temp_object, cfs_data_block * buffer);

// 清楚对象在内存中的数据
bool cfs_system_oc_flash_data_clear(const cfs_object_linked_list *temp_object);

// 删除内核对象
bool cfs_system_oc_object_delete(cfs_object_linked_list *temp_object);

// *****************************************************************************************************
// 设置和获取对象接口

/*设置数据数据对象的ID*/
bool cfs_system_oc_object_id_set( \
    cfs_object_linked_list *temp_cfs_handle, uint32_t temp_id);

// 得到数据数据对象的ID
uint32_t cfs_system_oc_object_id_get(const cfs_object_linked_list *temp_cfs_handle);

/*设置数据数据对象的可用ID*/
bool cfs_system_oc_object_valid_id_number_set( \
    cfs_object_linked_list * temp_cfs_handle, uint16_t temp_id);

// 得到数据数据对象的可用ID
uint16_t cfs_system_oc_object_valid_id_numbe_get( \
    const cfs_object_linked_list *temp_cfs_handle);

// 得到数据对象的类型
uint8_t cfs_system_oc_object_struct_type_get( \
    const cfs_object_linked_list *temp_cfs_handle);

// 得到内部系统数据对象指针
cfs_system *cfs_system_oc_system_object_get(const cfs_object_linked_list *temp_object);

/*使用初始化链表对象后返回的句柄，在通过crc-16-xmodem标识验证链表对象是否存在*/
// 存在返回链表对象，不存在返回NULL
cfs_object_linked_list *cfs_system_oc_object_linked_crc_16_verify( \
    cfs_system_handle_t temp_cfs_handle);



#endif /* __CFS_SYSTEM_OC_H__ */
