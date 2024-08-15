/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	open_paygo_flash.h
* Description:	Manage OpenPAYGO flash 
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes:        
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

// open_paygo_flash.h文件
#ifndef __OPEN_PAYGO_FLASH_H_
#define __OPEN_PAYGO_FLASH_H_

#include "main_struct_typedef.h"
#include "main_file_define.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "system_seve_flash.h"


enum close_flash_flag
{
    CLOSE_FLASH_LOCK,
    CLOSE_FLASH_DATE_ONE,
    CLOSE_FLASH_DATE_TOW,
    CLOSE_FLASH_PRODUCT_TIME,
    CLOSE_FLASH_PRODUCT_ID
};


// 清除内存页数
void open_paygo_flash_close_flash(enum close_flash_flag flag);

// 存入上锁，以及上锁日期
void open_paygo_flash_write_lock_state(__flash_lock * data, _Bool temp_flag);
// 读取上锁信息
_Bool open_paygo_flash_read_lock_sate(__flash_lock * data);

// 存入ID
void open_paygo_flash_write_product_id(__flash_product_id * data);
// 读取ID
_Bool open_paygo_flash_read_product_id(__flash_product_id * data);
// 存入有上报版本号标志
void open_paygo_flash_write_product_id_have_reported(uint8_t * data);

// 存入结束时间，总可用时间
void open_paygo_flash_write_date_one(__flash_date_one * data);
// 读取结束时间、总可用时间
_Bool open_paygo_flash_read_date_one(__flash_date_one * data);

// 存储间接时间
void open_paygo_flash_write_date_tow(__flash_date_tow * data);
// 读取间接时间
void open_paygo_flash_read_date_tow(__flash_date_tow * data);

// 存入累计时间、count
void open_paygo_flash_write_product_time(__flash_product_time * data);
// 读取可用时间、count
_Bool open_paygo_flash_read_product_time(__flash_product_time * data);

#endif
