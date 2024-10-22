#include "system_motor_control.h"

_Bool refrigerator_open_or_shut = FALSE;

// 初始化冰箱控制
void system_refrigerator_control_init(void)
{
	stc_gpio_cfg_t 	stc_out_motor_init;
	
	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
	stc_out_motor_init.enDir			= GpioDirOut;
	stc_out_motor_init.enDrv			= GpioDrvL;
	stc_out_motor_init.enPu 			= GpioPuDisable;
	stc_out_motor_init.enPd 			= GpioPdDisable;
	stc_out_motor_init.enCtrlMode		= GpioAHB;
	
	Gpio_Init(REFRIGERATOR_PORT, REFRIGERATOR_PIN, &stc_out_motor_init);
    // 关闭冰箱开关
    SYSTEM_REFRIGERATOR_DISABLE();

    refrigerator_open_or_shut = FALSE;
}

// 打开冰箱
void system_refrigerator_control_open(void)
{
    if(refrigerator_open_or_shut != TRUE)
    {
         // 打开冰箱开关
        SYSTEM_REFRIGERATOR_ENABLE();
        refrigerator_open_or_shut = TRUE;
    }
}


// 关闭冰箱
void system_refrigerator_control_shut(void)
{
    if(refrigerator_open_or_shut != FALSE)
    {
         // 关闭冰箱开关
        SYSTEM_REFRIGERATOR_DISABLE();
        refrigerator_open_or_shut = FALSE;
    }
}
