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

#ifndef __CFS_PORT_DEVICE_FLASH_H__
#define __CFS_PORT_DEVICE_FLASH_H__

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

/** 往flash中存储数据(字节写)
 * 
 * 要能实现指定长度存入内存(1字节速度存入)
 * 
 * \param uint32_t flash地址
 * \param uint8_t  数据指针 
 * \param uint16_t  数据块长度 
 * \return 成功返回true，否则false
 */
#ifdef CFS_WRITE_PORT_ONE_BYTE == (1u)
bool cfs_port_system_flash_write_byte(
    volatile uint32_t addr, uint8_t * data, uint16_t len);
#endif // CFS_WRITE_PORT_ONE_BYTE


/** 往flash中存储数据(半字写)
 * 
 * 要能实现指定长度存入内存(2字节速度存入)
 * 
 * \param uint32_t flash地址
 * \param uint8_t  数据指针 
 * \param uint16_t  数据块长度 
 * \return 成功返回true，否则false
 */
#ifdef CFS_WRITE_PORT_ONE_BYTE == (1u)
bool cfs_port_system_flash_write_half_word(
    volatile uint32_t addr, uint8_t * data, const uint16_t len);
#endif // CFS_WRITE_PORT_HALF_WORD


/** 往flash中存储数据(字写)
 * 
 * 要能实现指定长度存入内存(4字节速度存入)
 * 
 * \param uint32_t flash地址
 * \param uint8_t  数据指针 
 * \param uint16_t  数据块长度 
 * \return 成功返回true，否则false
 */
#ifdef CFS_WRITE_PORT_ONE_WORD == (1u)
bool cfs_port_system_flash_write_word( 
    volatile uint32_t addr, uint8_t * data, const uint16_t len);
#endif // CFS_WRITE_PORT_ONE_WORD


/** 往flash中存储数据(双字写)
 * 
 * 要能实现指定长度存入内存(8字节速度存入)
 * 
 * \param uint32_t flash地址
 * \param uint8_t  数据指针 
 * \param uint16_t  数据块长度 
 * \return 成功返回true，否则false
 */
#ifdef CFS_WRITE_PORT_DOUBLE_WORD == (1u)
bool cfs_port_system_flash_write_double_word(
    volatile uint32_t addr, uint8_t * data, uint16_t len);
#endif // CFS_WRITE_PORT_DOUBLE_WORD


/** 往flash中读取数据
 * 
 * 要能实现指定长度读取数据。
 * 
 * \param uint32_t flash地址
 * \param uint8_t  装载数据的缓存指针 
 * \param uint16_t  数据块长度 
 * \return 成功返回true，否则false
 */
#ifdef CFS_FLASH_READ_MODE == (1u)
bool cfs_port_system_flash_read(
    volatile uint32_t addr, uint8_t * buffer, uint16_t len);
#endif // CFS_FLASH_READ_MODE


/** 上操作flash保护锁。
 * 
 * 对操作flash进程进行保护上锁
 * 
 * \return 成功返回true，否则false
 */
bool cfs_port_system_flash_lock_enable(void);


/** 解除操作flash保护锁。
 * 
 * 解除操作flash进程的保护锁
 * 
 * \return 成功返回true，否则false
 */
bool cfs_port_system_flash_lock_disable(void);


/** 擦除指定页面的数据
 * 
 * \param uint32_t flash地址
 * \param uint16_t  擦除几页 
 * \return 成功返回true，否则false
 */
bool cfs_port_system_flash_erasing_page(volatile uint32_t addr, uint16_t page);


#ifdef __cplusplus
    }
#endif // __cplusplus


#endif /* __CFS_PORT_DEVICE_FLASH_H__ */
