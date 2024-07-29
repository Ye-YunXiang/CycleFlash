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

/*存储初始化文件系统返回的对象句柄*/
typedef uint64_t cfs_system_handle_t;

// 定义初始化内存的方式
#define CFS_MALLOC      malloc 
#define CFS_FREE        free

#define CFS_LIST_FUNCTION_DISABLE           0
#define CFS_FIXED_LENGTH_FUNCTION_DISABLE   0

/*允许的最长对象名,不许超过255*/
#define CFS_CONFIG_MAX_OBJECT_NAME_LEN   16

enum cycle_object_type
{
    // 没有数据类型
    CFS_FILESYSTEM_OBJECT_TYPE_NULL                 = 0,
    // 存储固定数据，不能随意更改，必须定长
    CFS_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE   = 1,
    // 循环存储数据，随意长度
    CFS_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH    = 2,
    // OTA升级缓存区
    CFS_FILESYSTEM_OBJECT_TYPE_OTA_BUFFER           = 3,
};

/*系统的存储对象，不定长对象记录每个存储区对象的内容*/
typedef struct cfs_filesystem_object 
{
    uint32_t addr_handle;                   // 文件系统在flash中的句柄
    uint16_t sector_size;                   // 扇区大小
    uint16_t data_size;                     // 数据大小, 只有定长有效，包含验证的crc8
    uint8_t list_sector_count;              // 目录页数,建议至少2页
    uint8_t data_sector_count;              // 扇区数量，建议至少3页
    enum cycle_object_type struct_type :8;  // 结构体类型
}cfs_system;

/*存储对象 - 对象类型 - 数据ID 单链表键值对*/
typedef struct cfs_linked_list
{
    struct cfs_filesystem_object *object_handle;    // 存储对象
    char *object_name;                              // 对象名字
    uint32_t data_id;                      	        // 数据块ID
    struct cfs_linked_list *next;                   // 链表下一个节点
    uint16_t valid_id_number;                       // 有效ID个数
    uint8_t this_linked_addr_crc_8;                 // 这个链表对象地址的crc-8值
}cfs_object_linked_list;


/*系统存入内存的数据块*/
// 存入数据：数据-crc校验码
typedef struct 
{
    uint32_t data_id;               // 数据块ID
    uint8_t *data_pointer;          // 存入数据的指针
    uint16_t data_len;              // 存入数据的长度
    uint8_t data_crc8;              // 数据的crc8校验码
}cfs_data_block;

/*系统存入内存的目录块*/
// 存入数据： 数据ID-数据地址-数据长度-crc校验码
typedef struct 
{
    uint32_t data_id;               // 存入的数据ID
    uint32_t data_addr;             // 对应的数据块地址
    uint16_t data_len;              // 数据块长度
    uint8_t  list_crc8;             // 上面的数据块包含（id、地址、长度）的CRC8校验码
}cfs_list_block;


#endif /* __CFS_SYSTEM_DEFINE_H__ */
