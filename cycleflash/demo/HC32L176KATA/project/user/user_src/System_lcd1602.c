/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_lcd1602.c 
* Description:	Initialize LCD1602
* 
* Author:		Soild_Alasong
* Date: 	 	2022/09/01
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Alasong create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#include "system_lcd1602.h"


/********************************************************************
 ** \brief  lcd显示字符
 ** @param  需要显示的字符
 ** @param  显示的屏幕位置
 ** \retval 无
********************************************************************/
static void LCD_Dispaly_Write_CMD_OR_Data(uint8_t Cmdordata,uint8_t Writetype)
{
	if(Writetype==0)
	{
		Gpio_ClrIO(LCDRS_PORT,LCDRS_PIN);		//write command
	}
	else
	{
		Gpio_SetIO(LCDRS_PORT,LCDRS_PIN);			//write data
	}
//	PORT_SetBits(PortA,cmdordata&0x00ff);								// 8 线驱动
//	PORT_ResetBits(PortA,(~cmdordata)&0x00ff);					// 8 线驱动
	Gpio_SetPort(GpioPortA,(Cmdordata&0xf0)&0x00f0);			// 4 线驱动
	Gpio_ClrPort(GpioPortA,((~Cmdordata)&0xf0)&0x00f0);		// 4 线驱动
	delay1ms(5);
	Gpio_SetIO(LCDEN_PORT,LCDEN_PIN);
	delay1ms(5);
	Gpio_ClrIO(LCDEN_PORT,LCDEN_PIN);
		
	Gpio_SetPort(GpioPortA,((Cmdordata&0x0f)<<4)&0x00f0);			// 4 线驱动
	Gpio_ClrPort(GpioPortA,(((~Cmdordata)&0x0f)<<4)&0x00f0);	// 4 线驱动
	delay1ms(5);
	Gpio_SetIO(LCDEN_PORT,LCDEN_PIN);
	delay1ms(5);
	Gpio_ClrIO(LCDEN_PORT,LCDEN_PIN);
}


/********************************************************************
 ** \brief  lcd引脚初始化
 ** \retval 无
********************************************************************/
void system_lcd1602_display_port_init(void)
{
	stc_gpio_cfg_t Stc_LCD_Port_Init;
	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
		Stc_LCD_Port_Init.enDir = GpioDirOut;
//		Stc_LCD_Port_Init.enDrv = GpioDrvL;
		Stc_LCD_Port_Init.enPu = GpioPuDisable;
		Stc_LCD_Port_Init.enPd = GpioPdDisable;
//		Stc_LCD_Port_Init.enOD = GpioOdDisable;
//		Stc_LCD_Port_Init.enCtrlMode = GpioAHB;
	

  	Gpio_Init(LCDRS_PORT, LCDRS_PIN, &Stc_LCD_Port_Init);
	Gpio_Init(LCDEN_PORT, LCDEN_PIN, &Stc_LCD_Port_Init);		
	Gpio_Init(LCDRW_PORT, LCDRW_PIN, &Stc_LCD_Port_Init);
	
	Gpio_Init(LCDD4_PORT, LCDD4_PIN, &Stc_LCD_Port_Init);
	Gpio_Init(LCDD5_PORT, LCDD5_PIN, &Stc_LCD_Port_Init);
	Gpio_Init(LCDD6_PORT, LCDD6_PIN, &Stc_LCD_Port_Init);
	Gpio_Init(LCDD7_PORT, LCDD7_PIN, &Stc_LCD_Port_Init);

	Gpio_ClrIO(LCDD4_PORT,LCDD4_PIN);
	Gpio_ClrIO(LCDD5_PORT,LCDD5_PIN);
	Gpio_ClrIO(LCDD6_PORT,LCDD6_PIN);
	Gpio_ClrIO(LCDD7_PORT,LCDD7_PIN);
		
	Gpio_Init(LCD_LIGHT_PORT,LCD_LIGHT_PIN ,&Stc_LCD_Port_Init);
	System_LCD_Light_Disable();

}


/********************************************************************
 ** \brief  LCD1602初始化
 ** \retval 无
********************************************************************/
void system_lcd1602_dispaly_config_init(void)
{
	//delay1ms(200);
	delay1ms(5);
	LCD_Dispaly_Write_CMD_OR_Data(0x30,0);
	delay1ms(5);
	LCD_Dispaly_Write_CMD_OR_Data(0x30,0);
	delay1ms(5);
	LCD_Dispaly_Write_CMD_OR_Data(0x30,0);
	delay1ms(5);
	LCD_Dispaly_Write_CMD_OR_Data(0x28,0);//配置16*2显示，5*7点阵，4位数据接口
	delay1ms(5);
	Gpio_SetIO(LCDEN_PORT,LCDEN_PIN);
	delay1ms(5);
	Gpio_ClrIO(LCDEN_PORT,LCDEN_PIN);
	delay1ms(5);
	LCD_Dispaly_Write_CMD_OR_Data(0x28,0);//配置16*2显示，5*7点阵，4位数据接口
	delay1ms(5);
	LCD_Dispaly_Write_CMD_OR_Data(0x0C,0);//设置开显示，不显示光标
	LCD_Dispaly_Write_CMD_OR_Data(0x06,0);//写字符后地址自动加1
	LCD_Dispaly_Write_CMD_OR_Data(0x01,0);//显示清0，数据指针清0
	delay1ms(5);
}


/********************************************************************
 ** \brief	lcd显示字符串
 ** @param  字符串指针
 ** @param	字符串长度
 ** @param	显示屏显示起始位置
 ** \retval 无
********************************************************************/
void system_lcd1602_string_dispaly(char *Lcd_string,uint8_t String_len, uint8_t Row_num)
{
	LCD_Dispaly_Write_CMD_OR_Data(Row_num,0);
	for(uint8_t i = 0; i < String_len; i++)
	{
		LCD_Dispaly_Write_CMD_OR_Data(*Lcd_string,1);
		Lcd_string++;
		delay1ms(20);
	}
}

/********************************************************************
 ** \brief  键盘输入时显示
 ** @param  char字符指针
 ** @param  显示在第几位
 ** @param	显示类型，0x00,0x23,0x2A
 ** \retval 无
********************************************************************/
void system_lcd1602_token_data_display(char * Openpaygo_Token, uint8_t * Token_Num, uint8_t Dispalay_Type)
{
	if(Dispalay_Type == 0x00)
	{
		LCD_Dispaly_Write_CMD_OR_Data(0x81 + *Token_Num, 0);				
		LCD_Dispaly_Write_CMD_OR_Data(*Openpaygo_Token + 0x30, 1);
	}
	else if(Dispalay_Type == 0x23)
	{
		LCD_Dispaly_Write_CMD_OR_Data(0x81 + *Token_Num, 0);				
		LCD_Dispaly_Write_CMD_OR_Data(0x23, 1);
	}
	else if(Dispalay_Type == 0x2A)
	{
		LCD_Dispaly_Write_CMD_OR_Data(0x80 + *Token_Num, 0);				
		LCD_Dispaly_Write_CMD_OR_Data(0x2A, 1);
	}
	else 
	{;}
}


/********************************************************************
 ** \brief  清除屏幕显示
 ** \retval 无
********************************************************************/
void system_lcd1602_display_clear_dispaly(void)
{
	LCD_Dispaly_Write_CMD_OR_Data(0x01,0);//显示清0，数据指针清0
}
