#ifndef __SYSTEM_MOTOR_CONTROL_H_
#define __SYSTEM_MOTOR_CONTROL_H_

#include "main_file_define.h"

// 开冰箱
#define SYSTEM_REFRIGERATOR_ENABLE()	Gpio_SetIO(REFRIGERATOR_PORT,REFRIGERATOR_PIN)
// 关冰箱
#define SYSTEM_REFRIGERATOR_DISABLE() Gpio_ClrIO(REFRIGERATOR_PORT,REFRIGERATOR_PIN)

// 初始化冰箱控制
void system_refrigerator_control_init(void);

// 打开冰箱
void system_refrigerator_control_open(void);

// 关闭冰箱
void system_refrigerator_control_shut(void);


#endif
