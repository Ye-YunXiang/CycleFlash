/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	open_paygo_manage.c 
* Description:	Manage the Open PAYGO logic
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "open_paygo_manage.h"
#include "system_ota.h"

__flash_date_tow INIT_THIS_DATE_TOW = {0,{255,255,255,255,255,255,255}};


// PAYGO状态初始化
__system_paygo_state _system_paygo_state;

// 是否上锁的结构体
__flash_lock _this_lock = {0, 0};
// 第一次时间，共可用时间
__flash_date_one _this_date_one = {0, 0};
// 间隔存储时间位置，间隔存储时间信号，间隔存储时间
__flash_date_tow _this_date_tow = {0, {255,255,255,255,255,255,255}};
// 累计时间，count
__flash_product_time _this_product_time = {0,0};

// 密钥，starting code, ID
__flash_product_id _this_product_id;



__this _this_pointer = {  &_this_lock,\
                                &_this_date_one,\
                                &_this_date_tow,\
                                &_this_product_time,\
                                &_this_product_id};

const uint64_t Openpaygo_Test_Token[5] = {800100000,800200000,800300000,888880000,888890000};


/********************************************************************
 ** @brief  初始化所有PAYGO存储结构体以及内存
 ** @param  无
********************************************************************/
void op_manage_init_all_PAYGO_value(void)
{
    // 初始化结构体
	memset(&_this_lock, 0, sizeof(__flash_lock));
	memset(&_this_date_one, 0, sizeof(__flash_date_one));
	
	_this_date_tow.interval_date = 0;
	memset(&_this_date_tow.location_date[0], 255, sizeof(_this_date_tow.location_date));
	
	memset(&_this_product_time, 0, sizeof(__flash_product_time));
	
    // 初始化内存
    open_paygo_flash_close_flash(CLOSE_FLASH_LOCK);
    open_paygo_flash_close_flash(CLOSE_FLASH_DATE_ONE);
    open_paygo_flash_close_flash(CLOSE_FLASH_DATE_TOW);
    open_paygo_flash_close_flash(CLOSE_FLASH_PRODUCT_TIME);

    // 初始显示管理结构体
    open_paygo_display_struct();		
}


// 启动流程1！！！！！！！！！！
/********************************************************************
 ** @brief  加载flash中所有激活变量
 ** @param  无
 ** @retval 无
********************************************************************/
void op_manage_load_activation_variables(void) 
{
	memset(&_this_product_id, 0, sizeof(__flash_product_time));
	
    open_paygo_flash_read_lock_sate(&_this_lock);

    if(open_paygo_flash_read_date_one(&_this_date_one) != FALSE)
    {
        open_paygo_flash_read_date_tow(&_this_date_tow);

        // _system_paygo_state.paygo_enabled = TRUE;
    }

    open_paygo_flash_read_product_time(&_this_product_time);

    if(_this_product_time.add_time >= 998)
    {
        _system_paygo_state.paygo_unlock = TRUE;   // unlock

        _system_paygo_state.paygo_enabled = FALSE;  // 停止计时
    }
    else
    {
        _system_paygo_state.paygo_unlock = FALSE;
    }

    if(open_paygo_flash_read_product_id(&_this_product_id) != FALSE)
    {
        _system_paygo_state.paygo_id = TRUE;
    }
    else
    {
        _system_paygo_state.paygo_id = FALSE;

        // _system_paygo_state.paygo_enabled = FALSE;

        _system_paygo_state.paygo_unlock = FALSE;
    }
}

