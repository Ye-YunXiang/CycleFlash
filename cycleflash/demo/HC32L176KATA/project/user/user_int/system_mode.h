#ifndef _SYSTEM_MODE_H_
#define _SYSTEM_MODE_H_

#include "main_file_define.h"

#include "lpm.h"

enum SLEEP_MODE_ENUM
{
    _SYSTEM_BOOT_SCREEN,    // 系统启动屏幕
    _SYSTEM_BOOT_RTC,       // 系统启动RTC
    _SYSTEM_BOOT_VERIFY,    // 系统启动验证
    _SYSTEM_RUN,            // 系统运行
    _SYSTEM_SLEEP,          // 系统休眠
    _SYSTEM_SLEEP_ROUSE,     // 系统睡眠唤醒

    _SYSTEM_TAMPER_OPEN     // 后盖被打开
};

struct SLEEP_MODE_STRUCT
{
    // 设备状态:TRUE 上电，FALSE 没上电
    _Bool device_power_on_state;
    // 运行状态
    uint8_t run_state;
	// 电平检测状态翻转
	_Bool state_flip_5v;
    // 后盖被打开   1 后盖被打开。
    _Bool tamper_open;
    // MQTT是否上报
    _Bool mqtt_tamper_open;

};

extern struct SLEEP_MODE_STRUCT	__sleep_mode;


// 系统所有外设初始化
void system_mode_peripherals_init(void);

// id烧录口是否打开
void system_mode_ID_uart_select_open(void);

// 唤醒后m25上报初始化
void system_mode_rouse_reported_init(void);

// 系统唤醒或睡眠流程,TRUE为开机流程，FALSE为全局状态
void system_mode_wakeUP_or_sleep_prrocess(_Bool state);

//  开机初始化所有引脚
void system_mode_IO_init(void);

//  进入睡眠设置引脚
void system_mode_sleep_IO_init(void);

// 关机设置
void system_mode_power_off_set(void);

// 判断进入深度睡眠
void system_mode_judge_depp_sleep_open(void);

#endif
