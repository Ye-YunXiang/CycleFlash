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

#include <assert.h>
#include <stddef.h>
#include "cfs_system_utils.h"


// CRC校验 // CRC-8 多项式：x^8 + x^2 + x^1 + x^0 (0x07)
uint8_t cfs_system_utils_crc_8_check(const uint8_t *data, uint32_t data_length, bool inversion_bit)
{
    uint8_t crc = 0;
    for (uint64_t i = 0; i < data_length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }

    // 是否按位取反
    if(inversion_bit == true)
    {
        return ~(crc);
    }
    else
    {
        return (crc);
    }
}

/* 初始值（0）、多项式（0x1021）、结果异或值（0）、输入翻转（falsh）、输出翻转（falsh）
 * 参数： uint8_t * 起始指针
 *       uint32_t  数据
*/
uint16_t cfs_system_utils_crc16_xmodem_check(const uint8_t *data, uint32_t data_length, bool inversion_bit)
{
    uint16_t crc_int = 0;
    uint8_t temp_char = 0;

    while (data_length--)
    {
        temp_char = *(data++);
        crc_int ^= (temp_char << 8);
        for (int i = 0; i < 8; i++)
        {
            if (crc_int & 0x8000)
                crc_int = (crc_int << 1) ^ (0x1021);
            else
                crc_int = crc_int << 1;
        }
    }

    // 是否按位取反
    if(inversion_bit == true)
    {
        return ~(crc_int^0);
    }
    else
    {
        return (crc_int^0);
    }
}


/* 初始值（0）、多项式（0x1021）、结果异或值（0）、输入翻转（falsh）、输出翻转（falsh）
 * 参数： uint8_t * 起始指针
 *       uint32_t  数据
*/
// 根据cfs系统专门创建的验证数据块函数, 这个数据块的crc16不参与验证
uint16_t cfs_system_utils_crc16_xmodem_check_data_block(const cfs_data_block *data, bool inversion_bit)
{
    assert(data->data_len != 0 && data->data_pointer != NULL);

    uint8_t *data_block_handle = (uint8_t *)(&data->data_id);
    uint32_t data_block_length = \
        data->data_len + sizeof(data->data_id);
    uint16_t crc_int = 0;
    uint8_t temp_char = 0;

    while (data_block_length)
    {
        if(data_block_length == data->data_len)
        {
            data_block_handle = data->data_pointer;
        }

        temp_char = *(data_block_handle++);
        crc_int ^= (temp_char << 8);
        for (int i = 0; i < 8; i++)
        {
            if (crc_int & 0x8000)
                crc_int = (crc_int << 1) ^ (0x1021);
            else
                crc_int = crc_int << 1;
        }
        
        data_block_length--;
    }

    // 是否按位取反
    if(inversion_bit == true)
    {
        return ~(crc_int^0);
    }
    else
    {
        return (crc_int^0);
    }
}
