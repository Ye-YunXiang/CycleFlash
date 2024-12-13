#ifndef __CFS_USER_CONFIG_H__
#define __CFS_USER_CONFIG_H__

// 一页FLASH大小
#define CFS_FLASH_SECTOR_SIZE (512u)


/** 擦除FLASH的默认值，这里只能两个值.
 *  根据芯片手册进行修改。
 *  value: (0xFF)     一般默认都是FF
 *  value: (0x00)    个别带有硬件校验的为00
*/
#define CFS_FLASH_ERASURE (0xFF)


/** XXX: 是否自行实现读取数据，设置错误会有位置情况
 *  下方模式从小到大依次兼容上一层
 *  value: (0u)    (uint8_t *)(arrdess)”的方式取flash值，
 *                  用于大部分32位MCU内置flash，请自行判断。
 *                  保留是因为效率能大大提高。
 * 
 * TODO: 自行读取的部分还没实现
 *  value: (1u)    自行实现读取数据，这里需要实现两个，
 *                 一个是读取一个数据，一个是读取指定长度数据。
 * 注意： 一旦选择自行实现读取，读写速度会变慢。
 * 注意： 如果没有开启，请自行判断用的mcu可否使用“(uint8_t *)(arrdess)”的方式取flash值。
*/
#define CFS_FLASH_READ_MODE (0u)


/* 循环储存的ID使用类型，这里是全局修改.
 * 默认使用uint32_t 类型，这里可以设置为uint64_t.
 * 请自己计算是否用到这么多的ID，减少每个数据块占用的大小。
 * value: (32)     这里设置为 uint32_t
 * value: (64)     这里设置为 uint64_t
*/
// TODO:还没做到项目里
#define CFS_ID_DATA_TYPE (32u)


/* 可写入的最小颗粒，这里只能设定三个值.
 * 根据芯片手册进行修改。
 * value: 1    1Byte
 * value: 2    半字
 * value: 4    1字
*/
#define CFS_WRITE_MIN_PARTICLE      (1u)


// 可使用写入方式，没有的请注释
// 配置好可用的方式之后，请去port文件中去实现它。
/**
 * XXX: 这里说清楚，至少要实现上面说的最小颗粒存储大小的。
 * XXX: 然后这里有用到请一定一定打开。
 * XXX: 没用到的请一定一定注释, 不然出现未知错误。
*/
#define CFS_WRITE_PORT_ONE_BYTE     (1u)    // 1 byte
#define CFS_WRITE_PORT_HALF_WORD    (2u)    // 2 byte
#define CFS_WRITE_PORT_ONE_WORD     (4u)    // 4 byte
// #define CFS_WRITE_PORT_DOUBLE_WORD  (8u)    // 8 byte


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


/* 打开全局页缓存选项，添加和‘CFS_FLASH_SECTOR_SIZE’一样大小的缓存区.
 * 打开后可以对指定ID进行修改，这里擦除页后，重写页。
 * 默认注释不使用，因为用不到，同时可以减少对RAM的负担。
*/
// TODO: 还没做相关的函数
//  #define CFS_FLASH_SECTOR_BUFFER_DEF


/* 打开变长存储的限制，可以存入变长长数据，开放几个专门使用变长存储的函数。
 * 变长数据会在内存中维护属于自己的数据检索表。
 * 由于变长数据，这里初始化的时候会检索flash中所有的数据，所以效率会比定长的慢很多。
 * 对于数据出错，会尝试找到下一个数据在哪里。
 * 如果寻找失败会直接使用最后读取到的数据，并直接对内存进行整理，有丢失数据的风险。
 * 然后这里维护的数据表格为最大回溯10条数据，所以本模式请谨慎使用。
 * 默认注释不使用，减少对RAM的负担。
*/
// TODO: 还没做相关的函数,这里预留，后面在添加。
//  #define CFS_FILESYSTEM_TYPE_VARIABLE_DATA_MODEL

#endif //__CFS_USER_CONFIG_H__