// 启动流程2！！！！！
/********************************************************************
 ** @brief  唤醒后初始化时间，并验证RTC是否正常，以及显示屏幕
 ** @param  无
 ** @retval 无
********************************************************************/
void op_manage_rouse_calibration_time_and_display(uint8_t flow)
{
    uint8_t i ;
    uint8_t temp_rtc_state;

    if(flow == 0)   // 启动屏幕
    {
        rtc_state.rtc_starting_test = TRUE;

        open_paygo_display_startup(DISPLAY_SLOGAN);

        // PAYGO 还有额度
        if(op_manage_PAYGO_do_odd_times() == TRUE)
        {
            uint32_t temp_time;

            uint8_t time_sate = starting_run_examine_get_sec(&temp_time, FALSE);

            if(!time_sate || temp_time < _this_date_tow.interval_date)
            {
                rtc_time_t timer_date_temp;

                second_conversion_date(&timer_date_temp, _this_date_tow.interval_date);

                example_set_rtc_time(&timer_date_temp);
            }

            // 允许MQTT进行循环发送信息
            //mqtt_state.mqtt_cyclic_send = TRUE;
            // 发送消息,打开GSM
            if(mqtt_state.mqtt_task_state == MQTT_TYPE_NULL || gsm_power_sending_time_count == 1 || gsm_power_sending_time_count == 0)
            {
                mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_ON;
            }
        }
            
        irq_time.lptimer_2s_flag = 0;
    }
    else if(flow == 1)  // 启动RTC
    {
        open_paygo_display_startup(DISPLAY_UP_LOADING);

        // uart初始化过专用，就不要重复初始化
        if(UART0_Fram_Record.set_up != TRUE || __sleep_mode.device_power_on_state == FALSE)
        {
            TIMER_5ms_ON(); // 打开5ms定时器 为功耗考虑
        }

        i = 0;
        temp_rtc_state = 0;

        // i 每次加一，就是250ms， 所以下面 18 * 0.25s = 4.5s
        while(i !=  18)
        {
            if(temp_rtc_state == 0 && !_system_paygo_state.paygo_unlock)
            {
                temp_rtc_state = open_paygo_time_starting_run_examine();            // 验证成功的话就会给lptime计数赋值
            }

            if(i == 0 || i ==1 || i == 4 || i == 5 || i == 8 || i == 9\
                                        || i == 12 || i == 14 || i == 16)
            {
                GREEN_SATE_OPEN();
            }
            else
            {
                GREEN_SATE_CLOSE();
            }

            if(irq_time.timer_250ms_flag == 1)
            {
                i++;

                irq_time.timer_250ms_flag = 0;
                irq_time.timer_5_ms_count = 0;
            }
			
			if(__sleep_mode.device_power_on_state != TRUE && __sleep_mode.run_state != _SYSTEM_SLEEP_ROUSE)
			{
				break;
			}
        }
        
        // uart初始化过专用，就不要关闭
        if(UART0_Fram_Record.set_up != TRUE || __sleep_mode.device_power_on_state == FALSE)
        {
            TIMER_5ms_OFF();     // 为功耗考虑
        }
    }
    else if(flow == 2)  // 启动检测
    {
        // 启动流程2.1 ！！！ 判断RTC状态，决定是否继续往下走
        op_manage_rtc_error_manage();
        
        // 启动流程2.2 ！！！初始化中断验证RTC或者存储的标志位时间
        op_manage_rouse_irq_time_init();  

        // 运行的显示状态和时间管理
        op_manage_run_display_state_and_time();
    }

    // 睡眠模式下不能进入
	if(__sleep_mode.run_state != _SYSTEM_SLEEP_ROUSE)
    {
		// 系统唤醒或睡眠流程
		system_mode_wakeUP_or_sleep_prrocess(TRUE);
    }
}

// 启动流程2.1 ！！
/********************************************************************
 ** @brief  对RTC是否坏掉的情况进行判断
 ** @param  无
 ** @retval 无
********************************************************************/
void op_manage_rtc_error_manage(void)
{
    // RTC状态是坏的，并且PAYGO状态不能是unlock
    if(rtc_state.rtc_run_state == RTC_ERROR && _system_paygo_state.paygo_unlock != TRUE)
    {
        if(_system_paygo_state.paygo_enabled == TRUE)
        {
            // 在运行的过程中RTC坏掉了
            if(irq_time.lcd_light_up_time >= 4)
            {
                open_paygo_display_rtc_error_run_manage();

                irq_time.lcd_light_up_time = 0;
            }
        }
        else
        {
            GREEN_SATE_OPEN();

            RED_SATE_OPEN();
                
            open_paygo_display_startup(DISPLAY_RTC_ERROR);

            // RTC 坏掉了
            while(1)
            {
                if(__sleep_mode.device_power_on_state == FALSE)
                {
                    break;
                }   
            }
        }
    }
}

