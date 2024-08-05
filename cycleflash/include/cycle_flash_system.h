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

#ifndef __CYCLE_FLASH_SYSTEM_H__
#define __CYCLE_FLASH_SYSTEM_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "cfs_system_define.h"


/** 初始化文件管理系统对象
 *
 * \param cfs_nv_system 初始化结构体指针
 * \return cfs_system_handle_t 初始化后的句柄，如果初始化失败返回 false。
 */
cfs_system_handle_t cfs_init( \
	cfs_system *temp_object, const char * const name);


/** 根据id往内存中写入数据
 *
 * \param cfs_system_handle_t 存储空间的句柄
 * \param uint32_t 要读取的ID
 * \param uint8_t 写入数据的指针
 * \param uint32_t 写入数据长度
 * \return 写入成功返回写入的数据个数，如果写入失败返回 0。
 */
uint32_t cfs_nv_write( cfs_system_handle_t temp_object, \
	uint32_t temp_id, const uint8_t *data, uint32_t len);


/** 根据ID读取内存中的数据
 *
 * \param cfs_system_handle_t 存储空间的句柄
 * \param uint32_t 要读取的ID
 * \param uint8_t 装载数据的指针
 * \param uint32_t 读取数据长度
 * \param uint16_t 读取过去的数据，用现在的id减去它
 * \return 读取数据成功后返回读取数据的个数，如果读取失败返回 0。
 */
uint32_t cfs_nv_read(cfs_system_handle_t temp_object, \
	uint32_t temp_id, uint8_t *data, uint32_t len, uint16_t past_id);


/** 清除指定对象的存储空间
 *
 * \param cfs_system_handle_t 存储空间的句柄
 * \return 擦除成功返回true，反则为flash
 */
bool cfs_nv_clear(cfs_system_handle_t temp_object);


/** 返回目前存储对象的ID
 *
 * \param cfs_system_handle_t 存储空间的句柄
 * \return 获取当前ID成功就返回ID，反则为flash
 */
uint32_t cfs_nv_get_current_id(cfs_system_handle_t temp_object)

#endif /* __CYCLE_FLASH_SYSTEM_H__ */
