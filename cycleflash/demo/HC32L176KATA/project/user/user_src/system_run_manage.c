/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_run_manage.c
* Description: 	Managing system running status
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "system_run_manage.h"
#include "system_ota.h"

// 判断和接收UART的ID是否有成功
_Bool judge_uart_receive_id(void * data_t)
{
	char log[50] = {0};
	//-------------------------------------------------------------------------------------
	//- 校验接收 
	//-------------------------------------------------------------------------------------
	int i = 0;
	uint16_t CRC = 0;
	char array_crc[2] = {0};
	// 数组从0开始，所以要后4位CRC要加上1
	char * source_data = &UART0_Fram_Record.Data_RX_BUF[UART0_Fram_Record.InfBit.FramLength - sizeof(array_crc)*2];
	// CRC提取到数组
    array_to_hex_bit(&array_crc[0], sizeof(array_crc), source_data);
	// 得到CRC
	for(i = 0; i < sizeof(array_crc); i++)
	{
		CRC = (CRC * 0x100) + (array_crc[i] & 0xff);
	}
	uint16_t CRC_File = CRC_16_XMODEM(0,0x1021,0,false,false, (uint8_t *)&UART0_Fram_Record.Data_RX_BUF[0], (UART0_Fram_Record.InfBit.FramLength - CRCID));
	
	sprintf(log,"CRC: %X    CRC_File: %X ", CRC, CRC_File);
	system_ble_uart0_send_string(&log[0], strlen(log));
	
	// 如果校验码不对，直接退出
	if(CRC != CRC_File)
	{
		return FALSE;
	}


	//-------------------------------------------------------------------------------------
	//- 开始接收 
	//-------------------------------------------------------------------------------------
	struct OpenpayGo_ID * op_id_t = (struct OpenpayGo_ID *)data_t;
	uint8_t Uart_Receive_Type = Uart_Receive_Type0;
	uint8_t Uart_Receive_Count = 0;		// 接收计数
	uint8_t Uart_Receive_All_Count = 0;	// 一共接收计数

	// 接收数组
	while(Uart_Receive_Type != Uart_Receive_Type_Succeed \
			&& Uart_Receive_All_Count <= UART0_Fram_Record.InfBit.FramLength \
			&& Uart_Receive_Type != Uart_Receive_Type_Failure)
	{
		if(Uart_Receive_Type == Uart_Receive_Type0)
		{
			if(Uart_Receive_Count <= SNID)	// 接收的序列号bit长度小于 9
			{
				// op_id_t.SNID_Receive_Temp[uart_tx_t.Uart_Receive_SN_Bits] = UartReceive_ID_Data;
			
				switch ( UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count] )
				{
					case 0x3B:          //receive ";"
						Uart_Receive_Type = Uart_Receive_Type1;
						Uart_Receive_Count = 0;
						break;
					default:
						Uart_Receive_Count++;
						break;
				}
			}
			else
			{
				Uart_Receive_Type = Uart_Receive_Type_Failure;
			}
		}
		else if(Uart_Receive_Type == Uart_Receive_Type1)
		{
			if(Uart_Receive_Count <= SCID)			// 接收的序列号bit长度小于 10
			{
				if(UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count] != 0x3B)
				{
					op_id_t->SCID_Receive_Temp[Uart_Receive_Count] = UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count];
				}

				switch (UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count])
				{
					case 0x3B:          //receive ";"
						Uart_Receive_Type = Uart_Receive_Type2;
						Uart_Receive_Count = 0;
						break;
					default:
						Uart_Receive_Count++;
						break;
				}
			}
			else
			{
				Uart_Receive_Type = Uart_Receive_Type_Failure;
			}
		}
		else if(Uart_Receive_Type == Uart_Receive_Type2)
		{
			if(Uart_Receive_Count <= SUID)		// 接收的序列号bit长度小于 10
			{ 
				if(UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count] != 0x3B)
				{
					op_id_t->SUID_Receive_Temp[Uart_Receive_Count] = UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count];
				}

				switch (UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count])
				{
					case 0x3B:          //receive ";"
						Uart_Receive_Type = Uart_Receive_Type3;
						Uart_Receive_Count = 0;
						break;
					default:
						Uart_Receive_Count++;
						break;
				}
			}
			else
			{
				Uart_Receive_Type = Uart_Receive_Type_Failure;
			}
		}
		else if(Uart_Receive_Type == Uart_Receive_Type3)
		{
			if(Uart_Receive_Count <= SKID)		// 接收的序列号bit长度小于 10
			{ 
				if(UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count] != 0x3B)
				{
					op_id_t->SKID_Receive_Temp[Uart_Receive_Count] = UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count];
				}

				switch (UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count])
				{
					case 0x3B:          //receive ";"
						Uart_Receive_Type = Uart_Receive_Type4;
						Uart_Receive_Count = 0;
						break;
					default:
						Uart_Receive_Count++;
						break;
				}
			}
			else
			{
				Uart_Receive_Type = Uart_Receive_Type_Failure;
			}
		}
		else if(Uart_Receive_Type == Uart_Receive_Type4)
		{
			if(Uart_Receive_Count <= PKID)		// 接收的序列号bit长度小于 10
			{ 
				if(UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count] != 0x3B)
				{
					op_id_t->PKID_Receive_Temp[Uart_Receive_Count] = UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count];
				}

				switch (UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count])
				{
					case 0x3B:          //receive ";"
						Uart_Receive_Type = Uart_Receive_Type5;
						Uart_Receive_Count = 0;
						break;
					default:
						Uart_Receive_Count++;
						break;
				}
			}
			else
			{
				Uart_Receive_Type = Uart_Receive_Type_Failure;
			}
		}
		else if(Uart_Receive_Type == Uart_Receive_Type5)
		{
			if(Uart_Receive_Count <= DSID)  	// 接收的序列号bit长度小于 32
			{
				if(UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count] != 0x3B)
				{
					op_id_t->DSID_Receive_Temp[Uart_Receive_Count] = UART0_Fram_Record.Data_RX_BUF[Uart_Receive_All_Count];
				}

				if(Uart_Receive_Count >= DSID)
				{
					// 接收成功
					Uart_Receive_Type = Uart_Receive_Type_Succeed;
				}
				else
				{
					Uart_Receive_Count++;
				}
			}
			else
			{
				Uart_Receive_Type = Uart_Receive_Type_Failure;
			}
		}
		else 
		{
			// 接收失败
			Uart_Receive_Type = Uart_Receive_Type_Failure;
		}

		// 接收的所有计数
		Uart_Receive_All_Count++;
	}


	if(Uart_Receive_Type == Uart_Receive_Type_Succeed)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