// 启动流程2.2 ！！！
/********************************************************************
 ** @brief  初始化中断时间，和中断检查RTC and 存储标志位
 ** @param  无
 ** @retval 无
********************************************************************/
void op_manage_rouse_irq_time_init(void)
{
    if(op_manage_PAYGO_do_odd_times() == TRUE)
    {
        uint32_t temp_time_data;

        _system_paygo_state.paygo_enabled = TRUE;

        // 得到距离结束剩多少秒
        temp_time_data = _this_date_one.final_date - irq_time.lptimer_time_count;

        if(temp_time_data >= 1800)          // 大于或等于30分钟
        {
            irq_time.irq_10_min = (temp_time_data % 600) + irq_time.lptimer_time_count;
            irq_time.irq_30_min = (temp_time_data % 1800) + irq_time.lptimer_time_count;

            irq_time.lptimer_test_flag = 0;
            irq_time.lptimer_flash_flag = 0;
        }
        else if(temp_time_data >= 600)      // 大于或等于10分钟
        {
            irq_time.irq_10_min = (temp_time_data % 600) + irq_time.lptimer_time_count;
            irq_time.irq_30_min = _this_date_one.final_date;

            irq_time.lptimer_test_flag = 0;
            irq_time.lptimer_flash_flag = 0;
        }
        else                                // 小于10分钟或者计时完毕
        {
            irq_time.irq_10_min = _this_date_one.final_date;
            irq_time.irq_30_min = _this_date_one.final_date;

            irq_time.lptimer_test_flag = 0;
            irq_time.lptimer_flash_flag = 0;
        }
    }
    else
    {
        _system_paygo_state.paygo_enabled = FALSE;

        // 加了10分钟
        irq_time.irq_10_min = irq_time.lptimer_time_count + 600;

        irq_time.lptimer_test_flag = 0;
        irq_time.lptimer_flash_flag = 0;
    }
}

// 启动流程3 ！！！验证版本号
/********************************************************************
 ** @brief  验证以及替换版本号
 ** @param  版本号
 ** @param  bool 是否重新存入，0为只是验证版本号，1为重新存入信息
 ** @retval 无
********************************************************************/
void op_manage_verify_version_numbe(const uint8_t *temp_version_number, _Bool temp_falg)
{
    if(temp_falg != TRUE)
    {
        if( memcmp(&_this_product_id.version, temp_version_number, sizeof(_this_product_id.version)) != 0) 
        {
            memcpy(&_this_product_id.version, temp_version_number, sizeof(_this_product_id.version));

            _this_product_id.have_reported = 0;
            
            open_paygo_flash_write_product_id(&_this_product_id);   // 重新存入ID

        }
        else if(bootloader_get_handover_performed() == TRUE)
        {
            _this_product_id.have_reported = 0;
            
            open_paygo_flash_write_product_id(&_this_product_id);   // 重新存入ID
        }
    }
    else
    {
        open_paygo_flash_write_product_id(&_this_product_id);   // 重新存入ID
    }
}

/********************************************************************
 ** @brief  往flash存入uart获取的ID
 ** @param  uint32_t starting code
 ** @param  uint32_t unit number code
 ** @param  char secret key
 ** @retval 无
********************************************************************/
void op_manage_set_flash_id(uint32_t * temp_scid, uint32_t * temp_suid, char * temp_skid, char * temp_pkid, char * temp_dsid)
{
    _this_product_id.starting_code = *temp_scid;

    _this_product_id.unit_number_code = *temp_suid;

    // 把32个hex转化为16个hex
    array_to_hex_bit((char *)&_this_product_id.secret_key[0], sizeof(_this_product_id.secret_key), temp_skid);

    memcpy(&_this_product_id.product_key, temp_pkid, sizeof(_this_product_id.product_key));

    memcpy(&_this_product_id.device_secret, temp_dsid, sizeof(_this_product_id.device_secret));

    open_paygo_flash_write_product_id(&_this_product_id);

    // PAYGO状态初始化
    memset(&_system_paygo_state, 0, sizeof(__system_paygo_state));

    _system_paygo_state.paygo_id = TRUE;
}


