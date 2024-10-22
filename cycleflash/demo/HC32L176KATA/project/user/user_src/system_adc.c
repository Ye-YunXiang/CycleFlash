#include "system_adc.h"


#define T25 298.15    // 25℃下的开尔文温度，也就是298.15K
#define R25 10        // 热敏上拉电阻的阻值
#define B	3435      // 热敏电阻本身系数


// 设备状态、设备电压、设备采集结束符、flash中的温度校准值、ADC采集数据（电池电压、外部电源电压、冰箱温度）
struct ADC_SAMPLE_STRUCT __ADC = {0, 0, 0, {0,0,0}};


// 检测外部电平引脚初始化，5V、后盖光敏
void system_adc_input_check_port_init(void)
{
    ///< 开启外设时钟 --------------------------------------------
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
    stc_gpio_cfg_t 	stcGpioCfg;

    stcGpioCfg.enDir         = GpioDirIn;
    stcGpioCfg.enDrv         = GpioDrvL;
    stcGpioCfg.enPu          = GpioPuDisable;
    stcGpioCfg.enPd          = GpioPdDisable;
    stcGpioCfg.enOD          = GpioOdDisable;
    stcGpioCfg.enCtrlMode    = GpioAHB;

    // 初始化检测外部输入5V---------------------------------------------------	
    Gpio_Init(ADC_5V_PORT, ADC_5V_PIN, &stcGpioCfg); 
    ADC_OPEN_5V_OFF_TEST_IRQ(); ///< 打开并配置低电平中断

    // 初始化检测后盖打开输入---------------------------------------------------	
    Gpio_Init(ADC_TAMPER_PORT, ADC_TAMPER_PIN, &stcGpioCfg); 
    ADC_TAMPER_CLOSE_TEST_IRQ(); ///< 关闭光敏中断检测

    // 使能IO口中断---------------------------------------------------
    EnableNvic(PORTA_IRQn, IrqLevel2, TRUE);               ///< 使能端口PORTA系统中断
}

// 检测电池ADC开关引脚初始化
void system_adc_bat_control_port_init(void)
{
    ///< 开启GPIO外设时钟 --------------------------------------------
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
    // 初始化控制检测电池开关---------------------------------------------------
	stc_gpio_cfg_t 	stc_gpio_cfg_init;
	
	stc_gpio_cfg_init.enDir			= GpioDirOut;
	stc_gpio_cfg_init.enDrv			= GpioDrvL;
	stc_gpio_cfg_init.enPu 			= GpioPuDisable;
	stc_gpio_cfg_init.enPd 			= GpioPdDisable;
	stc_gpio_cfg_init.enCtrlMode		= GpioAHB;
	
	Gpio_Init(BAT_ADC_EN_PORT, BAT_ADC_EN_PIN, &stc_gpio_cfg_init);	
    BAT_ADC_CLOSE();     // 关闭检测
}

///< ADC 采样端口初始化
void system_adc_port_init(void)
{    
    ///< 开启外设时钟 --------------------------------------------
    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
    stc_gpio_cfg_t 	stcGpioCfg;

    stcGpioCfg.enDir         = GpioDirIn;
    stcGpioCfg.enDrv         = GpioDrvL;
    stcGpioCfg.enPu          = GpioPuDisable;
    stcGpioCfg.enPd          = GpioPdDisable;
    stcGpioCfg.enOD          = GpioOdDisable;
    stcGpioCfg.enCtrlMode    = GpioAHB;

    Gpio_Init(ADC_BAT_PORT, ADC_BAT_PIN, &stcGpioCfg); 
    Gpio_Init(ADC_24V_PORT, ADC_24V_PIN, &stcGpioCfg);
    Gpio_Init(ADC_TEMP_PORT, ADC_TAMPER_PIN, &stcGpioCfg);

    Gpio_SetAnalogMode(ADC_BAT_PORT, ADC_BAT_PIN);           //PB14 (AIN21)
    Gpio_SetAnalogMode(ADC_24V_PORT, ADC_24V_PIN);           //PA00 (AIN0)
    Gpio_SetAnalogMode(ADC_TEMP_PORT, ADC_TAMPER_PIN);       //PB15 (AIN22)
}


