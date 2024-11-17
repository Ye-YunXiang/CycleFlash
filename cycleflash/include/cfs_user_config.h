#ifndef __CFS_USER_CONFIG_H__
#define __CFS_USER_CONFIG_H__

// 一页FLASH大小
#define CFS_FLASH_SECTOR_SIZE (512u)
/* 擦除FLASH的默认值，这里只能两个值.
 * 根据芯片手册进行修改。
 * value: (0xFF)     一般默认都是FF
 * value: (0x00)    个别带有硬件校验的为00
*/
#define CFS_FLASH_ERASURE (0xFF)

/* 可写入的最小颗粒，这里只能设定三个值.
 * 根据芯片手册进行修改。
 * value: 1    1Byte
 * value: 2    半字
 * value: 4    1字
*/
#define CFS_WRITE_MIN_PARTICLE      (1u)

// 可使用写入方式，没有的请注释
// 配置好可用的方式之后，请去port文件中去实现它。
#define CFS_WRITE_PORT_ONE_BYTE     (1u)    // 1 byte
#define CFS_WRITE_PORT_HALF_WORD    (2u)    // 2 byte
#define CFS_WRITE_PORT_ONE_WORD     (4u)    // 4 byte
#define CFS_WRITE_PORT_DOUBLE_WORD  (8u)    // 8 byte

// 定义初始化内存的方式
#define CFS_MALLOC      malloc 
#define CFS_FREE        free

// 名字的最大长度
#define CFS_NAME_LEN_MAX    (10u)

// DeBug 部分,不需要注释
// TODO:还没对这部分做定义
#define CFS_DEBUG
#define CFS_DEBUG_OUT(x) printf(x)


// 定义在 "cfs_system_utils.h" 中的 
// "uint16_t cfs_system_utils_check(const uint8_t *data, uint32_t data_length)"
/* 库里有三个添加校验方式，校验值类型一定要对上：
 * value:0      CHECK_SUM       (结果按位取反)
 * value:1      CRC16_XMODEM    (结果按位取反)
 * value:2      CRC16_XMODEM 查表法(占用256Byte的RAM) (结果按位取反)
 * value:3      用户自定义，自己去实现
 * 
 * 注释：上面按位取反是防止数据都是 0x00 的时候会校验不出来。
*/ 
#define CFS_CHECK 0



#endif //__CFS_USER_CONFIG_H__
