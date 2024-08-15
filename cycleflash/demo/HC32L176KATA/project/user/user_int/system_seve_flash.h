/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_seve_flash.h
* Description:	initialize FLASH
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#ifndef __SYSTEM_SEVE_FLASH_H
#define __SYSTEM_SEVE_FLASH_H

#include "main_file_define.h"

// 页251：1F600 - 1F7FF     页252：1F800 - 1F9FF
// 页253：1FA00 - 1FBFF     页254：1FC00 - 1FDFF    页255：1FE00 - 1FFFF

// 上锁，251页,存储：是否上锁（0为上锁）、连续输入错误次数、发出上锁时间
#define FLASH_ADDR_LOCK_STATE                           (0x0001F600)
#define FLASH_ADDR_LOCK_ERROR                           (0x0001F604)
#define FLASH_ADDR_LOCK_DATE                            (0x0001F608)

// 倒计时，252页，存储：第一次充值时间、总可用时间。
#define FLASH_ADDR_DATE_FINAL_DATE                      (0x0001F800)
#define FLASH_ADDR_DATE_SUM_AVAILABLE_TIME              (0x0001F808)

// 倒计时，253页，存储：时间位置（7个地址），30分钟存储的时间（8个地址）。
#define FLASH_ADDR_DATE_LOCATION_DATE                   (0x0001FA00)
#define FLASH_ADDR_DATE_INTERVAL_DATE                   (0x0001FA08)      // 8个地址8个地址的往上加

// 充值累计，254页，存储：累计时间（1个地址）、count值（1个地址）。
#define FLASH_ADDR_PRODUCT_TIME_ADD_TIME                (0x0001FC00)
#define FLASH_ADDR_PRODUCT_TIME_TOKEN_COUNT             (0x0001FC04)

// 存储ID，255页，间隔10个地址。
// 每个的长度都是固定的。
#define FLASH_ADDR_PRODUCT_ID_STARTING_CODE             (0x0001FE00)      // Starting Code
#define FLASH_ADDR_PRODUCT_ID_UNIT_NUMBER_CODE          (0x0001FE10)      // Unit Number Code
#define FLASH_ADDR_PRODUCT_ID_SECRET_KEY                (0x0001FE20)      // Secret Key       16地址
#define FLASH_ADDR_PRODUCT_ID_DEVICE_SECRET             (0x0001FE40)
#define FLASH_ADDR_PRODUCT_ID_PRODUCT_KEY               (0x0001FE70)
#define FLASH_ADDR_PRODUCT_ID_HAVE_REPORTED             (0x0001FE90)      // 是否上报过版本号
#define FLASH_ADDR_PRODUCT_ID_VERSION                   (0x0001FEA0)      // 存储版本号

// 下面结构体采用紧凑型的方式存储在内存
#pragma pack(1)

// 存储是否上锁相关事宜                不能随意修改！
struct flash_lock
{
	uint16_t lock_date;           // 上锁结束时间：这里可以定义为默认上锁30分钟，当前时间戳＋30分钟的秒
    uint8_t lock_error;             // 输入错误次数,存储的就正常数字，每次存储每次擦除
   
};

// 存储倒计时的变量，第一次存储时间      不能随意修改！
struct flash_date_one
{
    uint32_t available_time;        // 共可使用时间，存秒吧
    uint32_t final_date;         // 结束时间戳，存入时间戳

};

// 存储后面30分钟存储一次的时间     不能随意修改！
struct flash_date_tow
{
    uint32_t  interval_date;      // 间距存储时间时间戳
    uint8_t  location_date[7];      // 间接存储时间地址,注意初始化创建的时候，全部赋值为255
};

// 存储系统相关的：积累充值时间、count值    不能随意修改！
struct flash_product_time
{
    uint16_t add_time;              // 累计充值时间
    uint16_t token_count;           // Token Count

};

// 和系统相关的：三个ID值   不能随意修改！
struct flash_product_id
{
    uint8_t  secret_key[16];        // 密钥
    uint32_t starting_code;         // 启动码
    uint32_t unit_number_code;      // ID
    uint8_t  device_secret[32];
    uint8_t  product_key[11];
    
    // 存入OTA相关
    uint8_t  have_reported;         // 是否上报过版本号，0为无
    uint8_t  version[8];            // 版本号
};


// 只作用在上面
#pragma pack()

/********************************************************************************
 ** 辅助   
 *******************************************************************************/

// (必须为uint8，一个字节)存储数组数据，主：存储地址、源数据地址、 数组长度
void system_flash_write_array_data(volatile uint32_t addr, volatile uint8_t * data,  uint16_t len);  
// (必须为uint8，一个字节)读取数组数据, 主：读取地址、存储数据地址、数组长度
void system_flash_read_array_data(volatile uint32_t addr, volatile uint8_t * data, uint16_t len);       

// 用来计算间隔存储时间的地址有几个0,前6个地址最高位为0代表这里都存满了，读后面的地址。
// 存储地址偏移量为8，所以有几个零，就偏移几个8地址，就为间隔存储时间的地址。
uint32_t calculate_interval_date_addr(volatile uint8_t * data);

// 清除指定页
void system_write_flash_close(volatile uint32_t addr);
/********************************************************************************
 ** 业务逻辑   
 *******************************************************************************/


void system_write_flash_lock_close(void);
// 写入连续输入错误次数
void system_write_flash_lock_error(uint8_t * data);
// 写入上锁时间，以及上锁状态
void system_write_flash_lock_state(struct flash_lock * data);
// 读取上锁相关
void system_read_flash_lock_state(struct flash_lock * data);

// 清除页252
void system_write_flash_date_one_close(void);
// 存储结束时间、总可用时间
void system_write_flash_date_one(struct flash_date_one * data);
// 读取结束时间、总可用时间
void system_read_flash_date_one(struct flash_date_one * data);

// 清除页253
void system_write_flash_date_tow_close(void);
// 存储间接存储地址、间接存储时间
void system_write_flash_date_tow(struct flash_date_tow * data);
// 存储间接存储时间标志位
void system_write_flash_date_tow_flag(uint8_t * add);
// 读取间接存储地址、间接存储时间,temp_flag为1就是校验标志位，为0直接读取
void system_read_flash_date_tow(struct flash_date_tow * data, _Bool temp_flag);

// 清除页254
void system_write_flash_product_time_close(void);
// 一次写入数据
void system_write_flash_product_time_data(struct flash_product_time * data);
// 一次读取数据
void system_read_flash_product_time_data(struct flash_product_time * data);

// 清除页255
void system_write_flash_product_id_close(void);
// 一次写入数据
void system_write_flash_product_id_data(struct flash_product_id * data);
// 一次读取数据
void system_read_flash_product_id_data(struct flash_product_id * data);
// 写入有上报数据
void system_write_flash_product_id_have_reported(uint8_t *data);




#endif
