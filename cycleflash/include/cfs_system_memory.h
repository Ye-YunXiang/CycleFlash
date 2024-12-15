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

#ifndef __CFS_SYSTEM_MEMORY_H__
#define __CFS_SYSTEM_MEMORY_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cfs_system_define.h"

typedef enum
{
    //@def cfs操作的数据为空
    CFS_OC_READ_OR_WRITE_DATA_RESULT_NULL       = 0,
    //@def cfs数据页非空
    CFS_OC_READ_OR_WRITE_DATA_RESULT_NONEMPTY   = 1,
    //@def cfs写入或读取数据错误
    CFS_OC_READ_OR_WRITE_DATA_RESULT_ERROE      = 2,
    //@def cfs写入或读取数据有效
    CFS_OC_READ_OR_WRITE_DATA_RESULT_SUCCEED    = 3,
    //@def cfs读取数据的内存区为无效块
    CFS_OC_READ_DATA_RESULT_INVALID_BLOCK       = 4,
}cfs_oc_action_data_result;


// HACK: 新
// 计算需要填充的字节数
uint8_t cfs_memory_compute_memory_fill_length(uint16_t data_size);

//@def 通过可用ID计算要存入的地址位置。
/**
 * 
 */
uint32_t cfs_memory_calculate_fixed_id_flash_address(
    const cfs_object_list_t *object_list, cfs_data_id_t id_input);


//@def 根据ID计算有效数据个数
cfs_data_id_t cfs_memory_fixe_valid_id_number(
    const cfs_object_list_t *object_list, cfs_data_id_t id_input);


// *****************************************************************************************************
//@def 写入和读取数据 —— 接口


// HACK: 新
//@def 在遍历ID阶段，遍历指定位置的数据，得到位置id
/**
 * 直接校验对象缓存长度的数据，对比读取出来的数据长度。
 * 返回： 如果数据有效，返回数据长度，否则返回0
 */
// TODO: 这里后面需要兼容自定义读取函数，后面在做
uint16_t cfs_memory_read_verify_flash_data_id(const cfs_object_list_t *object_list,
                                              const uint32_t address,
          
                                              cfs_data_id_t get_id);

//@def 读取内存中指定的内存大小，经过数据校验正确后返回给中间层解析。
/**
 * 直接校验对象缓存长度的数据，对比读取出来的数据长度。
 * 
 * 返回读取的长度
 */
int cfs_memory_read_flash_fixed_data(const cfs_object_list_t *object_list,
                                     const uint32_t address,
                                     const data_max_len,
                                     uint8_t *data_buffer);


// HACK: 新
bool cfs_memory_flash_data_clear(const cfs_object_list_t *object_list);


#endif /* __CFS_SYSTEM_MEMORY_H__ */
