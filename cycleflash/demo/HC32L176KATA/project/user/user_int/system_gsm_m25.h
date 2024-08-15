/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_gsm_m25.h
* Description:	
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#ifndef __SYSTEM_GSM_M25_H_
#define __SYSTEM_GSM_M25_H_

#include "main_struct_typedef.h"
#include "main_file_define.h"



#define GSM_GET_PWRKEY()  Gpio_GetInputIO(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN)
#define GSM_GET_EN()      Gpio_GetInputIO(GSM_EN_PORT, GSM_EN_PIN)
//#define GSM_GET_EN()      GSM_EN_STATE

#define GSM_SET_EN()      Gpio_SetIO(GSM_EN_PORT, GSM_EN_PIN);
#define GSM_CLR_EN()      Gpio_ClrIO(GSM_EN_PORT, GSM_EN_PIN);
//#define GSM_SET_EN()      GSM_EN_STATE = 1
//#define GSM_CLR_EN()      GSM_EN_STATE = 0
#define GSM_SET_PWRKEY()  Gpio_SetIO(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN)
#define GSM_CLR_PWRKEY()  Gpio_ClrIO(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN)

#define GSM_POWER_RESTART_TIME  24000           // 模块超时重启时间     

//最大接收缓存字节数
#define RX_BUF_MAX_LEN     640//1024

//串口数据帧的处理结构体
extern struct  STRUCT_LPUSART0_Fram
{
	char  Data_RX_BUF [ RX_BUF_MAX_LEN ];
    union 
    {
        __IO uint16_t InfAll;
        struct 
        {
            __IO uint16_t FramLength       :15;
            __IO uint16_t FramFinishFlag   :1;
        } InfBit;
    }; 
	
} strM25_Fram_Record;

enum MQTT_TYPE
{
    MQTT_TYPE_NULL,         //                      0
    MQTT_TYPE_POWER_ON,     // 开机                 1
    MQTT_TYPE_POWER_OFF,     // 关机                2
    MQTT_TYPE_POWER_RESTART,     // 重启            3

    MQTT_TYPE_SEND,         // AT 发送状态          4
    MQTT_TYPE_WAIT_RX,       // AT 接收状态         5

    // 联网过程
    MQTT_TYPE_AT,           //                     6
    MQTT_TYPE_ATE0,         //                     7
    MQTT_TYPE_QICSGP,       // APN                  8
    MQTT_TYPE_CGATT,        // # 是否附基站         9
    MQTT_TYPE_QIREGAPP,     // # 执行接入           10
    MQTT_TYPE_QNITZ,        // 同步网络时间
    MQTT_TYPE_QLTS,         // 获取同步的网络时间
    MQTT_TYPE_CGDCONT,      // PDP                  11
    MQTT_TYPE_CGACT,        // # 激活PDP            12
    MQTT_TYPE_QIACT,        // # 激活移动场景       13
    MQTT_TYPE_QILOCIP,      // # 获取IP地址         14
    MQTT_TYPE_QISTAT,       // # 查看连接状态       15
    
    // MQTT连接和发送过程
    MQTT_TYPE_VERSIN,       // MQTT版本             16
    MQTT_TYPE_KEEPALIVE,    // MQTT心跳             17
    MQTT_TYPE_ALIAUTH,      // MQTT阿里云参数       18
    MQTT_TYPE_QMTOPEN,      // TCP                  19
    MQTT_TYPE_QMTCONN,      // MQTT连接             20
    MQTT_TYPE_QMTPUB,        // 发送信息            21

    MQTT_TYPE_QMTSUB,           // 订阅OTA主动获取  22
    MQTT_TYPE_QMTPUB_VERSION,   // 上报版本         23
    MQTT_TYPE_QMTPUB_OTA,       // OTA获取          24

    // OTA升级过程
    MQTT_TYPE_QHTTPURL,      // 给它链接            25
    MQTT_TYPE_QHTTPGET,      // 获取链接资源        26
    MQTT_TYPE_QHTTPDL,       // 保存到内存中        27
    MQTT_TYPE_QFOPEN,        // 打开刚保存的bin     28
    MQTT_TYPE_QFREAD,        // 读取打开的文件      29

    // 查询信号
    MQTT_TYPE_CSQ          // 信号                  30
};

enum OTA_TYPE
{
    OTA_TYPE_NULL,          // 无任务
    OTA_TYPE_UPGRADE_TASK,  // 有升级任务
    OTA_TYPE_BEING_UPDATED  // 正在升级，禁止输入
};

//串口数据帧的处理结构体
extern struct STRUCT_MQTT_STATE
{
    // 0无升级， 1有升级任务, 2正在升级
    uint8_t mqtt_ota_state;     // ota状态管理

	uint8_t mqtt_at_type;       // 管理mqtt发送流程  发送指令的流程
    uint8_t mqtt_task_state;    // 管理mqtt任务状态  开机、关机、发送、接收、重启等
    uint8_t mqtt_unsent_count;    // 开机后未发送次数，3次内没连接基站成功或者连接TCP失败就重启。大于3次就关机。
    uint16_t mqtt_task_state_count_down;    // 专门用来倒计时延时时间，一个计数为5ms
    uint16_t mqtt_wait_receive_count;    // 等待回复超时时间

    _Bool mqtt_cyclic_send;  // 是否打开GSM的发送，关闭它，不管有没有都要循环发送，暂时不启用
} mqtt_state;


struct GSM_MQTT_PARAM           // 管理mqqt发送参数
{
    uint16_t count;             
    uint16_t time_remaining_hour;
    uint16_t time_remaining_min;
    float    BAT_VDC;
    float    DC_VDC;
    int    refrigerator_thermometer;      //冰箱温度
    _Bool    power_state;   // 当前状态
};

extern uint32_t gsm_power_restart_time;

extern uint32_t gsm_power_sending_time_count;

extern _Bool GSM_EN_STATE;


// MQTT发送拼接，所有需要拼接的信息都通过这里
void system_gsm_mqtt_send_splicing(enum MQTT_TYPE temp_mqtt_type);

// 开关机和供电引脚初始化
void system_gsm_port_init(void);
// 开机/关机控制
void system_gsm_enabled_state(void);
// 开机
_Bool system_gsm_enabled_power_on(void);
// 关机
_Bool system_gsm_enabled_power_off(void);

// 设置进入关机流程
void system_gsm_ready_to_shut_down(void);
// 设置进入重启流程
void system_gsm_ready_to_restart(void);

// 获取OTA 的 URL
void system_gsm_mqtt_get_uart(void);
// MQTT发送处理
void system_gsm_mqtt_send(void);
// MQTT接收处理
void system_gsm_mqtt_receive(void);

// MQTT总流程控制
void system_gsm_mqtt_manage(void);

#endif
