/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_internal_rtc.c 
* Description:	Initialize RTC
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "system_internal_rtc.h"

stc_rtc_time_t readtime;		// 从内置 RTC 读取出来的BCD码

/********************************************************************
 ** \brief  初始化RTC，设置时钟，和启用周期中断
 ** @param  无
 ** \retval 无
********************************************************************/
void system_internal_rtc_init(void)
{
	Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);

    stc_rtc_initstruct_t RtcInitStruct;
    Sysctrl_SetPeripheralGate(SysctrlPeripheralRtc,TRUE);					//RTC模块时钟打开
	
			RtcInitStruct.rtcAmpm = RtcPm;                     				//24小时制
			//RtcInitStruct.rtcClksrc = RtcClkRcl;               				//内部低速时钟
			RtcInitStruct.rtcClksrc = RtcClkXtl;               				//外部低速时钟
			RtcInitStruct.rtcPrdsel.rtcPrdsel = RtcPrds;            		//周期中断类型PRDS
			RtcInitStruct.rtcPrdsel.rtcPrds = Rtc1H;		               //周期中断时间间隔
		
			RtcInitStruct.rtcTime.u8Second 		= SEC_T;  	        		//配置RTC时间
			RtcInitStruct.rtcTime.u8Minute 		= MIN_T;
			RtcInitStruct.rtcTime.u8Hour   		= HOUR_T;
			RtcInitStruct.rtcTime.u8Day    		= DAY_T;
			RtcInitStruct.rtcTime.u8Month  		= MON_T;
			RtcInitStruct.rtcTime.u8Year   		= YEAR_T;
		
			RtcInitStruct.rtcCompen = RtcCompenEnable; 	  	    			// 使能时钟误差补偿
			RtcInitStruct.rtcCompValue = 0;             	  				//补偿值  根据实际情况进行补偿
	
    Rtc_Init(&RtcInitStruct);
  
	rtc_alarm_irq_close();                              						//关闭闹钟中断    

    rtc_cmd_open();                                       					//使能RTC开始计数

	Rtc_StartWait();                                        //启动RTC计数，如果要立即切换到低功耗，需要执行此函数  
}

// 启动闹钟
void rtc_alarm_irq_open(void)
{
	Rtc_AlmIeCmd(TRUE);
	EnableNvic(RTC_IRQn, IrqLevel2, TRUE);
}

// 关闭闹钟
void rtc_alarm_irq_close(void)
{
	Rtc_AlmIeCmd(FALSE);
	EnableNvic(RTC_IRQn, IrqLevel2, FALSE);
}

/********************************************************************
 ** \brief  设置RTC时间
 ** @param  时间结构体指针，设置好要设置的时间后传进来
 ** \retval 返回0是正常，其他都是异常
********************************************************************/
uint8_t system_internal_rtc_set_time(struct rtc_time *rtc_time)
{
		if(rtc_time->year > 100) rtc_time->year = 0; 
		//十进制转为BCD码
		rtc_time->year = ((rtc_time->year /10) << 4) + rtc_time->year %10;
		rtc_time->mon  = ((rtc_time->mon  /10) << 4) + rtc_time->mon  %10;
		rtc_time->day  = ((rtc_time->day  /10) << 4) + rtc_time->day  %10;
		rtc_time->hour = ((rtc_time->hour /10) << 4) + rtc_time->hour %10;
		rtc_time->min  = ((rtc_time->min  /10) << 4) + rtc_time->min  %10;
		rtc_time->sec  = ((rtc_time->sec  /10) << 4) + rtc_time->sec  %10;
	
		rtc_alarm_irq_close();
		rtc_cmd_close();
	
		stc_rtc_time_t artcTime;
				artcTime.u8Second 		= rtc_time->sec;  	            	//配置RTC时间
				artcTime.u8Minute 		= rtc_time->min;
				artcTime.u8Hour   		= rtc_time->hour;
				artcTime.u8Day    		= rtc_time->day;
				artcTime.u8Month  		= rtc_time->mon;
				artcTime.u8Year   		= rtc_time->year;
		uint8_t enRet = Rtc_SetTime(&artcTime);
	
		rtc_alarm_irq_open();
//		rtc_Nvic_open();               										//使能RTC中断向量
		rtc_cmd_open(); 
		
		return enRet;
}

/********************************************************************
 ** \brief  获取RTC时时钟，填充到形参的指针中
 ** @param  时间结构体指针
 ** \retval 无
********************************************************************/
void system_internal_rtc_get_time(struct rtc_time *get_rtc)
{
	Rtc_ReadDateTime(&readtime);  // 获取RTC时钟值
	
	get_rtc->sec  = (readtime.u8Second/16)*10 + readtime.u8Second%16;   // 读取时钟值
	get_rtc->min  = (readtime.u8Minute/16)*10 + readtime.u8Minute%16;
	get_rtc->hour = (readtime.u8Hour/16)*10 + readtime.u8Hour%16;
	get_rtc->day  = (readtime.u8Day/16)*10 + readtime.u8Day%16;
	get_rtc->mon  = (readtime.u8Month/16)*10 + readtime.u8Month%16;
	get_rtc->year = (readtime.u8Year/16)*10 + readtime.u8Year%16;
}
