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

#ifndef __CYCLE_MIDDLE_H__
#define __CYCLE_MIDDLE_H__

#include "cfs_system_define.h"

//@def 初始化数据对象
cfs_object_t * cfs_middle_add_object_init(
    const uint8_t *name, 
    const uint32_t address,
    const uint16_t sector_count, 
    const uint16_t data_size);

//@def 初始化遍历ID
bool cfs_middle_object_id_init(const cfs_object_t *object);

//@def 初始化遍历ID
bool cfs_middle_object_id_init(const cfs_object_t *object);

//@def 查找对象对象
cfs_object_list_t *cfs_middle_find_object(const cfs_object_t *object);

//@def 检查重复地址是否通过
bool cfs_middle_check_address(const uint32_t address, const uint32_t sector_count);

//@def 读取数据,读取成功返回读取的原始数据长度
int cfs_middle_data_read(
    cfs_object_list_t *object_list, cfs_data_id_t read_id, uint8_t *data, uint16_t len);

//@def 写入数据,写入成功返回写入的原始数据长度
uint32_t cfs_middle_data_write(
    cfs_object_list_t *object_list, cfs_data_id_t write_id, uint8_t *data, uint16_t len);

//@def 清除本对象数据
bool cfs_system_oc_flash_data_clear(cfs_object_list_t *object_list);

//@def 返回目前存储对象的ID
uint32_t cfs_middle_get_current_id(const cfs_object_list_t *object_list);

//@def 返回目前存储对象的可用ID
uint32_t cfs_middle_get_current_valid_id(const cfs_object_list_t *object_list);

#endif