/********************************************************************
 ** @brief  运行的显示状态和显示时间管理
 ** @param  无
 ** @retval 无
********************************************************************/
void op_manage_run_display_state_and_time(void)
{
    if(_system_paygo_state.paygo_unlock && display_state_manage.display_state_enum != DISPLAY_UNLOCK)
    {
        display_state_manage.display_state_flag = TRUE;

        display_state_manage.display_state_enum = DISPLAY_UNLOCK;
    }
    else if(!_system_paygo_state.paygo_unlock && _system_paygo_state.paygo_enabled && _this_date_one.available_time != 0)
    {

        uint32_t temp_value = _this_date_one.final_date - irq_time.lptimer_time_count;

        uint32_t temp_display_day = temp_value / 86400;

        uint32_t temp_display_hour = (temp_value - temp_display_day * 86400) / 3600;

        if(display_state_manage.display_state_enum != DISPLAY_ACTIVATED_TIME)
        {
            display_state_manage.display_state_flag = TRUE;

            display_state_manage.display_state_activity = TRUE;

            display_state_manage.display_state_enum = DISPLAY_ACTIVATED_TIME;
        }

        if(display_state_manage.display_hour != temp_display_hour || display_state_manage.display_day != temp_display_day)
        {
            display_state_manage.display_state_activity = TRUE;

            display_state_manage.display_day = temp_display_day;

            display_state_manage.display_hour = temp_display_hour;

            // display_state_manage.all_display_min = temp_value / 60;
        }
    }
    else if(!_system_paygo_state.paygo_unlock && _system_paygo_state.paygo_id && display_state_manage.display_state_enum != DISPLAY_NO_PAYGO)   // 没有PYAYGO
    {
        display_state_manage.display_state_activity = FALSE;
        display_state_manage.display_state_flag = TRUE;

        display_state_manage.display_state_enum = DISPLAY_NO_PAYGO;
    }
    else if(!_system_paygo_state.paygo_unlock && !_system_paygo_state.paygo_id && display_state_manage.display_state_enum != DISPLAY_NO_ID)    // 没有ID
    {
        display_state_manage.display_state_activity = FALSE;
        display_state_manage.display_state_flag = TRUE;

        display_state_manage.display_state_enum = DISPLAY_NO_ID;
    }
}


/********************************************************************
 ** @brief  PAYGO信用量状态管理
 ** @param  无
 ** @retval 无
********************************************************************/
void    op_manager_paygo_credit_and_workqueue(void)
{
    // unlock判断
    if(_this_product_time.add_time >= 998 && _system_paygo_state.paygo_enabled)
    {
        _system_paygo_state.paygo_enabled = FALSE;
        _system_paygo_state.paygo_unlock = TRUE;

        memset(&_this_date_one, 0, sizeof(_this_date_one));
		memcpy(&_this_date_tow, &INIT_THIS_DATE_TOW, sizeof(_this_date_tow));

        open_paygo_flash_close_flash(CLOSE_FLASH_DATE_ONE);
        open_paygo_flash_close_flash(CLOSE_FLASH_DATE_TOW);

        GREEN_SATE_CLOSE();
        RED_SATE_CLOSE();

        //mqtt_state.mqtt_cyclic_send = TRUE;
    }

    // 计时结束判断
    if(_this_date_one.final_date <= irq_time.lptimer_time_count && _system_paygo_state.paygo_enabled \
                                                                    && !_system_paygo_state.paygo_unlock)
    {
        _system_paygo_state.paygo_enabled = FALSE;

        memset(&_this_date_one, 0, sizeof(_this_date_one));
		memcpy(&_this_date_tow, &INIT_THIS_DATE_TOW, sizeof(_this_date_tow));

        // 清除
        open_paygo_flash_close_flash(CLOSE_FLASH_DATE_ONE);
        open_paygo_flash_close_flash(CLOSE_FLASH_DATE_TOW);

        GREEN_SATE_CLOSE();
        RED_SATE_CLOSE();

        //mqtt_state.mqtt_cyclic_send = FALSE;
    }

    // 启动后定时器秒数大于内存中活动的秒数，立刻存储一次。
    if(((irq_time.lptimer_time_count - _this_date_tow.interval_date) > 3600) && _system_paygo_state.paygo_enabled \
                                                                                && !_system_paygo_state.paygo_unlock)
    {
        // 存入计时状态下的间接时间
        op_manage_write_date_tow(NULL);

        //mqtt_state.mqtt_cyclic_send = TRUE;
    }

    // 是否开启冰箱判断
    if((_system_paygo_state.paygo_unlock || _system_paygo_state.paygo_enabled) && __sleep_mode.tamper_open != TRUE)
    {
        // 打开冰箱
        system_refrigerator_control_open();
    }
    else
    {
        // 关闭冰箱
        system_refrigerator_control_shut();
    }

    // 验证时间
	open_paygo_time_run_examine();
    // 如果验证时间失败开始验证RTC有没有坏掉
    open_paygo_time_starting_run_examine();
}