// 烧录ID流程管理
void system_input_ID_manage(void)
{
	if(_system_paygo_state.paygo_id != TRUE  && UART0_Fram_Record.InfBit.FramFinishFlag == 1 \
										&& UART0_Fram_Record.InfBit.FramLength >= 10 \
										&& UART0_Fram_Record.Data_RX_BUF != NULL)
	{
		RED_SATE_OPEN();
		GREEN_SATE_OPEN();

		struct OpenpayGo_ID openpaygo_id_t;
		memset(&openpaygo_id_t, 0, sizeof(struct OpenpayGo_ID));

		_Bool flag_t = judge_uart_receive_id(&openpaygo_id_t);

		if(flag_t == TRUE)
		{
			// 返回接收到的ID
			returns_the_uart_receive_id(&openpaygo_id_t, TRUE);

			uint32_t scid = array_to_int32_bit(&openpaygo_id_t.SCID_Receive_Temp[0], SCID);
			
			uint32_t suid = array_to_int32_bit(&openpaygo_id_t.SUID_Receive_Temp[0], SUID);

			// 存入ID, 并初始化内存
			op_manage_set_flash_id(&scid, &suid, &openpaygo_id_t.SKID_Receive_Temp[0], &openpaygo_id_t.PKID_Receive_Temp[0], &openpaygo_id_t.DSID_Receive_Temp[0]);

			RED_SATE_CLOSE();
			GREEN_SATE_CLOSE();

			// 关闭UART0功能口
			system_uart0_config_off();

			// 初始化内存以及清除测试的PAYGO
			op_manage_init_all_PAYGO_value();

			// 运行的显示状态和时间管理
    		op_manage_run_display_state_and_time();
		}
		else
		{
			// 返回接收失败
			returns_the_uart_receive_id(NULL, FALSE);

			memset(&UART0_Fram_Record.Data_RX_BUF[0], 0, UART0_Fram_Record.InfBit.FramLength);
			UART0_Fram_Record.InfBit.FramLength = 0;
			UART0_Fram_Record.InfBit.FramFinishFlag = 0;
		}
	}
}


