#ifndef __CFS_USER_CONFIG_H__
#define __CFS_USER_CONFIG_H__

// 一页FLASH大小
#define CFS_FLASH_SECTOR_SIZE 512
// 擦除FLASH的默认值
#define CFS_FLASH_ERASURE 0xFF

// 可写入的最小颗粒
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
#define CFS_DEBUG
#define CFS_DEBUG_OUT(x) printf(x)

// CRC16部分，不需要使用库的查表法，请注释
// 并且请自己实现 "cfs_system_utils.h" 中的 
// "uint16_t cfs_system_utils_check(const uint8_t *data, uint32_t data_length)" 
// 希望您能自己进cfs_system_utils中实现它
#define CFS_CHECK



#endif //__CFS_USER_CONFIG_H__
