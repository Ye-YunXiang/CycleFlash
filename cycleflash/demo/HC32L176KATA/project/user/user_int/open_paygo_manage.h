/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	open_paygo_manage.h
* Description:	Manage the Open PAYGO logic
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#ifndef __OPEN_PAYGO_MANAGE_H_
#define __OPEN_PAYGO_MANAGE_H_

#include "main_struct_typedef.h"
#include "main_file_define.h"

// openPAYGO 核心文件
#include "opaygo_core_extended.h"
#include "opaygo_decoder_extended.h"
#include "opaygo_core.h"
#include "siphash.h"
#include "opaygo_decoder.h"
#include "opaygo_value_utils.h"
#include "restricted_digit_set_mode.h"


#define NO_ENTER_TOKEN_VALUE			0xFFFFFFFFFFFFFFFF

#define LOCKING_TIME   60


// 管理内部所有的结构体
typedef struct
{
    __flash_lock * _this_lock_pointer;
    // 第一次时间，共可用时间
    __flash_date_one * _this_date_one_pointer;
    // 间隔存储时间位置，间隔存储时间信号，间隔存储时间
    __flash_date_tow * _this_date_tow_pointer;
    // 累计时间，count
    __flash_product_time * _this_product_time_pointer;
    // 密钥，starting code, ID
    __flash_product_id * _this_product_id_pointer;
}__this;

extern __this _this_pointer;

typedef struct
{
    _Bool paygo_id;           // 有没有PAYGO  ID
    _Bool paygo_enabled;      // 有没有在计时
    _Bool paygo_unlock;       // PAYGO 有没有 unlock
}__system_paygo_state;

extern __system_paygo_state _system_paygo_state;


//  初始化所有PAYGO存储结构体以及内存
void op_manage_init_all_PAYGO_value(void);


// 启动流程1！！！加载所有存储在flash中的变量
void op_manage_load_activation_variables(void);

// 启动流程2！！！唤醒校准 paygo 时间
void op_manage_rouse_calibration_time_and_display(uint8_t flow);
// 启动流程2.1 ！！！对RTC坏掉的情况进行判断
void op_manage_rtc_error_manage(void);
// 启动流程2.2 ！！！初始化中断时间
void op_manage_rouse_irq_time_init(void);

// 启动流程3 ！！！验证版本号
void op_manage_verify_version_numbe(const uint8_t *temp_version_number, _Bool temp_falg);

// 运行的显示状态和时间管理
void op_manage_run_display_state_and_time(void);
// 验证Token，检测是否为测试码或者要解码
void op_manage_recognition_input_token(uint32_t Input_Token_Data);

// PAYGO信用量状态管理
void op_manager_paygo_credit_and_workqueue(void);

// 存入间接时间
// 必须是在计时状态，并且没有unlock的情况下才能存
void op_manage_write_date_tow(void * struct_data);

// 存入ID
void op_manage_set_flash_id(uint32_t * temp_scid, uint32_t * temp_suid, char * temp_skid, char * temp_pkid, char * temp_dsid);

// 密钥解析
int op_manage_product_get_openpaygo_activation_value(uint64_t input_token, uint16_t * temp_count);

// 对解析出来的密钥进行判断
void op_manage_product_opepaygo_token_judge_result(int temp_credit_remaining, uint16_t temp_count);

// 添加天数
void op_manage_add_activation_time(int activation_value);

// 设置覆盖天数
void op_manage_set_activation_time(int activation_value);

// 判断PAYGO有无额度，用于打开M25发信息
_Bool op_manage_PAYGO_do_odd_times(void);

// 后盖打开显示管理
void op_manage_tamper_open_manage(void);


// 上锁管理，成功返回1，失败返回0 
// flag置0，为输入错误，true为验证是否上锁
_Bool op_manage_lock_control(_Bool temp_flag);


// 给出MQTT的数据
void op_manage_get_mqtt_param(void * temp_param);

#endif