// 存入间接时间
// 必须是在计时状态，并且没有unlock的情况下才能存
void op_manage_write_date_tow(void * struct_data)
{
    if(struct_data == NULL)
    {
        if(_system_paygo_state.paygo_enabled && !_system_paygo_state.paygo_unlock)
        {
            _this_date_tow.interval_date = irq_time.lptimer_time_count;

            // 存入间接时间
            open_paygo_flash_write_date_tow(&_this_date_tow);

            //mqtt_state.mqtt_cyclic_send = TRUE;
        }
    }
    else
    {
        // 存入间接时间
        open_paygo_flash_write_date_tow( (__flash_date_tow *)struct_data);
    }
}

 
/********************************************************************
 ** @brief  验证Token，检测是否为测试码或者要解码
 ** @param  uint32_t 键盘输入获取的token
 ** @retval 无
********************************************************************/
void op_manage_recognition_input_token(uint32_t input_token_data)
{
//	uint32_t Read_Openpaygo_Unit_Number;
    
    // 代表有输入
	if(input_token_data != (uint32_t)NO_ENTER_TOKEN_VALUE)
	{
        if(Openpaygo_Test_Token[4] == input_token_data)	    //three hour test code   88889 查count值
		{
            display_state_manage.display_state_token_enum = DISPLAY_TOKEN_COUNT;
        }
 		else if(Openpaygo_Test_Token[3] == input_token_data)			//research device id	 88888 用来查ID。
		{
            display_state_manage.display_state_token_enum = DISPLAY_TOKEN_ID;
		}
		else if(Openpaygo_Test_Token[2] == input_token_data)	//three hour test code   8003 在无ID状态下充值充值3天
		{
            // 没用ID的情况下可以用
            if(!_system_paygo_state.paygo_id)
            {
                uint32_t rtc_green_time;

                starting_run_examine_get_sec(&rtc_green_time, FALSE);

                _this_date_one.final_date = rtc_green_time + (3*24*60*60);
                _this_date_one.available_time = (3*24*60*60);

                _this_date_tow.interval_date = rtc_green_time;

                // 初始化中断时间
                op_manage_rouse_irq_time_init();

                // 存入
                open_paygo_flash_write_date_one(&_this_date_one);
                op_manage_write_date_tow(&_this_date_tow);

                // 打开计时标志
                _system_paygo_state.paygo_enabled = TRUE;
            }
            else
            {
                display_state_manage.display_state_token_enum = DISPLAY_NO_TOKEN;
            }
		}
        else if(Openpaygo_Test_Token[1] == input_token_data)	//three hour test code   8002 在无ID状态下充值充值3小时
		{
            // 没用ID的情况下可以用
            if(!_system_paygo_state.paygo_id)
            {
                uint32_t rtc_green_time;

                starting_run_examine_get_sec(&rtc_green_time, FALSE);

                _this_date_one.final_date = rtc_green_time + (3*60*60);
                _this_date_one.available_time = (3*60*60);

                _this_date_tow.interval_date = rtc_green_time;

                // 初始化中断时间
                op_manage_rouse_irq_time_init();

                // 存入
                open_paygo_flash_write_date_one(&_this_date_one);
                op_manage_write_date_tow(&_this_date_tow);

                // 打开计时标志
                _system_paygo_state.paygo_enabled = TRUE;
            }
            else
            {
                display_state_manage.display_state_token_enum = DISPLAY_NO_TOKEN;
            }
		}
		// 正式要注销 ################################################################################
		else if(Openpaygo_Test_Token[0] == input_token_data)			//research device id	 8001 清除count。##########################################################
		{
            display_state_manage.display_state_token_enum = DISPLAY_TOKEN_COUNT_CLOSE;

            _this_product_time.token_count = 0;

            open_paygo_flash_write_product_time(&_this_product_time); 
		}
		else 													//user input openpaygo token  用户输入OpenPAYGO记号
		{
            int get_credit_remaining = 0;
            uint16_t temp_token_count = _this_product_time.token_count; 				// Token Count

			get_credit_remaining = op_manage_product_get_openpaygo_activation_value(input_token_data, &temp_token_count);

			// 对解析出来的密钥进行判断
            op_manage_product_opepaygo_token_judge_result(get_credit_remaining, temp_token_count);
		}
		
        // 打开显示标记
		display_state_manage.display_state_flag = TRUE;

        if(_system_paygo_state.paygo_enabled && !_system_paygo_state.paygo_unlock)
        {
            display_state_manage.display_state_activity = TRUE;     // 显示数字标记
        }
	}
}


