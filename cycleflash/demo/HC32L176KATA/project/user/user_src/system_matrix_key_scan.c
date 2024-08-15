/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_matrix_key.c
* Description: 	Scan the keyboard for 'matrix_value' and 'keypad_pressed_keycode_allow' signals
* 
* Author:		Soild_Alasong
* Date: 	 	2022/9/01
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Alasong create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "system_matrix_key_scan.h"

#define MATRIX_KEY_DELAY_1MS		1
#define MATRIX_KEY_DELAY_50MS		50
#define MATRIX_KEY_DELAY_COUNT	50

struct 
{
	bool key_r1_press_flag;
	bool key_r2_press_flag;
	bool key_r3_press_flag;
	bool key_r4_press_flag;
}matrix_key_typedef;

struct 
{	
	bool keypad_scan_count_flag;
}system_key_flag;

// 按键扫描次数
struct 
{
	uint8_t matrix_key_scan_count;
}key_count;

volatile matrix_typeedef matrix_value;		

static volatile uint8_t ASCII_CODE[13] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x2A,0x23};	//0\1\2\3\4\5\6\7\8\9\*\#

// 全局按键输入信号
bool keypad_pressed_keycode_allow;


void system_matrix_key_scan_init(void)
{
	stc_gpio_cfg_t		stc_in_matrix_key_init;
	stc_gpio_cfg_t 	stc_out_matrix_key_init;
	
	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
	stc_out_matrix_key_init.enDir				= GpioDirOut;
	stc_out_matrix_key_init.enDrv				= GpioDrvL;
	stc_out_matrix_key_init.enPu 				= GpioPuDisable;
	stc_out_matrix_key_init.enPd 				= GpioPdDisable;
//	stc_out_matrix_key_init.enCtrlMode	= GpioAHB;
	
	stc_in_matrix_key_init.enDir				= GpioDirIn;
	stc_in_matrix_key_init.enPu 				= GpioPuEnable;
	stc_out_matrix_key_init.enCtrlMode			= GpioAHB;

//	feedback_single_keycode = true;
	
	Gpio_Init(KEY_C1_PORT, KEY_C1_PIN, &stc_out_matrix_key_init);
	Gpio_Init(KEY_C2_PORT, KEY_C2_PIN, &stc_out_matrix_key_init);
	Gpio_Init(KEY_C3_PORT, KEY_C3_PIN, &stc_out_matrix_key_init);
	
	Gpio_Init(KEY_R1_PORT, KEY_R1_PIN, &stc_in_matrix_key_init);
	Gpio_Init(KEY_R2_PORT, KEY_R2_PIN, &stc_in_matrix_key_init);
	Gpio_Init(KEY_R3_PORT, KEY_R3_PIN, &stc_in_matrix_key_init);
	Gpio_Init(KEY_R4_PORT, KEY_R4_PIN, &stc_in_matrix_key_init);
}


/******************************************************************************
 ** \brief  矩阵按键扫描函数
 **
 **	\param	无
 **
 ** \return 无
 **
 ******************************************************************************/
