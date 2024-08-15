/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_lcd1602.h
* Description:	Initialize LCD1602
* 
* Author:		Soild_Alasong
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Alasong create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#ifndef __SYSTEM_LCD1602_H
#define __SYSTEM_LCD1602_H

#include "main_file_define.h"

// 开灯
#define System_LCD_Light_Enable()	    Gpio_SetIO(LCD_LIGHT_PORT,LCD_LIGHT_PIN)
// 关灯
#define System_LCD_Light_Disable()      Gpio_ClrIO(LCD_LIGHT_PORT,LCD_LIGHT_PIN)

// 引脚初始化
void system_lcd1602_display_port_init(void);
// 初始化信号发送
void system_lcd1602_dispaly_config_init(void);
// 发送字符串
void system_lcd1602_string_dispaly(char *Lcd_string,uint8_t String_len, uint8_t Row_num);

void system_lcd1602_token_data_display(char *Openpaygo_Token, uint8_t * Token_Num, uint8_t Dispalay_Type);
// 清除数据
void system_lcd1602_display_clear_dispaly(void);

void system_lcd_dispaly_payg_token(uint32_t PayG_Token);


#endif