/********************************************************************
 ** @brief  对键盘获取的密钥进行解密，如解密成功就赋值count
 ** @param  uint64_t 输入的密钥
 ** @param  uint16_t open paygo 的 count
 ** @retval 解密出来的天数，-1为无效码
********************************************************************/
int op_manage_product_get_openpaygo_activation_value(uint64_t input_token, uint16_t * temp_count)
{
	int credit_data = 0;

	uint8_t i = 0;

	uint32_t openpaygo_id_starting_code = _this_product_id.starting_code;		// Starting Code
	
	uint8_t openpaygo_id_secret_key[sizeof(_this_product_id.secret_key)];		// Secret Key

    for(i = 0; i < sizeof(_this_product_id.secret_key); i++)
	{
		openpaygo_id_secret_key[i] = _this_product_id.secret_key[i];
	}
	
	__NOP();
	
	// 直接计算OpenPAYGO的解码，解码出活动时间	
	credit_data = GetActivationValueFromToken(input_token, temp_count, openpaygo_id_starting_code, (unsigned char *)openpaygo_id_secret_key);
	
	__NOP();
	
	return credit_data;
}


/********************************************************************
 ** @brief  对解析出来的密钥进行判断
 ** @param  int 解密出来的天数
 ** @param  uint16_t 解密出来的count
 ** @retval 无
********************************************************************/
void op_manage_product_opepaygo_token_judge_result(int temp_credit_remaining, uint16_t temp_count)
{
    switch (temp_credit_remaining)
    {
        case INVALID_TOKEN_VALUE:   // -1
        case PAYGO_ID_VALUE:        // 996
        case PAYGO_COUNT_VALUE:     // 997
        case COUNTER_SYNC_VALUE:    // 999

            display_state_manage.display_state_token_enum = DISPLAY_NO_TOKEN;

            op_manage_lock_control(FALSE);

            break;

        case PAYGO_DISABLE_VALUE:   // 0
           
            display_state_manage.display_state_token_enum = DISPLAY_TOKEN_DISABLE;

            // 重置结构体
            memset(&_this_date_one, 0, sizeof(_this_date_one));
		    memcpy(&_this_date_tow, &INIT_THIS_DATE_TOW, sizeof(_this_date_tow));

            // 清除flash
            open_paygo_flash_close_flash(CLOSE_FLASH_DATE_ONE);
            open_paygo_flash_close_flash(CLOSE_FLASH_DATE_TOW);

            // 清除累计的东西，除了count值
            _this_product_time.add_time = 0;
            _this_product_time.token_count = temp_count;
            open_paygo_flash_write_product_time(&_this_product_time);

            // 重置设备状态
            _system_paygo_state.paygo_enabled = TRUE;
            _system_paygo_state.paygo_unlock = FALSE;

            // 重新验证时间标志
            rtc_state.rtc_starting_test = TRUE;

            break;

        case PAYGO_UNLOCK_VALUE:    // 998

            _system_paygo_state.paygo_unlock = TRUE;

            _this_product_time.add_time = 998;

            _this_product_time.token_count = temp_count;

            open_paygo_flash_write_product_time(&_this_product_time);           
            
            break;

        default:                    // 啥也不是

            if(temp_credit_remaining > PAYGO_DISABLE_VALUE && temp_credit_remaining <= MAX_ACTIVATION_VALUE \
									&& _system_paygo_state.paygo_unlock != TRUE && rtc_state.rtc_run_state == RTC_NORMAL)
            {				
                display_state_manage.display_state_token_enum = DISPLAY_TOKEN_EFFICACIOUS;

                _this_product_time.token_count = temp_count;

                if( _this_product_time.token_count % 2)
                {
                    op_manage_set_activation_time(temp_credit_remaining);       // 奇数为覆盖
                }
                else
                {
                    op_manage_add_activation_time(temp_credit_remaining);       // 偶数为添加
                }

                 _system_paygo_state.paygo_enabled = TRUE;
            }
            else
            {
                display_state_manage.display_state_token_enum = DISPLAY_NO_TOKEN;
            }

            break;
    }

    if(temp_credit_remaining != -1 && _this_lock.lock_error != 0)
    {
        _this_lock.lock_date = 0;

        _this_lock.lock_error = 0;

        open_paygo_flash_close_flash(CLOSE_FLASH_LOCK);
    }
    else if(temp_credit_remaining != -1 && temp_credit_remaining != 996 && \
            temp_credit_remaining != 997 && mqtt_state.mqtt_task_state == MQTT_TYPE_NULL)
    {
        mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_ON;
    }
}


