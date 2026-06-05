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

#ifndef __CYCLE_FLASH_SYSTEM_H__
#define __CYCLE_FLASH_SYSTEM_H__

#include <stdint.h>
#include <limits.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===========================================================================
// 用户配置 —— 根据 MCU 按需修改
// ===========================================================================

// 一页FLASH大小
#ifndef CFS_FLASH_SECTOR_SIZE  
#define CFS_FLASH_SECTOR_SIZE               (512u)
#endif // CFS_FLASH_SECTOR_SIZE   // 方便测试用例添加的守卫宏，关闭后无法测试

/** 擦除FLASH的默认值，这里只能两个值.
 *  根据芯片手册进行修改。
 *  value: (0xFF)     一般默认都是FF
 *  value: (0x00)    个别带有硬件校验的为00
*/
#define CFS_FLASH_ERASURE                   (0xFFu)

/** XXX: 是否自行实现读取数据，设置错误会有位置情况
 *  下方模式从小到大依次兼容上一层
 *  value: (0u)     定制使用32位MCU内置flash模式。
 *                  (uint8_t *)(arrdess)”的方式取flash值，用于大部分32位MCU内置flash，请自行判断。
 *                  保留能大大能大大提高效率,去除了拷贝内存的步骤，同时节省了内部读取的缓存区。(高效)
 *                  (重要！！！ -》 由于内置flash很稳定，所以一次读取错误不会在读取第二次)
 * 
 *  value: (1u)     自定义模式。
 *                  自行实现接口的读取数据函数，开启后会创建数据缓存区，用于读取数据。
 *                  (重要！！！ -》 本模式第一次读取如果错误了，会尝试读取第二次)
 *                  
 * 注意： 如果没有开启，请自行判断用的mcu可否使用“(uint8_t *)(arrdess)”的方式取flash值。
*/
#ifndef CFS_FLASH_READ_MODE
#define CFS_FLASH_READ_MODE                 (0u)
#endif // CFS_FLASH_READ_MODE   // 方便测试用例添加的守卫宏，关闭后无法测试

/** 如果上方读取模式选用了自定义模式，那需要用户设定好读取缓存区的大小。
 * 毕竟机制不同，所以用户要在下方设定好可可覆盖用户存入数据大小的缓存区，共给库进行读取。
 * 设定的缓存大小要大于用户存入的最大数据大小，以及4字节倍数大小。
 * 程序运行中不会做验证，所以如果设定太小，会有内存溢出等未知错误。
*/
#define CFS_FLASH_READ_BUFFER_SIZE      (512u)

/* 循环储存的ID使用类型，这里是全局修改.
 * 默认使用uint32_t 类型，这里可以设置为uint64_t.
 * 请自己计算是否用到这么多的ID，减少每个数据块占用的大小。
 * value: (32)     这里设置为 uint32_t
 * value: (64)     这里设置为 uint64_t
*/
#define CFS_ID_DATA_TYPE                    (32u)

// 配置好可用的方式之后，请去port文件中去实现它。
/**
 * value: (0u)     关闭
 * value: (1u)     打开
 * 
 * XXX: 这里说清楚，至少要实现上面说的最小颗粒存储大小的。
 * XXX: 然后这里有用到请一定一定打开。
 * XXX: 没用到的请一定一定关闭, 不然出现未知错误。
 * 
 * 注意：这里修改要固定，与存入数据的位数字节对齐补充字节有关。
 * ！！！ 所以本配置必须固定，如果修改了，可能导致目前设备存储的数据无法读取。 ！！！
*/
/*  写入方式开关，需与 cfs_port.c 实现匹配 */
#define CFS_WRITE_PORT_ONE_BYTE             (1u)
#define CFS_WRITE_PORT_HALF_WORD            (0u)
#define CFS_WRITE_PORT_ONE_WORD             (0u)
#define CFS_WRITE_PORT_DOUBLE_WORD          (0u)

/* 校验算法
 * 库里有三个添加校验方式，校验值类型一定要对上：
 * value:(0u)      CHECK_SUM       (结果按位取反)
 * value:(1u)      CRC16_XMODEM    
 * value:(2u)      CRC16_XMODEM 查表法(占用256Byte的RAM)
 * value:(3u)      用户自定义，自己去实现
 * 
 * 注释：上面校验和按位取反是防止数据都是 0x00 的时候会校验不出来。
*/ 
#define CFS_CHECK                           (1u)


// DeBug 部分,不需请设置 CFS_DEBUG (0u)
/* DeBug 部分
 * CFS_DEBUG (0u)      不启用DEBUG
 * CFS_DEBUG (1u)      启用DEBUG
 * 
 * 注释：如果启用了DEBUG，自己按需修改下方的宏 "CFS_DEBUG_OUT" 和 "CFS_ASSERT"。
*/ 
#ifndef CFS_DEBUG
#define CFS_DEBUG (1u)
#endif
#if CFS_DEBUG == (1u)
    /** 对于下方两个宏参数的解释
     * x ：断言的条件，直接填入条件即可，结果为否触发断言。
     * y ：断言失败后的输出信息，输入为字符串。
    */
    // 下方为上方DEBUG定义后的断言方式。
    // 可自定义-----------------------
    #define CFS_ASSERT(x) if (!x) {    \
        while(1);                       \
    }
#else
    // 下方为上方DEBUG定义后的断言方式。
    // 可自定义-----------------------
    #define CFS_ASSERT(x) ((void)0)
#endif  // CFS_DEBUG




// ===========================================================================
// 下方为库内实现，用户修改 ----------------------------------------------------
// ===========================================================================
// 类型定义
// ===========================================================================
// 版本号
#define CFS_VERSION ("3.0.0")