/********************************************************************
 ** \brief  返回接收到的ID
 ** @param  struct OpenpayGo_ID指针
 ** @param  _Bool 是否接收成功返回
 ** \retval 无
********************************************************************/
void returns_the_uart_receive_id(void * pointer, _Bool RECEIVE_DONE)
{
	char UART_SCID[] = {0X53, 0X43, 0X49, 0X44, 0X3A};
	char UART_SUID[] = {0X53, 0X55, 0X49, 0X44, 0X3A};
	char UART_SKID[] = {0X53, 0X4B, 0X49, 0X44, 0X3A};
	char UART_PKID[] = {0X50, 0X4B, 0X49, 0X44, 0X3A};
	char UART_DSID[] = {0X44, 0X53, 0X49, 0X44, 0X3A};

	char R_N[] = {0x0D, 0x0A};	// \r\n

	if(RECEIVE_DONE == TRUE)
	{
		struct OpenpayGo_ID * OPG_ID_data = (struct OpenpayGo_ID *)pointer;

		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));

		// SCID返回
		system_ble_uart0_send_string(&UART_SCID[0], sizeof(UART_SCID));
		system_ble_uart0_send_string(&OPG_ID_data->SCID_Receive_Temp[0], sizeof(OPG_ID_data->SCID_Receive_Temp));
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));

		// SUID返回
		system_ble_uart0_send_string(&UART_SUID[0], sizeof(UART_SUID));
		system_ble_uart0_send_string(&OPG_ID_data->SUID_Receive_Temp[0], sizeof(OPG_ID_data->SUID_Receive_Temp));
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));

		// SKID返回
		system_ble_uart0_send_string(&UART_SKID[0], sizeof(UART_SKID));
		system_ble_uart0_send_string(&OPG_ID_data->SKID_Receive_Temp[0], sizeof(OPG_ID_data->SKID_Receive_Temp));
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));

		// PKID返回
		system_ble_uart0_send_string(&UART_PKID[0], sizeof(UART_PKID));
		system_ble_uart0_send_string(&OPG_ID_data->PKID_Receive_Temp[0], sizeof(OPG_ID_data->PKID_Receive_Temp));
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));

		// DSID返回
		system_ble_uart0_send_string(&UART_DSID[0], sizeof(UART_DSID));
		system_ble_uart0_send_string(&OPG_ID_data->DSID_Receive_Temp[0], sizeof(OPG_ID_data->DSID_Receive_Temp));
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));
	}
	else
	{
		char failure_reply[] = {'F', 'a', 'i', 'l', 'e', 'd', ',', ' ', 'p', 'l', 'e', 'a', 's', 'e', ' ', 'r', 'e', 's', 'e', 'n', 'd', '!'};
		// 接收失败
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));
		system_ble_uart0_send_string(&failure_reply[0], sizeof(failure_reply));
		system_ble_uart0_send_string(&R_N[0], sizeof(R_N));
	}
}