/********************************************************************
 ** @brief  Open PAYGO 活动添加天数
 ** @param  int 要添加的天数
 ** @retval 无
********************************************************************/
void op_manage_add_activation_time(int activation_value)
{	
    _this_date_one.available_time += activation_value * 86400;

    _this_date_one.final_date = (_this_date_one.final_date == 0) ? \
		(irq_time.lptimer_time_count + (activation_value * 86400)) : (_this_date_one.final_date + (activation_value * 86400));

    _this_product_time.add_time = ((_this_product_time.add_time + activation_value) > 998) ? \
                                                998 : (_this_product_time.add_time + activation_value);

    open_paygo_flash_write_date_one(&_this_date_one);

    open_paygo_flash_write_product_time(&_this_product_time);
}


/********************************************************************
 ** @brief  Open PAYGO 活动设置覆盖天数
 ** @param  int 要覆盖的天数
 ** @retval 无
********************************************************************/
void op_manage_set_activation_time(int activation_value)
{
    _this_date_one.available_time = activation_value * 86400;

    _this_date_one.final_date = irq_time.lptimer_time_count + (activation_value * 86400);

    // 累计天数不变

    open_paygo_flash_write_date_one(&_this_date_one);

    open_paygo_flash_write_product_time(&_this_product_time);
}


// 后盖打开显示管理
void op_manage_tamper_open_manage(void)
{
    // 打开后盖
    if(__sleep_mode.tamper_open == TRUE)
    {
        system_lcd1602_display_clear_dispaly();		// 清除屏幕
        System_LCD_Light_Disable();		// 关闭显示
        RED_SATE_CLOSE();				// 关闭红灯
        GREEN_SATE_CLOSE();				// 关闭绿灯

        system_refrigerator_control_shut();     // 关闭冰箱

        display_state_manage.display_state_token_enum = DISPLAY_TAMPER;	// 后盖打开

        // __sleep_mode.mqtt_tamper_open = 1; // 上报打开标志

        __sleep_mode.run_state = _SYSTEM_BOOT_SCREEN;   // 重启系统

        if(mqtt_state.mqtt_task_state == MQTT_TYPE_NULL || gsm_power_sending_time_count <= 1)
        {
            mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_ON;
        }
        
        while(__sleep_mode.tamper_open == TRUE)
        {
            // M25 在发送
            system_gsm_mqtt_manage();		
        }

        // 显示时间计算标志位和管理
        //op_manage_run_display_state_and_time();
    }
}


