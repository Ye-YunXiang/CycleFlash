/*
 * This file is part of the cycle_flash_system Library.
 *
 * Copyright (c) 2026, YeYunXiang, <poetrycloud@foxmail.com>
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

/*
 * 用户需根据 MCU 实现以下 8 个函数
 */

#ifndef __CFS_PORT_H__
#define __CFS_PORT_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CFS_WEAK __attribute__((weak))

// 往flash中存储数据(字节写)
bool cfs_port_write_byte(volatile uint32_t addr, const uint8_t *data, uint16_t len);

// 往flash中存储数据(半字写)
bool cfs_port_write_half_word(volatile uint32_t addr, const uint8_t *data, uint16_t len);

// 往flash中存储数据(字写)
bool cfs_port_write_word(volatile uint32_t addr, const uint8_t *data, uint16_t len);

// 往flash中存储数据(双字写)
bool cfs_port_write_double_word(volatile uint32_t addr, const uint8_t *data, uint16_t len);

// 往flash中读取数据
bool cfs_port_read(volatile uint32_t addr, uint8_t *buf, uint16_t len);

// 上操作flash保护锁
bool cfs_port_lock_enable(void);
// 解除操作flash保护锁。
bool cfs_port_lock_disable(void);

// 擦除指定页面的数据
bool cfs_port_erase_page(volatile uint32_t addr, uint16_t pages);

#ifdef __cplusplus
}
#endif

#endif /* __CFS_PORT_H__ */