// 检测电池ADC初始化
void system_adc_run_init(void)
{
    ///< ADC模块 初始化 -------------------------------------------------------
    stc_adc_cfg_t              stcAdcCfg;
    DDL_ZERO_STRUCT(stcAdcCfg);
    
    // ///< 开启ADC/BGR外设时钟
    Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, TRUE);
    Bgr_BgrEnable();                                        ///< 开启BGR
	
    // Bgr_TempSensorEnable(); // 使能内部温度传感器
    
    ///< ADC 初始化配置
    stcAdcCfg.enAdcMode         = AdcScanMode;               ///<采样模式-扫描
    stcAdcCfg.enAdcClkDiv       = AdcMskClkDiv8;            ///<采样分频-8
    stcAdcCfg.enAdcSampCycleSel = AdcMskSampCycle12Clk;     ///<采样周期数-12
    stcAdcCfg.enAdcRefVolSel    = AdcMskRefVolSelInBgr2p5; 	///<参考电压选择-内部2.5
    stcAdcCfg.enAdcOpBuf        = AdcMskBufDisable;          ///<OP BUF配置-关
    stcAdcCfg.enInRef           = AdcMskInRefEnable;        ///<内部参考电压使能-开
    stcAdcCfg.enAdcAlign        = AdcAlignRight;            ///<转换结果对齐方式-右
    Adc_Init(&stcAdcCfg);
	
    // ADC 顺序采样模式 配置------------------------------------------------
    stc_adc_sqr_cfg_t          stcAdcSqrCfg;
    
    DDL_ZERO_STRUCT(stcAdcSqrCfg);
        
    ///< 顺序扫描模式功能及通道配置
    stcAdcSqrCfg.bSqrDmaTrig = FALSE;
    stcAdcSqrCfg.enResultAcc = AdcResultAccDisable;
    stcAdcSqrCfg.u8SqrCnt    = 4;
    Adc_SqrModeCfg(&stcAdcSqrCfg);

    Adc_CfgSqrChannel(AdcSQRCH0MUX, AdcExInputCH21);    // 小电池
    Adc_CfgSqrChannel(AdcSQRCH1MUX, AdcExInputCH0);     // 外部电源
    Adc_CfgSqrChannel(AdcSQRCH2MUX, AdcExInputCH22);    // 冰箱温度
 
    // 清除中断标志位
    Adc_ClrIrqStatus(AdcMskIrqSqr);
    ///< ADC 中断使能
    Adc_EnableIrq();
    EnableNvic(ADC_DAC_IRQn, IrqLevel3, TRUE);

    __ADC.ADC_sample_flag = 0;
    
    // ///< 启动顺序扫描采样
    //  Adc_SQR_Start();
    //	BAT_ADC_OPEN();
}

// 关闭ADC的所有相关时钟
void system_adc_sleep(void)
{
    // 清除中断标志位
    Adc_ClrIrqStatus(AdcMskIrqSqr);
    ///< ADC 中断使能
    Adc_DisableIrq();
    EnableNvic(ADC_DAC_IRQn, IrqLevel3, FALSE);
    // 关闭采样
    Adc_SQR_Stop();
    // 关闭ADC检测脚
    BAT_ADC_CLOSE();
    Bgr_BgrDisable();            ///< 关闭BGR
	
	// 重新初始化ADC
	M0P_ADC->CR0 = 0x27F0;
	
	// 关闭ADC时钟
	Sysctrl_SetPeripheralGate(SysctrlPeripheralAdcBgr, FALSE);
}

// 打开电池ADC检测
void system_adc_sample_on(void)
{
    // 打开电池ADC检测脚
    BAT_ADC_OPEN();
	
	delay1ms(5);

    __ADC.ADC_sample_flag = 0; 	

    // 清除中断标志位
    Adc_ClrIrqStatus(AdcMskIrqSqr);
    ///< 启动顺序扫描采样
    Adc_SQR_Start();                    
}

// 关闭电池ADC检测
void system_adc_sample_off(void)
{
    // 关闭采样
    Adc_SQR_Stop();
    // 关闭ADC检测脚
    BAT_ADC_CLOSE();
}



// 温度相关。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
//参数介绍：

//B，是一个系数，与热敏电阻本身系数有关（百度说这个由厂家提供），3435（问卖家即可）
//R25是上拉电阻的阻值，
//Rntc是上面求出来的热敏电阻的阻值
//T25：25℃下的开尔文温度，也就是298.15K
//Tn：此时计算得到的实际开尔文温度（要得到摄氏度 C=Tn-273.15 ）

//模拟量转电阻
float temp_Get_R(uint32_t u32AdcRestult)
{
	//2.5参考电压，先求出电压
	float v1 = (float)(u32AdcRestult * 2.5) / 4096;//ntc上的电压
	float v2 = 2.5 - v1;
	float r = (v1 / v2) * 10;
	
	return r;
}

//实现ln(x) 的计算
double myln(double a)
{
   int N = 15;//取了前15+1项来估算
   int k, nk;
   double x, xx, y;
   x = (a - 1) / (a + 1);
   xx = x * x;
   nk = 2 * N + 1;
   y = 1.0 / nk;
   for(k = N; k > 0; k--)
   {
     nk = nk - 2;
     y = 1.0 / nk + xx * y;
   }

   return 2.0 * x * y;
}

// 传入ADC检测指数得到开尔文的温度
uint32_t system_adc_get_kelvin_temperature(uint32_t u32AdcRestult)
{
	float N1, N2, N3, N4;
	float Rntc = temp_Get_R(u32AdcRestult);
	N1 = (myln(R25) - myln(Rntc)) / B;
	N2 = 1 / T25 - N1;
	N3 = 1 / N2;
	N4 = N3 - 273.15;//开尔文转摄氏度
	
    // 放大温度1000倍
	return (uint32_t)(N4 * 1000);
}