/********************************************************************
 ** \brief  把数组的每两位转换为1个HEX
 ** @param  char 数组指针，接收的HEX数组
 ** @param  uint8_t 数组长度，接收的HEX数组
 ** @param  char 源数组长度，源数据
 ** \retval 无
********************************************************************/
void array_to_hex_bit(char * Array_data, uint8_t Array_len, char * temp_skid)
{
	uint8_t i = 0;
	uint8_t j = 0;
 	uint8_t temp_skid_byte = 0;

    // 把32个hex转化为16个hex
    for( i = 0; i < Array_len; i++)
    {
        for(j = 0; j < 2; j++)
        {
            if(*temp_skid >= '0' && *temp_skid <= '9')
            {
                *temp_skid -= 0x30;
            }
            else if(*temp_skid >= 'a' && *temp_skid <= 'z')
            {
                *temp_skid -= 0x57;
            }
            else if(*temp_skid >= 'A' && *temp_skid <= 'Z')
            {
                *temp_skid -= 0x37;
            }

            if(j == 0)
            {
                temp_skid_byte = *temp_skid * 0x10;
            }
            else
            {
                temp_skid_byte += *temp_skid;
            }

            temp_skid++;
        }

        *Array_data = temp_skid_byte;
		Array_data++;
    }
}


/********************************************************************
 ** \brief  把数组转换为一串数字
 ** @param  char数组指针
 ** @param  char数组长度
 ** \retval 无
********************************************************************/
uint64_t array_to_int32_bit(char * Array_data, uint8_t Array_len)
{
	uint8_t i;
	uint64_t result = 0;

	for ( i = 0; i < Array_len; i++)
	{
		if (*Array_data <= 9) 
		{
            result = result * 10 + *Array_data;
		}
		else if(*Array_data >= '0' && *Array_data <= '9') 
		{
			result = result * 10 + (*Array_data - '0');
		}

		Array_data++;
    }

	return result;
}


/********************************************************************
 ** \brief  获取输入键盘值
 ** \retval 无
********************************************************************/
int get_key_pressed(void) 
{
	system_martix_key_scan();		// 扫描键盘呀

	unsigned char this_char = matrix_value.matrix_scan_value;	// 得到矩阵扫描出来的结果

	switch (this_char) {
        case '*':
					return STAR_KEY;

        case '#':
					return HASH_KEY;

        case 0x00 :
					return INVALID_KEY;

        default:
					return ((char) (this_char - '0')); // this_char is now an int
	}
}


