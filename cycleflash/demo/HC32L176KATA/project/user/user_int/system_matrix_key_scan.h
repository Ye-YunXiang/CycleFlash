/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_matrix_key.c
* Description: 	Scan the keyboard for 'matrix_value' and 'keypad_pressed_keycode_allow' signals
* 
* Author:		Soild_Alasong
* Date: 	 	2022   /9/01
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Alasong create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128ҳÿҳ512�ֽ�
******************************************************************************/
#ifndef __SYSTEM_MATRIX_KEY_SCAN_H
#define __SYSTEM_MATRIX_KEY_SCAN_H

#include "main_file_define.h"

typedef struct
{
	uint8_t matrix_scan_value;
}matrix_typeedef;

extern volatile matrix_typeedef	 matrix_value;

extern bool keypad_pressed_keycode_allow;

void system_matrix_key_scan_init(void);
void system_martix_key_scan(void);

#endif