// 判断PAYGO有无额度，用于打开M25发信息
_Bool op_manage_PAYGO_do_odd_times(void)
{
    if(_this_date_one.available_time != 0 && _this_date_one.available_time != 0xffffffff \
										&& _system_paygo_state.paygo_unlock == FALSE)
	{
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*******************************************************************************
** 上锁管理
 ******************************************************************************/

/********************************************************************
 ** @brief  输入密钥错误上锁管理
 ** @param  _Bool true为已经上锁来验证，false为还没上锁添加错误次数
 ** @retval 未上锁返回1，已上锁返回0
********************************************************************/
_Bool op_manage_lock_control(_Bool temp_flag)
{
    if(temp_flag)
    {
        if(_this_lock.lock_date != 0)
        {
            if(irq_time.lptimer_2s_flag  == 2)
            {
                _this_lock.lock_date -= 2;

                irq_time.lptimer_2s_flag = 0;
            }

            return 0;
        }
        else if(_this_lock.lock_date == 0 && _this_lock.lock_error >= 6)
        {
            _this_lock.lock_date = 0;

            _this_lock.lock_error = 0;

            open_paygo_flash_close_flash(CLOSE_FLASH_LOCK);
        }
    }
    else
    {
        if(_this_lock.lock_error >= 5)       // 错5次
        {
            irq_time.lptimer_2s_flag = 0;

            _this_lock.lock_error = 6;

            _this_lock.lock_date = LOCKING_TIME;

            open_paygo_flash_write_lock_state(&_this_lock, TRUE);

            return 0;
        }
        else
        {
            _this_lock.lock_error++;
            
            open_paygo_flash_write_lock_state(&_this_lock, FALSE);
        } 
    }

    return 1;
}

/*******************************************************************************
** MQTT相关
 ******************************************************************************/

/********************************************************************
 ** @brief  填充MQTT的内容
 ** @param  struct GSM_MQTT_PARAM 
 ** @retval 无
********************************************************************/
void op_manage_get_mqtt_param(void * temp_param)
{
    system_adc_sample_on(); // 检测ADC电压

	delay100us(5);

	struct GSM_MQTT_PARAM * _mqtt_param = (struct GSM_MQTT_PARAM *)(temp_param);

    _mqtt_param->count = _this_product_time.token_count;

    
    // 填充发送时间
    if(!_system_paygo_state.paygo_unlock && _system_paygo_state.paygo_enabled && _this_date_one.available_time != 0)
    {
        _mqtt_param->time_remaining_hour = (_this_date_one.final_date - irq_time.lptimer_time_count) / 60 /60;

        _mqtt_param->time_remaining_min = ((_this_date_one.final_date - irq_time.lptimer_time_count) /60 ) % 60;
    }
    else if(_system_paygo_state.paygo_unlock == TRUE)
    {
        _mqtt_param->time_remaining_hour = 9999;

        _mqtt_param->time_remaining_min = 9999;
    }
    else {
        _mqtt_param->time_remaining_hour = 0;

        _mqtt_param->time_remaining_min = 0;
	}

    // 转换电池电压
    _mqtt_param->BAT_VDC = (float)__ADC.ADC_sample_data[0] / 1000;
    // 转换外部输入电压
    _mqtt_param->DC_VDC = (float)__ADC.ADC_sample_data[1] / 1000;


    // 转化为整数冰箱温度
    _mqtt_param->refrigerator_thermometer = __ADC.ADC_sample_data[2] / 1000;
    if(_mqtt_param->refrigerator_thermometer > (100.0) || _mqtt_param->refrigerator_thermometer < (-50.0))
    {
        _mqtt_param->refrigerator_thermometer = 999.0;
    }

    _mqtt_param->power_state = __sleep_mode.device_power_on_state;
}