/********************************************************************
 ** \brief  等待按键输入
 ** \retval 无
********************************************************************/
uint64_t wait_for_token_entry(void)			
{
    // 获得键盘输入值以及返回的密钥
    int last_key = 0;
	uint64_t temp_token = NO_ENTER_TOKEN_VALUE;            
	                       
    // 有无键盘输入值判断，键盘输入时间限制
    _Bool no_pressed_token = 0;
    uint8_t keypad_free_time_count = 0; 

    // 键盘输入的个数，键盘输入值记录
    uint8_t keypad_pressed_num = 0;
    char input_open_paygo_token[TOKEN_LENGTH] = {0};

	// 用于标志是否输入完毕
	_Bool keypad_pressed_input_sure_token = FALSE;

	if(op_manage_lock_control(TRUE) && ((STAR_KEY == get_key_pressed()) && (no_pressed_token == FALSE)))
	{
		no_pressed_token = TRUE;
	}
	else if((STAR_KEY == get_key_pressed()) && !op_manage_lock_control(TRUE))		//是否上锁有输入
	{
		open_paygo_display_startup(DISPLAY_KEYBOARD_LOCK);
		display_state_manage.display_state_flag = TRUE;

        if(_system_paygo_state.paygo_enabled && !_system_paygo_state.paygo_unlock)
        {
            display_state_manage.display_state_activity = TRUE;     // 显示数字标记
        }

		matrix_value.matrix_scan_value = 0;
	}
	else if(HASH_KEY != get_key_pressed() && (STAR_KEY != get_key_pressed()) && (0x00 != matrix_value.matrix_scan_value))
	{
        irq_time.lcd_light_up_time = 0;
		matrix_value.matrix_scan_value = 0;
	}

    // 只要输入*号，就一直在里面不断的获取扫描结果，直到输入#，或者超时
	while(no_pressed_token)			
	{
		last_key = get_key_pressed();

		switch (last_key)
		{
			case STAR_KEY:

				if(keypad_pressed_keycode_allow)
				{
					GREEN_SATE_OPEN();

					keypad_pressed_keycode_allow = FALSE;

					// 用户输入时间归零，准备计时5秒钟，没有继续输入就结束
					keypad_free_time_count = 0;		
					irq_time.lcd_light_up_time = 0;
					matrix_value.matrix_scan_value = 0;		// 清空矩阵扫描结果

					// 清空输入个数以及存储token的数组
					keypad_pressed_num = 0;
					memset(input_open_paygo_token, 0x00, sizeof(input_open_paygo_token));

					// 显示
					open_paygo_token_data_display('*', keypad_pressed_num);

					GREEN_SATE_CLOSE();
				}
				break;

			case HASH_KEY:

				GREEN_SATE_OPEN();

				keypad_pressed_keycode_allow = FALSE;
				no_pressed_token = FALSE;
				keypad_pressed_input_sure_token = TRUE;
				keypad_free_time_count = 0;
				matrix_value.matrix_scan_value = 0;	

				open_paygo_token_data_display('#', keypad_pressed_num);
				delay1ms(1000);
				
				GREEN_SATE_CLOSE();

				break;

			case INVALID_KEY:

				matrix_value.matrix_scan_value = 0;
				break;
			
			default:

				// 是不是有输入信号
				if(keypad_pressed_keycode_allow)
				{
					GREEN_SATE_OPEN();

					// 清空用户输入时间计数
					keypad_free_time_count = 0;
					// 清空亮屏时间，打开亮屏
					irq_time.lcd_light_up_time = 0;

					// 得到输入的数字
					input_open_paygo_token[keypad_pressed_num] = get_key_pressed();

					open_paygo_token_data_display(input_open_paygo_token[keypad_pressed_num], keypad_pressed_num);
					
					keypad_pressed_num++;

					matrix_value.matrix_scan_value = 0;
					keypad_pressed_keycode_allow = FALSE;

					GREEN_SATE_CLOSE();
				}
				break;
		}

		// 红灯不开
		RED_SATE_CLOSE();

		if(irq_time.lptimer_1s_flag)
		{
			keypad_free_time_count++;

			// 距离按过按键大于5秒就退出输入状态
			if(keypad_free_time_count > 5)			
			{
				no_pressed_token = FALSE;					// 设置输入超时，退出循环

				irq_time.lcd_light_up_time = 0;				// 清空亮屏时间
				keypad_free_time_count = 0;					// 清空输入超时计时

				matrix_value.matrix_scan_value = 0;
				keypad_pressed_input_sure_token = TRUE;		// 设置有输入
				//temp_token = NO_ENTER_TOKEN_VALUE;
			}
			irq_time.lptimer_1s_flag = FALSE;
		}

		// 关机和后盖打开期间不许输入
		if(__sleep_mode.device_power_on_state == FALSE || __sleep_mode.tamper_open == TRUE)
		{
			break;
		}
	}

	// 输入密钥完毕
	//	这一段虽然可以嵌入上面，用更简单的方式实现，为了方便阅读和预防变量超出类型范围，所以放最后吧。
	if(keypad_pressed_input_sure_token == TRUE)	 
	{
		// 拿到输入的密钥
		temp_token = array_to_int32_bit(&input_open_paygo_token[0], TOKEN_LENGTH);
		return temp_token;
	}

	return temp_token;
}


/********************************************************************
 ** \brief  运行控制
 ** \retval 无
********************************************************************/
void system_console_run_manage(void)
{
	// 关机和后盖打开期间不许输入
	if(__sleep_mode.device_power_on_state != FALSE || __sleep_mode.tamper_open != TRUE)
	{
		uint32_t input_token;

		input_token = wait_for_token_entry();

		op_manage_recognition_input_token(input_token);
	}

	// 显示时间计算标志位和管理
	op_manage_run_display_state_and_time();

	// PEYGO 状态管理和存储
	op_manager_paygo_credit_and_workqueue();

	// 烧录ID管理
	system_input_ID_manage();
}