#define CFS_RETURN_ERROR                    (-1)

#if CFS_WRITE_PORT_ONE_BYTE != 0
    #define CFS_WRITE_MIN_PARTICLE          (1u)
#elif CFS_WRITE_PORT_HALF_WORD != 0
    #define CFS_WRITE_MIN_PARTICLE          (2u)
#elif CFS_WRITE_PORT_ONE_WORD != 0
    #define CFS_WRITE_MIN_PARTICLE          (4u)
#elif CFS_WRITE_PORT_DOUBLE_WORD != 0
    #define CFS_WRITE_MIN_PARTICLE          (8u)
#else
    #error "No write granularity configured!"
#endif

// 数据定义-----------------
/*无ID状态, 这里为uint32_t*/
// 经过思考，ID的正式使用从0开始。
#if CFS_ID_DATA_TYPE == (32u)
    typedef uint32_t cfs_data_id_t;
    #define CFS_CONFIG_NOT_LINKED_DATA_ID           (UINT_MAX)
    #define CFS_CONFIG_DATA_ID_UPPER_LIMIT          (UINT_MAX - 10u)
    #define CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN  (8u)
    #define CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN    (6u)
#elif CFS_ID_DATA_TYPE == (64u)
    typedef uint64_t cfs_data_id_t;
    #define CFS_CONFIG_NOT_LINKED_DATA_ID           (ULLONG_MAX)
    #define CFS_CONFIG_DATA_ID_UPPER_LIMIT          (ULLONG_MAX - 10u)
    #define CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN  (12u)
    #define CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN    (10u)
#else
    #error "CFS_ID_DATA_TYPE must be 32 or 64"
    #error "您没有对循环储存的ID使用类型宏进行配置！"
#endif

#define CFS_CONFIG_NOT_LINKED_VALID_DATA_ID         (0u)

/*  cfs_object_t —— 循环存储对象控制块。*/
typedef struct cfs_object 
{
    /* --- 用户配置（初始化时写入，之后只读） --- */
    uint32_t address;                           // 起始地址
    uint32_t sector_count;                      // 扇区数（≥3）
    uint16_t data_size;                         // 单条数据大小

    /* --- 运行时状态（由库维护） --- */
    cfs_data_id_t data_id;                      // 当前写入 ID
    cfs_data_id_t valid_id;                     // 可读取的记录数
    uint16_t data_buffer_size;                  // 块总大小（含头部+填充）
} cfs_object_t;

/*  句柄类型（兼容旧代码） */
typedef struct cfs_object *cfs_object_handle_ptr;

// 数据块结构（仅内存中使用，flash 只存前三个字段）
// 存入数据结构：`ID(变长) | 长度(2byte) | check(2byte) | 数据`
// 校验码只校验有效数据，等于存入数据长度
#pragma pack(1)
typedef struct cfs_data_block 
{
    cfs_data_id_t data_id;
    uint16_t data_len;
    uint16_t data_check;
    uint8_t *data_ptr;
} cfs_data_block_t;
#pragma pack()






// ===========================================================================
// 公共 API
// ===========================================================================

/** 初始化文件管理系统对象
 *
 * \param obj 存储对象
 * \param address 缓存区起始地址
 * \param sector_count 共有几页flash区域。
 * \param data_size 用户数据块大小。
 * \return cfs_object_handle_ptr 初始化后的句柄，如果初始化失败返回 false。
 * \brief 户提供 cfs_object_t 存储，库扫描 flash 恢复运行时状态。
 */
bool cfs_nv_object_init(cfs_object_t *obj,
                        uint32_t address, 
                        uint32_t sector_count,
                        uint16_t data_size);

/** 往内存中添加写入数据
 *
 * \param obj 存储对象
 * \param data u8-写入数据的指针
 * \param len u16-写入数据长度
 * \param error_retry u8-写入失败重试次数,每次重试都会向上累加ID。
 * \return 写入成功返回写入的数据个数，如果写入失败返回 -1。
 */
int cfs_nv_write(cfs_object_t *obj, 
                const uint8_t *data,
                uint16_t len, 
                uint8_t error_retry);

/** 根据ID读取内存中的数据
 *
 * \param obj 存储对象
 * \param data 装载数据的指针
 * \param len 读取数据长度
 * \param read_in_past 读取第 read_in_past 条历史数据（0=最新）
 * \return 读取数据成功后返回读取数据的个数，如果读取失败返回 0， 错误返回-1。
 */
int cfs_nv_read(cfs_object_t *obj, 
                uint8_t *data,
                uint16_t len, 
                cfs_data_id_t read_in_past);

/** 清除指定对象的存储空间
 *
 * \param obj 存储对象
 * \return 擦除成功返回true，反则为flash
 * \brief 擦除对象所有扇区，重置 ID
 */
bool cfs_nv_clear(cfs_object_t *obj);

/** 返回目前存储对象的ID
 *
 * \param obj 存储对象
 * \return 获取当前ID成功就返回ID，反则为flash
 */
cfs_data_id_t cfs_nv_get_current_id(const cfs_object_t *obj);

/** 获取有效记录数
 *
 * \param obj 存储对象
 * \return 获取当成功就返回ID，反则为flash
 */
cfs_data_id_t cfs_nv_get_current_valid_id(const cfs_object_t *obj);

#ifdef __cplusplus
}
#endif

/* expose internal functions for unit testing */
#ifdef CFS_TEST
uint32_t      _data_addr(const cfs_object_t *obj, cfs_data_id_t id);
cfs_data_id_t _valid_id(const cfs_object_t *obj, cfs_data_id_t id);
#endif

#endif /* __CYCLE_FLASH_SYSTEM_H__ */
