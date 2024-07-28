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

// 遍历数据页，初始化ID值
uint32_t cfs_system_oc_traverse_data_page_id_init(uint32_t temp_cfs_handle);

// 遍历目录页，初始化ID值
uint32_t cfs_system_oc_traverse_list_page_id_init(uint32_t temp_cfs_handle);

// 读取内存中的数据
bool cfs_system_oc_read_flash_data(const uint32_t addr, cfs_data_block * buffer);

// 得到内部系统数据对象指针
cfs_system *cfs_system_oc_system_object_get(const uint32_t temp_object);

/*遍历数据页，初始化ID值*/
uint32_t cfs_system_oc_traverse_data_page_id_init(uint32_t temp_cfs_handle);

/*遍历目录页，初始化ID值*/
uint32_t cfs_system_oc_traverse_list_page_id_init(uint32_t temp_cfs_handle);

/*设置数据数据对象的ID*/
bool cfs_system_oc_object_id_set(uint32_t temp_cfs_handle, uint32_t temp_id);

// 得到数据数据对象的ID
uint32_t cfs_system_oc_object_id_get(uint32_t temp_cfs_handle);

/*铜鼓crc-8标识验证链表对象是否存在*/
bool cfs_system_oc_object_linked_crc_8_verify(\
    const uint32_t temp_object, const uint8_t temp_crc_8);

cfs_object_linked_list *cfs_system_oc_add_object(\
    cfs_system *object_pointer, const char * const name);

bool cfs_system_oc_flash_repeat_address(const cfs_system *temp_object);

uint8_t cfs_system_oc_cfs_system_crc_8_check(const uint8_t *data, uint32_t data_length);


#endif /* __CFS_SYSTEM_OC_H__ */
