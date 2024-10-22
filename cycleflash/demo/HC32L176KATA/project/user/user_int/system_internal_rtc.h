/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_internal_rtc.h
* Description:	Initialize RTC
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#ifndef __SYSTEM_INTERNAL_RTC_H
#define __SYSTEM_INTERNAL_RTC_H

#include "main_file_define.h"

#define	rtc_cmd_open()			    Rtc_Cmd(TRUE)
#define	rtc_cmd_close()			    Rtc_Cmd(FALSE)

#define YEAR_T 	    0x23
#define MON_T		0x01
#define DAY_T		0x01
#define HOUR_T	    0x00
#define MIN_T		0x00
#define SEC_T		0x00
#define	WEEK_T	    0x01

#pragma pack(1)
struct rtc_time
{
    uint8_t  sec;				//时间：秒    
    uint8_t  min;				//时间：分
    uint8_t  hour;    	        //时间：时
    uint8_t  day;				//时间：日    
    uint8_t  mon;               //时间：月
    uint8_t year;			    //时间：年
};
#pragma pack()

// 初始化RTC
void system_internal_rtc_init(void);

// 启动闹钟
void rtc_alarm_irq_open(void);

// 关闭闹钟
void rtc_alarm_irq_close(void);

// 设置RTC时间
uint8_t system_internal_rtc_set_time(struct rtc_time *rtc_time);
// 获取RTC时间
void system_internal_rtc_get_time(struct rtc_time *get_rtc);

#endif
