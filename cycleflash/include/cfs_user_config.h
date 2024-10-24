#ifndef __CFS_USER_CONFIG_H__
#define __CFS_USER_CONFIG_H__



// 一页FLASH大小
#define CFS_FLASH_SECTOR_SIZE 512
// 擦除FLASH的默认值
#define CFS_FLASH_ERASURE 0xFF

// 定义初始化内存的方式
#define CFS_MALLOC      malloc 
#define CFS_FREE        free

// DeBug 部分,不需要注释
#define CFS_DEBUG
#define CFS_DEBUG_OUT(x) printf(x)

// CRC16部分，不需要使用库的查表法，请注释
// 并且请自己实现 "cfs_system_utils.h" 中的 
// "uint16_t cfs_system_utils_crc16_check(const uint8_t *data, uint32_t data_length)" 函数
#define CFS_CRC16

#endif
