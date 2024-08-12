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

#ifndef __CFS_SYSTEM_DEFINE_H__
#define __CFS_SYSTEM_DEFINE_H__

#include <stdint.h>
#include <limits.h>

/*存储初始化文件系统返回的对象句柄*/


typedef struct {
    int32_t high;  // 高 32 位
    int32_t low;   // 低 32 位
    int8_t sign;   // 符号位 (1: 正, -1: 负)
}cfs_system_handle_t;

typedef uint64_t cfs_system_handle_t;

// 一页FLASH大小
#define CFS_BUFFER_SIZE 512

// 定义初始化内存的方式
#define CFS_MALLOC      malloc 
#define CFS_FREE        free

enum cycle_object_type
{
    // 没有数据类型
    CFS_FILESYSTEM_OBJECT_TYPE_NULL                 = 0,
    // 存储固定数据
    CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE   = 1,
    // 循环存储数据
    CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH    = 2,
    // OTA升级缓存区
    CFS_FILESYSTEM_OBJECT_TYPE_OTA_BUFFER           = 3,
};


/*系统的存储对象，不定长对象记录每个存储区对象的内容*/
typedef struct cfs_filesystem_object 
{
    uint32_t addr_handle;                   // 文件系统在flash中的句柄
    uint16_t sector_size;                   // 扇区大小
    uint16_t sector_count;                  // 扇区数量，建议至少3页
    uint16_t data_size;                     // 存入的数据大小
    enum cycle_object_type struct_type :8;  // 结构体类型
}cfs_system;


/*无ID状态*/
#define CFS_CONFIG_NOT_LINKED_DATA_ID       UINT_MAX
/*无有效ID*/
#define CFS_CONFIG_NOT_LINKED_VALID_DATA_ID   0

/*存储对象 - 对象类型 - 数据ID 单链表键值对*/
typedef struct cfs_linked_list
{
    struct cfs_filesystem_object *object_handle;            // 存储对象
    uint32_t data_id;                      	                // 数据块ID
    struct cfs_linked_list *prior;                          // 链表上一个节点
    struct cfs_linked_list *next;                           // 链表下一个节点
    uint16_t valid_id_number;                               // 有效ID个数
    uint16_t this_linked_addr_crc_16;                       // 这个链表对象地址的crc-16-xmodem值
}cfs_object_linked_list;


/*不算数据长度，通过数据块算包头包尾的长度*/
// SIZEOF(data_id) + SIZEOF(data_crc_16)
#define CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN  6
// 读取数据块的偏移长度
// SIZEOF(data_id)
#define CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN    4
/*系统存入内存的数据块*/
// 存入数据结构：`ID(4byte) | 数据(最大为1页大小-7) | CRC16(2byte)`
typedef struct 
{
    uint32_t data_id;               // 数据块ID
    uint8_t *data_pointer;          // 存入数据的指针
    uint16_t data_len;              // 存入数据的长度
    uint16_t data_crc_16;           // 数据的crc8校验码
}cfs_data_block;


#endif /* __CFS_SYSTEM_DEFINE_H__ */
