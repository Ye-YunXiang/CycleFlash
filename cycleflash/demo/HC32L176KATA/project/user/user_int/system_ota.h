#ifndef __SYSTEM_OTA_H_
#define __SYSTEM_OTA_H_

#include "main_file_define.h"


/*--------------------------------------------------------------------
 *---CRC16 校验算法
*---------------------------------------------------------------------*/
#define CRC_16_XMODEM  ota_bin_crc_check

// CRC校验拿到的文件
// 参数: 初始值、多项式、结果异或值、输入反转、输出反转、起始指针、数据长度
// 参数中前面的要查表，后面两个数据才是自己给的
// 本程序使用XMODEM标准：ota_bin_crc_check(0,0x1021,0,false,false,puchMsg,usDataLen)
uint16_t ota_bin_crc_check(uint16_t wCRCin, uint16_t wCPoly, uint16_t wResultXOR, \
                            _Bool input_invert, _Bool ouput_invert, const uint8_t *puchMsg, uint32_t usDataLen);

/*--------------------------------------------------------------------
 *---系统复位
*---------------------------------------------------------------------*/
void __OTA_SYSTEM_RESTART(void);

/*--------------------------------------------------------------------
 *---初始化设置
*---------------------------------------------------------------------*/
// 读取 bootloader内容
void bootloader_read(void);
// 写入bootloader内容
void bootloader_write(void);
// 处理 bootloader内的信息
void bootloader_manage(void);

// 清除所有可能会发送内存泄漏的全局动态内
void ota_close_global_interrupt(void);


/*--------------------------------------------------------------------
 *---OTA 文件验证处理
*---------------------------------------------------------------------*/
//@@@@@ 校验文件和切换程序 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void ota_bin_checkout(void);

// 存入bin文件
void ota_write_bin_data(uint8_t * data, uint16_t len, uint32_t addr_move);

// 设置bin文件校验码
void ota_set_bin_crc(uint8_t * bin_head, uint8_t crc_length);

// 获取bin文件
void ota_set_bin_file(uint8_t * bin_head);

// 设置下载的bin文件大小
void ota_set_gsm_bin_size(uint32_t size);

/*---------------------------------------------------------------
 *--get end set bootloader 的数据
*-------------------------------------------------------------*/
// 拿到偏移量
uint32_t ota_get_ota_bin_move(void);

// 得到文件大小
uint32_t ota_get_ota_bin_size(void);

// 获得是否已经切换程序
uint8_t bootloader_get_handover_performed(void);

// 获得是否更新失败
uint8_t bootloader_get_fail(void);

// 清除更新失败标志
void bootloader_clear_fail(void);

#endif
