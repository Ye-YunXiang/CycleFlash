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

/********************************************************************
 ** 请不要对这个文件进行任何修改 ***************************************
 ** Please do not make any changes to this file *********************
 ********************************************************************/

#ifndef __CFS_SYSTEM_DEFINE_H__
#define __CFS_SYSTEM_DEFINE_H__

#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

#include "cfs_user_config.h"

// 存储区状态，用于初始化flash后在第一页的头
#define CFS_FLASH_STATE_INIT            (0x01010101)    // 初始化flash
#define CFS_FLASH_STATE_VARIABLE_CYCLE  (0x0A0A0A0A)    // 变长数据使用/预留
#define CFS_FLASH_STATE_FIXED_CYCLE     (0x0F0F0F0F)    // 定长数据使用


/*存储初始化文件系统返回的对象句柄*/
typedef struct cfs_object *cfs_object_handle_ptr;
// 数据定义-----------------
#if CFS_ID_DATA_TYPE == (32u)
    typedef uint32_t cfs_data_id_t;
#elif CFS_FLASH_ERASURE == (0x00)
    typedef uint64_t cfs_data_id_t;
#endif  // CFS_ID_DATA_TYPE


/*无ID状态, 这里为uint32_t*/
#if CFS_FLASH_ERASURE==(0xFF) && CFS_ID_DATA_TYPE==(32u)
    #define CFS_CONFIG_NOT_LINKED_DATA_ID   (UINT_MAX)
#elif CFS_FLASH_ERASURE==(0xFF) && CFS_ID_DATA_TYPE==(64u)
    #define CFS_CONFIG_NOT_LINKED_DATA_ID   (ULLONG_MAX)
#elif CFS_FLASH_ERASURE == (0x00)
    #define CFS_CONFIG_NOT_LINKED_DATA_ID   (0x00000000)
#endif  // CFS_FLASH_ERASURE
/*无有效ID*/
#define CFS_CONFIG_NOT_LINKED_VALID_DATA_ID (0u)


/*不算数据长度，通过数据块算包头包尾的长度*/
// SIZEOF(data_id) + SIZEOF(data_len) + SIZEOF(data_crc_16)
#define CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN (8u)
// 读取数据块的偏移长度
// SSIZEOF(data_id) + SIZEOF(data_len)
#define CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN (6u)


// 定义存储区的数据类型
typedef enum cfs_object_type
{
    //@def 没有数据类型
    CFS_OBJECT_TYPE_NULL,
    //@def 初始化结束标志，没有存入数据
    CFS_OBJECT_TYPE_INIT,
    //@def 存储固定长度数据
    CFS_OBJECT_TYPE_FIXED_DATA_STORAGE,
    //@def 循环变长长度数据/ 预留，还未实现
    CFS_OBJECT_TYPE_VARIABLE_DATA_LENGTH,
}cfs_object_type_t;


/*系统的存储对象，不定长对象记录每个存储区对象的内容*/
typedef struct cfs_object
{
    const uint8_t *name;             // 对象的名字
    const uint32_t address;          // 文件系统在flash中的句柄
    const uint32_t sector_count;     // 扇区数量，建议至少3页
    const uint16_t data_size;        // 存入的数据大小
    const uint8_t data_fill;         // 数据填充大小
} cfs_object_t;

/*存储对象 - 对象类型 - 数据ID 单链表键值对*/
typedef struct cfs_object_list
{
    struct cfs_object_list *next;     // 链表对象
    struct cfs_object *object_handle; // 存储对象

    uint8_t *name;                    // 对象的名字
    cfs_data_id_t data_id;            // 数据块ID
    uint32_t valid_id_number;         // 有效ID个数
    uint16_t data_buffer_size;        // 数据存入大小
} cfs_object_list_t;

// 通用数据块存入缓存区，用于存入数据块，数据块大小用对象中最长的大小。
typedef struct cfs_block_buffer
{
    uint8_t *buffer_ptr;         // 数据块缓存指针
    uint16_t buffer_size; // 数据块缓存大小
    bool use_flag;               // 本缓存目前有没有被占用
} cfs_block_buffer_t;

// 这里要重新定义存入数据的格式
// 这里打算让后面分配好的地址直接分配过来这个结构体。
// 存入数据结构：`ID(4byte) | 长度(2byte) | 数据 | CRC16(2byte)`
typedef struct cfs_data_block
{
    cfs_data_id_t *data_id;       // 数据块
    uint16_t *data_size;          // 存入数据的指针
    uint8_t *data;                // 存入数据的长度ID
    uint16_t *data_check;         // 数据的校验码
} cfs_data_block_t;

// 错误定义判断---------------------------------------------
// TODO: 需要对用户定义部分做判断

#endif /* __CFS_SYSTEM_DEFINE_H__ */
