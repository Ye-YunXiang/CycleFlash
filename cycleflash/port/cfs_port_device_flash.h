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

#ifndef __CFS_PORT_DEVICE_FLASH_H__
#define __CFS_PORT_DEVICE_FLASH_H__

#include <stdbool.h>
#include <stdint.h>

/** 初始化本地设备的flash外设
 * 
 * 如果需要就根据应用的设备进行初始化，不需要可以无视。
 *
 * \param 无
 * \return 初始化返回true，否则false
 */
bool cfs_port_system_flash_init(void);


/** 往flash中存储数据
 * 
 * 要能实现指定长度存入内存。
 * 
 * \param uint32_t flash地址
 * \param uint8_t  数据指针 
 * \return 成功返回true，否则false
 */
bool cfs_port_system_flash_write_data( \
    const uint32_t addr, const uint8_t * data, const uint16_t len);


/** 往flash中读取数据
 * 
 * 要能实现指定长度读取数据。
 * 
 * \param uint32_t flash地址
 * \param uint8_t  装载数据的缓存指针 
 * \return 成功返回true，否则false
 */
bool cfs_port_system_flash_read_data( \
    const uint32_t addr, uint8_t * buffer, const uint16_t len);


/** 擦除指定页面的数据
 * 
 * \param uint32_t flash地址
 * \param uint8_t  擦除几页 
 * \return 成功返回true，否则false
 */
bool cfs_port_system_flash_erasing_page(const uint32_t addr, const uint16_t len);


#endif /* __CFS_PORT_DEVICE_FLASH_H__ */
