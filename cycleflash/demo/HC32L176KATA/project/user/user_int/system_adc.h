#ifndef _SYSTEM_ADC_H_
#define _SYSTEM_ADC_H_

#include "main_file_define.h"

// adc的低电压点
#define ADC_LEW_V       9100
// adc的高电压点
#define ADC_HIGH_V      9800

//v2.0.2 20124-3-7 以及之前的
// #define BAT_ADC_OPEN()              Gpio_ClrIO(BAT_ADC_EN_PORT, BAT_ADC_EN_PIN)
// #define BAT_ADC_CLOSE()             Gpio_SetIO(BAT_ADC_EN_PORT, BAT_ADC_EN_PIN)
//v2.0.2 20124-3-22 以及之后的
#define BAT_ADC_OPEN()              Gpio_SetIO(BAT_ADC_EN_PORT, BAT_ADC_EN_PIN)
#define BAT_ADC_CLOSE()             Gpio_ClrIO(BAT_ADC_EN_PORT, BAT_ADC_EN_PIN)

// 采集电压
#define ADC_START()                 Adc_SQR_Start()

// 获取外部输入的电平状态
#define ADV_5V_GET_STATE()          Gpio_GetInputIO(ADC_5V_PORT, ADC_5V_PIN)
// 关机中断检测，低电平
#define ADC_OPEN_5V_OFF_TEST_IRQ()  Gpio_DisableIrq(ADC_5V_PORT, ADC_5V_PIN, GpioIrqHigh); Gpio_EnableIrq(ADC_5V_PORT, ADC_5V_PIN, GpioIrqLow)
// 关闭关机中断检测，低电平
#define ADC_CLOSE_5V_OFF_TEST_IRQ() Gpio_DisableIrq(ADC_5V_PORT, ADC_5V_PIN, GpioIrqLow)
// 打开开机中断检测，高电平
#define ADC_OPEN_5V_ON_TEST_IRQ()   Gpio_EnableIrq(ADC_5V_PORT, ADC_5V_PIN, GpioIrqHigh)

// 获取外部输入的电平状态
#define ADV_TAMPER_GET_STATE()      Gpio_GetInputIO(ADC_TAMPER_PORT, ADC_TAMPER_PIN)
// 后盖关上中断检测，低电平
#define ADC_TAMPER_OFF_TEST_IRQ()   Gpio_DisableIrq(ADC_TAMPER_PORT, ADC_TAMPER_PIN, GpioIrqHigh); Gpio_EnableIrq(ADC_TAMPER_PORT, ADC_TAMPER_PIN, GpioIrqLow)
// 后盖打开中断检测，高电平
#define ADC_TAMPER_ON_TEST_IRQ()    Gpio_DisableIrq(ADC_TAMPER_PORT, ADC_TAMPER_PIN, GpioIrqLow); Gpio_EnableIrq(ADC_TAMPER_PORT, ADC_TAMPER_PIN, GpioIrqHigh)
// 关闭后盖关上中断检测
#define ADC_TAMPER_CLOSE_TEST_IRQ() Gpio_DisableIrq(ADC_TAMPER_PORT, ADC_TAMPER_PIN, GpioIrqHigh);Gpio_DisableIrq(ADC_TAMPER_PORT, ADC_TAMPER_PIN, GpioIrqLow)

enum DEVICE_VOLTAGE
{
    NULL_VOLTAGE,       // 初始化   
    LOW_VOLTAGE,        // < 9.1V         24V：< 21V
    MEDIUM_VOLTAGE,     // 9.1 - 9.8V     24V: 21V - 22.6V
    HIGH_VOLTAGE        // > 9.8V         24V: >22.6   
};

struct ADC_SAMPLE_STRUCT
{
    // 设备电压
    uint8_t device_voltage;
	// 采集结束标识符
    _Bool ADC_sample_flag;
    // 2s信号
    uint8_t ADC_2s_flg;
    // 采样数据， 【0】为电池电压，【1】为外部电源电压，【2】温度
    uint32_t ADC_sample_data[3];   
};

extern struct ADC_SAMPLE_STRUCT	__ADC;

// 检测外部电平引脚初始化，5V、后盖光敏
void system_adc_input_check_port_init(void);

// 检测电池ADC开关引脚初始化
void system_adc_bat_control_port_init(void);

///< ADC 采样端口初始化
void system_adc_port_init(void);

// 检测电池ADC初始化
void system_adc_run_init(void);

// 关闭ADC的所有相关时钟
void system_adc_sleep(void);

// // ADC中断检测电池电压
// void Adc_IRQHandler(void);

// 打开电池ADC检测
void system_adc_sample_on(void);

// 关闭电池ADC检测
void system_adc_sample_off(void);

// 传入ADC检测指数得到开尔文的温度
uint32_t system_adc_get_kelvin_temperature(uint32_t u32AdcRestult);

#endif