void system_martix_key_scan(void)
{
	if(key_count.matrix_key_scan_count == 0)
	{
		Gpio_ClrIO(KEY_C1_PORT,KEY_C1_PIN);
		Gpio_SetIO(KEY_C2_PORT,KEY_C2_PIN);
		Gpio_SetIO(KEY_C3_PORT,KEY_C3_PIN);
	}
	else if(key_count.matrix_key_scan_count == 1)
	{
		Gpio_SetIO(KEY_C1_PORT,KEY_C1_PIN);
		Gpio_ClrIO(KEY_C2_PORT,KEY_C2_PIN);
		Gpio_SetIO(KEY_C3_PORT,KEY_C3_PIN);
	}
	else 
	{
		Gpio_SetIO(KEY_C1_PORT,KEY_C1_PIN);
		Gpio_SetIO(KEY_C2_PORT,KEY_C2_PIN);
		Gpio_ClrIO(KEY_C3_PORT,KEY_C3_PIN);
	}

		//=====================================================================//
	//	                   scan first row								   //
	//=====================================================================//
	if(Gpio_GetInputIO(KEY_R1_PORT,KEY_R1_PIN) == false)
	{
		if(matrix_key_typedef.key_r1_press_flag == false)
		{
			uint8_t temp_count = 0;
			while( temp_count < MATRIX_KEY_DELAY_COUNT )
			{
				if(Gpio_GetInputIO(KEY_R1_PORT,KEY_R1_PIN) != false)	
					break;
				delay1ms(MATRIX_KEY_DELAY_1MS);
				temp_count++;
			}
			//if(Gpio_GetInputIO(KEY_R1_PORT,KEY_R1_PIN) == false)
			if(temp_count == MATRIX_KEY_DELAY_COUNT)
			{
				matrix_key_typedef.key_r1_press_flag = true;
				system_key_flag.keypad_scan_count_flag = true;
				keypad_pressed_keycode_allow = true;

				switch(key_count.matrix_key_scan_count)
				{
					case 0:
						matrix_value.matrix_scan_value = ASCII_CODE[1];
						break;
					
					case 1:	
						matrix_value.matrix_scan_value = ASCII_CODE[2];
						break;
					
					case 2:	
						matrix_value.matrix_scan_value = ASCII_CODE[3];
						break;
					
					default:
						break;
				}
			}
		}
	}
	else 
	{
		if(matrix_key_typedef.key_r1_press_flag == true)
		{
			delay1ms(MATRIX_KEY_DELAY_50MS);
			if(Gpio_GetInputIO(KEY_R1_PORT,KEY_R1_PIN) == true)
			{
				matrix_key_typedef.key_r1_press_flag = false;
				system_key_flag.keypad_scan_count_flag = false;
			}
		}
	}
	//=====================================================================//
	//				scan second row		    								//
	//=====================================================================//
	if(Gpio_GetInputIO(KEY_R2_PORT,KEY_R2_PIN) == false)
	{
		if(matrix_key_typedef.key_r2_press_flag == false)
		{
			uint8_t temp_count = 0;
			while( temp_count < MATRIX_KEY_DELAY_COUNT )
			{
				if(Gpio_GetInputIO(KEY_R2_PORT,KEY_R2_PIN) != false)	
					break;
				delay1ms(MATRIX_KEY_DELAY_1MS);
				temp_count++;
			}
			if(temp_count == MATRIX_KEY_DELAY_COUNT)
			//if(Gpio_GetInputIO(KEY_R2_PORT,KEY_R2_PIN) == false)
			{
				matrix_key_typedef.key_r2_press_flag = true;
				system_key_flag.keypad_scan_count_flag = true;
				keypad_pressed_keycode_allow = true;
				switch(key_count.matrix_key_scan_count)
				{
					case 0:
						matrix_value.matrix_scan_value = ASCII_CODE[4];
						break;
					
					case 1:	
						matrix_value.matrix_scan_value = ASCII_CODE[5];
						break;
					
					case 2:	
						matrix_value.matrix_scan_value = ASCII_CODE[6];
						break;
					
					default:
						break;
				}
			}
		}
	}
	else 
	{
		if(matrix_key_typedef.key_r2_press_flag == true)
		{
			delay1ms(MATRIX_KEY_DELAY_50MS);
			if(Gpio_GetInputIO(KEY_R2_PORT,KEY_R2_PIN) == true)
			{
				matrix_key_typedef.key_r2_press_flag = false;
				system_key_flag.keypad_scan_count_flag = false;
			}
		}
	}
	//=====================================================================//
	//					sacn thirth row			   			    		   //
	//=====================================================================//
	if(Gpio_GetInputIO(KEY_R3_PORT,KEY_R3_PIN) == false)
	{
		if(matrix_key_typedef.key_r3_press_flag == false)
		{
			uint8_t temp_count = 0;
			while( temp_count < MATRIX_KEY_DELAY_COUNT )
			{
				if(Gpio_GetInputIO(KEY_R3_PORT,KEY_R3_PIN) != false)	
					break;
				delay1ms(MATRIX_KEY_DELAY_1MS);
				temp_count++;
			}
			if(temp_count == MATRIX_KEY_DELAY_COUNT)
			//if(Gpio_GetInputIO(KEY_R3_PORT,KEY_R3_PIN) == false)
			{
				matrix_key_typedef.key_r3_press_flag = true;
				system_key_flag.keypad_scan_count_flag = true;
				keypad_pressed_keycode_allow = true;
				switch(key_count.matrix_key_scan_count)
				{
					case 0:
						matrix_value.matrix_scan_value = ASCII_CODE[7];
						break;
					
					case 1:	
						matrix_value.matrix_scan_value = ASCII_CODE[8];
						break;
					
					case 2:	
						matrix_value.matrix_scan_value = ASCII_CODE[9];
						break;
					
					default:
						break;
				}
			}
		}
	}
	else 
	{
		if(matrix_key_typedef.key_r3_press_flag == true)
		{
			delay1ms(MATRIX_KEY_DELAY_50MS);
			if(Gpio_GetInputIO(KEY_R3_PORT,KEY_R3_PIN) == true)
			{
				matrix_key_typedef.key_r3_press_flag = false;
				system_key_flag.keypad_scan_count_flag = false;
			}
		}
	}
	//=====================================================================//
	//                   scan fourth row									//
	//=====================================================================//
	if(Gpio_GetInputIO(KEY_R4_PORT,KEY_R4_PIN) == false)
	{
		if(matrix_key_typedef.key_r4_press_flag == false)
		{
			uint8_t temp_count = 0;
			while( temp_count < MATRIX_KEY_DELAY_COUNT )
			{
				if(Gpio_GetInputIO(KEY_R4_PORT,KEY_R4_PIN) != false)	
					break;
				delay1ms(MATRIX_KEY_DELAY_1MS);
				temp_count++;
			}
			if(temp_count == MATRIX_KEY_DELAY_COUNT)
			//if(Gpio_GetInputIO(KEY_R4_PORT,KEY_R4_PIN) == false)
			{
				matrix_key_typedef.key_r4_press_flag = true;
				system_key_flag.keypad_scan_count_flag = true;
				keypad_pressed_keycode_allow = true;
				switch(key_count.matrix_key_scan_count)
				{
					case 0:
						matrix_value.matrix_scan_value = ASCII_CODE[10];
//						feedback_single_keycode = false;
						break;
					
					case 1:	
						matrix_value.matrix_scan_value = ASCII_CODE[0];
						break;
					
					case 2:	
						matrix_value.matrix_scan_value = ASCII_CODE[11];
//						feedback_single_keycode = false;
						break;
					
					default:
						break;
				}
			}
		}
	}
	else 
	{
		if(matrix_key_typedef.key_r4_press_flag == true)
		{
			delay1ms(MATRIX_KEY_DELAY_50MS);
			if(Gpio_GetInputIO(KEY_R4_PORT,KEY_R4_PIN) == true)
			{
				matrix_key_typedef.key_r4_press_flag = false;
				system_key_flag.keypad_scan_count_flag = false;
			}
		}
	}

	if(system_key_flag.keypad_scan_count_flag == false)
	{
		if(key_count.matrix_key_scan_count < 2)
		{
			key_count.matrix_key_scan_count++;
		}
		else
		{
			key_count.matrix_key_scan_count = 0;
			return;
		}
	}
}


