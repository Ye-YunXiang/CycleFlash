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


#ifndef __CFS_SYSTEM_UTILS_H__
#define __CFS_SYSTEM_UTILS_H__

#include <stdbool.h>
#include <stdint.h>
#include "cfs_system_define.h"

uint8_t cfs_system_utils_crc_8_check(const uint8_t *data, uint32_t data_length, bool inversion_bit);

uint16_t cfs_system_utils_crc16_xmodem_check(const uint8_t *data, uint32_t data_length, bool inversion_bit);

// 直接传入数据块，验证数据块内的数据，然后得出CRC16-xmodem
uint16_t cfs_system_utils_crc16_xmodem_check_data_block(const cfs_data_block *data, bool inversion_bit);

#endif /* __CFS_SYSTEM_UTILS_H__ */
