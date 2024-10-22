/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_gsm_m25.c
* Description:	
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/
#include "system_gsm_m25.h"
#include "system_ota.h"

_Bool GSM_EN_STATE = 0;

/* ***********************************************************************
 * 需要记住的几个阿里Token
 * 上报属性： /sys/${ProductKey}/${deviceName}/thing/event/property/post
 * 上报设备版本：/ota/device/inform/${ProductKey}/${deviceName}
 *      {
            "id": "123",
            "params": {
                "version": "1.0.1"
            }
        }
 * 主动获取OTA订阅： /sys/${ProductKey}/${deviceName}/thing/ota/firmware/get_reply
 * 主动获取OTA信息： /sys/${ProductKey}/${deviceName}/thing/ota/firmware/get
 *      {
            "id": "123",
            "version": "1.0",
            "params": {
            },
            "method": "thing.ota.firmware.get"
        }
 *
**/

// MQTT循环发送倒计时装载， 1点为1s    
#define DEFINE_GSM_POWER_SEND_TIME_COUNT     600         // 目前装载10分钟
// 读取校验码位数
#define READ_GSM_BIN_CRC            2
// 每次读取bin文件大小
#define READ_GSM_BIN_SIZE           512

#define AT                        "AT\r\n"                                        // 1、AT。OK
#define ATE0                      "ATE0\r\n"                                      // 2、关闭回显 OK
#define AT_QICSGP                 "AT+QICSGP=1,\"iot-eu.aer.net\",\"\",\"\"\r\n"  // 3、定义物联网的APN基站位置。 OK
#define AT_CGATT                  "AT+CGATT?\r\n"                                 // 4、检测是否附着成功基站
#define AT_QIREGAPP               "AT+QIREGAPP\r\n"                               // 5、执行接入 OK
#define AT_CGDCONT                "AT+CGDCONT=1,\"IP\",\"\"\r\n"                  // 6、定义PDP上下文。 OK
#define AT_CGACT                  "AT+CGACT=1,1\r\n"                              // 7、激活PDP上下文  OK
#define AT_QIACT                  "AT+QIACT\r\n"                                  // 8、激活移动场景  OK
#define AT_QILOCIP                "AT+QILOCIP\r\n"                                // 9、获取地址
#define AT_QISTAT                 "AT+QISTAT\r\n"                                 // 10、查看连接状态
#define AT_QNITZ                  "AT+QNITZ=1\r\n" 
#define AT_QLTS                   "AT+QLTS\r\n" 
// OTA相关 ----------------
#define AT_QHTTPGET               "AT+QHTTPGET=60\r\n"                            // 下载链接的文件
// 注意：这一步会返回文件大小
#define AT_QHTTPDL                "AT+QHTTPDL=\"RAM:bin.BIN\",204800\r\n"         // 把文件保存在200k大小的RAM中
// 注意：这一步会返回文件指针
#define AT_QFOPEN                 "AT+QFOPEN=\"RAM:bin.BIN\"\r\n"
// MQTT相关----------------------
#define AT_QMTCFG_VERSIN          "AT+QMTCFG=\"VERSION\",0,1\r\n"                 // 定义版本号。 OK
#define AT_QMTCFG_KEEPALIVE       "AT+QMTCFG=\"KEEPALIVE\",0,60\r\n"              // 定义心跳时间。 OK
#define AT_QMTCLOSE               "AT+QMTCLOSE=0\r\n"                             // 退出MQTT的TCP连接。
#define AT_QMTDISC                "AT+QMTDISC=0\r\n"                                // 断开MQTT客户端和服务端连接


#define AT_CSQ                    "AT+CSQ\r\n"                                    // 查信号。


#define AT_REPLY_OK               "OK"  
// TCP连接回复
#define AT_REPLY_QMTOPEN_OK       "0,0" // "\r\n+QMTOPEN: 0,0"  
// 连接MQTT服务器回复
#define AT_REPLY_QMTCONN_OK       "0,0,0" // "\r\n+QMTCONN: 0,0,0" 
// 上报属性回复
#define AT_REPLY_QMTPUB_OK        "0,3441,0" // "\r\n+QMTPUB: 0,3441,0"  
// 查看是否附上基站                 
#define AT_REPLY_CGATT_OK         "1"
// 查询当前连接状态
#define AT_REPLY_QISTAT_OK       "STATUS"
// 查询IP地址
#define AT_REPLY_QILOCIP_OK       "."

// 发送链接回复
// #define AT_REPLY_QHTTPURL         "CONNECT"  // 收到这个回复可以发链接
// 存入文件回复
#define AT_REPLY_QHTTPDL          "+QHTTPDL: " // 定位到文件大小的位置
// 读取文件
#define AT_REPLY_QFOPEN           "+QFOPEN: "  // 定位到读取文件的指针位置
// 读取回复读取了几个字节
#define AT_REPLY_QFREAD           "CONNECT "    // 定位到读取了几个字节，注意connect后面有一个空格
// 订阅成功的回复
#define AT_REPLY_QMTSUB            "0,1,0"     // 就0号TCP，消息标识符为1，服务器接收到消息
// 主动获取OTA信息，订阅回复
#define AT_REPLY_QMTSUB_OTA                 "+QMTRECV:"
// 主动获取OTA信息，订阅回复有升级链接---头
#define AT_REPLY_QMTSUB_OTA_HEAD                 "\"url\":\""
// 主动获取OTA信息，订阅回复有升级链接---尾
#define AT_REPLY_QMTSUB_OTA_TAIL                 "\",\"md5\":\""
// 读取信号
#define AT_REPLY_CSQ                    "+CSQ: "        //+CSQ: 28,0

#define AT_REPLY_QLTS                   "+QLTS:"       // +QLTS: "24/03/19,03:53:17+32,0"

// 读取版本号
#define AT_REPLY_VERSION              "\"version\":\""

// 关机回复
#define AT_REPLY_POWER_OFF_OK          "NORMAL POWER DOWN" 

// 信号强度
uint8_t csq_value = 0;

// M25接收数据结构体
struct  STRUCT_LPUSART0_Fram strM25_Fram_Record = { 0 };
// MQTT状态管理
struct  STRUCT_MQTT_STATE mqtt_state = { 0 };
// GSM重启倒计时
uint32_t gsm_power_restart_time = 0;
// GSM发送计时，就多久上传数据
uint32_t gsm_power_sending_time_count = 0;

// 指向存放下载链接指针的，没用的时候就释放掉
char *ota_bin_url = NULL;
// 存放返回的文件指针
uint32_t ota_bin_pointer = 0;
// 每次读取文件大小
uint16_t ota_bin_read_size = READ_GSM_BIN_SIZE;
// 同步时间
uint8_t mqtt_lock_in_time[30]; 


// 初始化所有结构体和变量
void system_gsm_value(void)
{
    // 信号强度
    csq_value = 0;

    // M25接收数据结构体
    memset(&strM25_Fram_Record, 0, sizeof(struct  STRUCT_LPUSART0_Fram));
    // MQTT状态管理
    memset(&mqtt_state, 0, sizeof(struct  STRUCT_MQTT_STATE));

    // GSM重启倒计时
    gsm_power_restart_time = 0;
    // GSM发送计时，就多久上传数据
    gsm_power_sending_time_count = 0;

    // 指向存放下载链接指针的，没用的时候就释放掉
    if(ota_bin_url != NULL)
    {
        ota_bin_url = NULL;
    }

    // 存放返回的文件指针
    ota_bin_pointer = 0;
    // 每次读取文件大小
    ota_bin_read_size = READ_GSM_BIN_SIZE;
    // 同步时间
    memset(&mqtt_lock_in_time[0], 0, sizeof(mqtt_lock_in_time));
}


/********************************************************************
 ** \brief  MQTT发送拼接，所有需要拼接的信息都通过这里
 ** @param  无
 ** \retval 无
********************************************************************/
void system_gsm_mqtt_send_splicing(enum MQTT_TYPE temp_mqtt_type)
{
    // 有点怕栈溢出，所以动态创建
    char *temp_mqtt_str = (char *)malloc(400);
    // 上报指令
    char *send_at_qmtpub_token = (char *)malloc(150);

    // 重新获取product key 填充 '\0'
	uint8_t temp_product_key[sizeof(_this_pointer._this_product_id_pointer->product_key)+1] = {0};
	memcpy(temp_product_key, _this_pointer._this_product_id_pointer->product_key, \
									sizeof(_this_pointer._this_product_id_pointer->product_key));
	// 重新获取device_secretx 填充 '\0'
	uint8_t temp_device_secret[sizeof(_this_pointer._this_product_id_pointer->device_secret)+1] = {0};
	memcpy(temp_device_secret, _this_pointer._this_product_id_pointer->device_secret,\
									sizeof(_this_pointer._this_product_id_pointer->device_secret));

    switch (temp_mqtt_type)
    {
        case MQTT_TYPE_ALIAUTH:  // 阿里云参数
            sprintf(temp_mqtt_str, "AT+QMTCFG=\"ALIAUTH\",0,\"%s\",\"%d\",\"%s\"\r\n", \
                                temp_product_key, \
                                _this_pointer._this_product_id_pointer->unit_number_code, \
                                temp_device_secret);
            break;

        case MQTT_TYPE_QMTOPEN:  // TCP连接
            sprintf(temp_mqtt_str, "AT+QMTOPEN=0,\"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883\r\n", \
                                temp_product_key);
            break;

        case MQTT_TYPE_QMTCONN:  // MQTT连接
            sprintf(temp_mqtt_str, "AT+QMTCONN=0,\"%d\"\r\n", \
                                _this_pointer._this_product_id_pointer->unit_number_code);
            break;

        case MQTT_TYPE_QMTPUB:  // 上报属性
			if(temp_mqtt_type == MQTT_TYPE_QMTPUB)
			{
				struct GSM_MQTT_PARAM temp_param;
				op_manage_get_mqtt_param((void *)(&temp_param)); 

				// 上报的内容-----------------------------------------------
                // 开头
                sprintf(temp_mqtt_str, "{\"method\":\"thing.service.property.set\",\"id\":\"%d\",\
                                            \"params\":{\"count\":%d,\"time_remaining\":{\"hour\":%d,\"min\":%d},\
                                            \"CSQ\":%d,\"BAT_VDC\":%.3f,\"DC_VDC\":%.3f,\"power_state\":%d", \
                                            _this_pointer._this_product_id_pointer->unit_number_code,\
                                            temp_param.count, \
                                            temp_param.time_remaining_hour,temp_param.time_remaining_min,\
                                            csq_value,\
                                            temp_param.BAT_VDC,\
                                            temp_param.DC_VDC,\
                                            temp_param.power_state);
                                            
                // 后盖是否打开，上报打开时间    
                if(__sleep_mode.mqtt_tamper_open == TRUE)
                {
                    sprintf(temp_mqtt_str, "%s,\"tamper_open\":\"%s\"", temp_mqtt_str, mqtt_lock_in_time);
                    __sleep_mode.mqtt_tamper_open = FALSE;
                }

                // 是否为开机状态，上报温度
                if(__sleep_mode.device_power_on_state == TRUE)
                {
                    sprintf(temp_mqtt_str, "%s,\"refrigerator_thermometer\":%d",\
                                            temp_mqtt_str,\
                                            temp_param.refrigerator_thermometer);
					
					__NOP();
                }

                // 上报结尾---------------------------------------------------------
                sprintf(temp_mqtt_str, "%s},\"version\":\"1.0.0\"}", temp_mqtt_str);

				// 上报Token 和要发送的数据字节
				sprintf(send_at_qmtpub_token, "AT+QMTPUB=0,3441,1,0,\
										\"/sys/%s/%d/thing/event/property/post\",%d\r\n",\
										temp_product_key, \
										_this_pointer._this_product_id_pointer->unit_number_code,\
										strlen(temp_mqtt_str));

				system_ble_lpuart0_send_string(send_at_qmtpub_token, strlen(send_at_qmtpub_token));

				delay1ms(500);
			}
            break;

        // 关于OTA获取 --------------------------------------------------------------------
        case MQTT_TYPE_QMTSUB:           // 订阅OTA主动获取
            sprintf(temp_mqtt_str, "AT+QMTSUB=0,1,\"/sys/%s/%d/thing/ota/firmware/get_reply\",2\r\n",\
                                    temp_product_key, \
                                    _this_pointer._this_product_id_pointer->unit_number_code);
            break;

        case MQTT_TYPE_QMTPUB_VERSION:   // 上报版本
            // 上报的内容
            sprintf(temp_mqtt_str, "{\"id\":\"%d\",\"params\":{\"version\": \"%s\"}}", \
                                    _this_pointer._this_product_id_pointer->unit_number_code,\
                                    _this_pointer._this_product_id_pointer->version);
            // 上报Token 和要发送的数据字节
            sprintf(send_at_qmtpub_token, "AT+QMTPUB=0,3441,1,0,\
                                    \"/ota/device/inform/%s/%d\",%d\r\n",\
                                    temp_product_key, \
                                    _this_pointer._this_product_id_pointer->unit_number_code,\
                                    strlen(temp_mqtt_str));

            system_ble_lpuart0_send_string(send_at_qmtpub_token, strlen(send_at_qmtpub_token));

            delay1ms(500);
            break;

        case MQTT_TYPE_QMTPUB_OTA:       // OTA获取
            // 上报的内容
            sprintf(temp_mqtt_str, "{\"id\":\"123\",\"version\":\"1.0\",\
                                    \"params\":{},\"method\":\"thing.ota.firmware.get\"}");
            // 上报Token 和要发送的数据字节
            sprintf(send_at_qmtpub_token, "AT+QMTPUB=0,3441,1,0,\
                                    \"/sys/%s/%d/thing/ota/firmware/get\",%d\r\n",\
                                    temp_product_key, \
                                    _this_pointer._this_product_id_pointer->unit_number_code,\
                                    strlen(temp_mqtt_str));
            system_ble_lpuart0_send_string(send_at_qmtpub_token, strlen(send_at_qmtpub_token));

            delay1ms(500);
            break;

        // 关于HTTP--------------------------------------------------------------------
        case MQTT_TYPE_QHTTPURL:     // 上传下载链接
            sprintf(temp_mqtt_str, "AT+QHTTPURL=%d,30\r\n",strlen(ota_bin_url));
            system_ble_lpuart0_send_string(temp_mqtt_str, strlen(temp_mqtt_str));

            delay1ms(500);

            sprintf(temp_mqtt_str, "%s",ota_bin_url);
            break; 

        case MQTT_TYPE_QFREAD:       // 通过指针读取文件
            // 设置读取文件长度
            if(temp_mqtt_type == MQTT_TYPE_QFREAD)
            {
				uint32_t temp_size = ota_get_ota_bin_size();
				uint32_t temp_move = ota_get_ota_bin_move();
                uint32_t temp_bin_move = temp_size - READ_GSM_BIN_CRC - temp_move;
                
                if( temp_bin_move >= READ_GSM_BIN_SIZE) // 文件长度大于正常读取文件长度，继续一次读取一页的文件
                {
                    ota_bin_read_size = READ_GSM_BIN_SIZE;
                }
                else if(temp_bin_move < READ_GSM_BIN_SIZE && temp_bin_move != 0)    // 文件长度小于一页，计算剩余长度
                {
                    ota_bin_read_size = temp_bin_move;      // 剩余的读取出来
                }
                else    // 文件读取完毕，等于0了，最后读取校验码
                {
                    ota_bin_read_size = READ_GSM_BIN_CRC;
                }

				sprintf(temp_mqtt_str, "AT+QFREAD=%d,%d\r\n",ota_bin_pointer, ota_bin_read_size);
			}
            break;

        default:
            break;
    }

    strM25_Fram_Record.InfBit.FramLength = 0;
    strM25_Fram_Record.InfBit.FramFinishFlag = 0;
	
	__NOP();
    system_ble_lpuart0_send_string(temp_mqtt_str, strlen(temp_mqtt_str));

    // 一定要释放
    free(temp_mqtt_str);
    temp_mqtt_str = NULL;
    free(send_at_qmtpub_token);
    send_at_qmtpub_token = NULL;
}


/********************************************************************
 ** \brief  初始化GSM的电源使能引脚和开机使能引脚
 ** @param  无
 ** \retval 无
********************************************************************/
void system_gsm_port_init(void)
{
    // 初始化所有结构体和变量***********************************
    system_gsm_value();

    stc_gpio_cfg_t 	stc_out_led_init;
	
	Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio, TRUE);
	
	stc_out_led_init.enDir		= GpioDirOut;
	stc_out_led_init.enDrv		= GpioDrvL;
	stc_out_led_init.enPu 		= GpioPuDisable;
	stc_out_led_init.enPd 		= GpioPdDisable;
	stc_out_led_init.enCtrlMode	= GpioAHB;
	
	Gpio_Init(GSM_EN_PORT, GSM_EN_PIN, &stc_out_led_init);
	Gpio_Init(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN, &stc_out_led_init);

    Gpio_ClrIO(GSM_EN_PORT, GSM_EN_PIN);
    Gpio_ClrIO(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN);
}


/********************************************************************
 ** \brief  GSM开机
 ** @param  无
 ** \retval 1为成功，0还在执行
********************************************************************/
_Bool system_gsm_enabled_power_on(void)
{
    if(GSM_GET_EN() == FALSE)
    {
        TIMER_5ms_ON();     // 打开5ms定时器 为功耗考虑
		
		// LPuart0 引脚初始化
		system_lpuart0_port_init();
		
        GSM_SET_EN();
        mqtt_state.mqtt_task_state_count_down = 20; //倒计时100ms
    }
    else if(GSM_GET_PWRKEY() == FALSE)
    {
        GSM_SET_PWRKEY();
        mqtt_state.mqtt_task_state_count_down = 400; //倒计时2S
    }
    else
    {
        GSM_CLR_PWRKEY();
        mqtt_state.mqtt_at_type = MQTT_TYPE_AT;
        mqtt_state.mqtt_task_state = MQTT_TYPE_SEND;
		
		mqtt_state.mqtt_task_state_count_down = 800; //倒计时4S
        return 1;
    }
    
    return 0;
}

/********************************************************************
 ** \brief  GSM关机，会在这里卡
 ** @param  无
 ** \retval 1为成功，0还在执行
********************************************************************/
_Bool system_gsm_enabled_power_off(void)
{
	if(GSM_GET_EN() == TRUE && GSM_GET_PWRKEY() == FALSE)
	{	
        GSM_SET_PWRKEY();
		mqtt_state.mqtt_task_state_count_down = 400; //倒计时2S
	}
	else if(GSM_GET_EN() == TRUE)
	{
        GSM_CLR_PWRKEY();
        GSM_CLR_EN();
		
		// 关闭 LPuart0 端口
		system_lpuart0_port_close();

		mqtt_state.mqtt_task_state_count_down = 400; //倒计时2S
	}
    else
    {
        mqtt_state.mqtt_at_type = MQTT_TYPE_NULL;
		mqtt_state.mqtt_task_state = MQTT_TYPE_NULL;

		TIMER_5ms_OFF();    // 关闭5ms定时器  为功耗考虑
        return 1;
    }
	
	return 0;
}


/********************************************************************
 ** \brief  GSM开机/关机/重启 控制
 ** @param  无
 ** \retval 无
********************************************************************/
void system_gsm_enabled_state(void)
{
    // 只要机器重启过了一次，然后重启计数又是倍数关系，直接关机，下次在发了。
    if(mqtt_state.mqtt_unsent_count != 0 && (mqtt_state.mqtt_unsent_count%3 == 0) \
                                    && mqtt_state.mqtt_task_state == MQTT_TYPE_POWER_RESTART)
    {
        mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_OFF;
        gsm_power_restart_time = 0;
        mqtt_state.mqtt_unsent_count = 0;

        // 唯一的一个全局动态内存，一定要注意有没有释放哦
        // 内存泄漏 no wey no wey！！！！！！
        if(ota_bin_url != NULL)
        {
            free(ota_bin_url);
            ota_bin_url = NULL;
        }
    }

    // 任务倒计时， 一个点为 5mS， 起到一个缓冲的作用，
    if(mqtt_state.mqtt_task_state_count_down == 0)
    {
        // 现在MQTT的任务状态
        switch (mqtt_state.mqtt_task_state)
        {
            case MQTT_TYPE_POWER_ON:
                if(system_gsm_enabled_power_on())
                {
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                break;
            
            case MQTT_TYPE_POWER_OFF:
                system_gsm_enabled_power_off();
                break;
            case MQTT_TYPE_POWER_RESTART:
                if(system_gsm_enabled_power_off())
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_ON;
                }
                break;

            default:
                break;
        }
    }
}

// 获取OTA 的 URL
void system_gsm_mqtt_get_uart(void)
{
    if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTSUB_OTA_HEAD) == TRUE \
                && (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_VERSION) == TRUE)
    {
        uint8_t *ota_version_head = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_VERSION); 
        ota_version_head = ota_version_head + strlen(AT_REPLY_VERSION);
        
        uint8_t temp_version[sizeof(_this_pointer._this_product_id_pointer->version)];
        do{
            sprintf((char *)temp_version,"%s%c", temp_version, *ota_version_head);
            ota_version_head++;
        } while (*ota_version_head != 0x22);

        // 版本是否一致
        if( memcmp(&_this_pointer._this_product_id_pointer->version, temp_version, \
                                sizeof(_this_pointer._this_product_id_pointer->version)) != 0) 
        {
            uint8_t *ota_head = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTSUB_OTA_HEAD); 
            ota_head = ota_head + strlen(AT_REPLY_QMTSUB_OTA_HEAD);
            uint8_t *ota_tail = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTSUB_OTA_TAIL); 
            // 计数url赋值到哪里了
            uint16_t temp_bin_url_count = 0;

            // 检查是否要先释放内存
            if(ota_bin_url != NULL)
            {
                free(ota_bin_url);
                ota_bin_url = NULL;
            }
            ota_bin_url = (char *)malloc(ota_tail - ota_head + 1);

            do
            {
                ota_bin_url[temp_bin_url_count] = *ota_head;
                temp_bin_url_count++;
                ota_head++;
            } while (ota_head != ota_tail);

            ota_bin_url[temp_bin_url_count] = '\0';

            
            // 设置进入重启流程
            system_gsm_ready_to_restart();
            mqtt_state.mqtt_ota_state = 1;

        }
        else
        {
            // 进入关机流程
            system_gsm_ready_to_shut_down();
        }
    }
    else
    {
        // 进入关机流程
        system_gsm_ready_to_shut_down();
    }                    
}


/********************************************************************
 ** \brief  MQTT接收处理
 ** @param  无
 ** \retval 无
********************************************************************/
void system_gsm_mqtt_receive(void)
{
    if(strM25_Fram_Record.InfBit.FramFinishFlag == 1 && mqtt_state.mqtt_task_state == MQTT_TYPE_WAIT_RX \
                                                                && strM25_Fram_Record.InfBit.FramLength < 4)
    {
        strM25_Fram_Record.InfBit.FramFinishFlag = 0;
    }
    else if(strM25_Fram_Record.InfBit.FramFinishFlag == 1 && mqtt_state.mqtt_task_state == MQTT_TYPE_WAIT_RX)// 如果 lpuart0 接收到数据  并且 任务状态也为接收状态
    {
        // 预先设置为发送状态，如没用接收到消息会重置为当前状态
        mqtt_state.mqtt_task_state = MQTT_TYPE_SEND;
        // 等待回复超时，这是正向计时的计时器
        mqtt_state.mqtt_wait_receive_count = 0;
        // 任务倒计时，给一点点接收处理的事件，同时如果没有回复也不要轰炸的太快，算是控制一发送的频率，一点为5ms
        mqtt_state.mqtt_task_state_count_down = 20;

        // 执行任务流程
        switch (mqtt_state.mqtt_at_type)
        {
            // 连接配置----------------
            case MQTT_TYPE_AT:
            case MQTT_TYPE_ATE0:  
            case MQTT_TYPE_QICSGP:
            case MQTT_TYPE_CGDCONT:
            case MQTT_TYPE_QIREGAPP:
            case MQTT_TYPE_CGACT:
            case MQTT_TYPE_QIACT:
            // 同步网络时间
            case MQTT_TYPE_QNITZ:
            // MQTT配置---------------
            case MQTT_TYPE_VERSIN:
            case MQTT_TYPE_KEEPALIVE:
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type++;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                break;
            
            case MQTT_TYPE_ALIAUTH: 
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_CSQ;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
				break;

            // 同步网络时间
            case MQTT_TYPE_QLTS: 
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QLTS) == TRUE )
                {
                    memset(mqtt_lock_in_time, 0, sizeof(mqtt_lock_in_time));
                    uint8_t *q = &mqtt_lock_in_time[0];
                    uint8_t *p = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QLTS); 
                    p = p + strlen(AT_REPLY_QLTS);
                    
                    _Bool temp_start = 0;
                    uint8_t temp_count = 0;
                    do{
                        if(*p != '"' && temp_start == TRUE)
                        {
                            *q = *p;
                            q++;
                        }
                        else if(*p == '"' && temp_start != TRUE)
                        {
                            temp_start = TRUE;
                            temp_count = 0;
                        }
                        else if(*p == '"' && temp_start == TRUE)
                        {
                            *q = '\0';
                            break;
                        }

                        if(temp_count > 28)
                        {
                            *q = '\0';
                            break;
                        }
                        p++;
                        temp_count++;
                    } while (1);

                    mqtt_state.mqtt_at_type = MQTT_TYPE_CGDCONT;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
				break;

            // 获取一下信号
            case MQTT_TYPE_CSQ: 
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_CSQ) == TRUE )
                {
                    csq_value = 0;
                    uint8_t *p = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_CSQ); 
                    p = p + strlen(AT_REPLY_CSQ);

                    do{
                        if(*p >= 0x30 && *p <= 0x39)
                        {
                            csq_value = csq_value * 10 + (*p - '0');
                        }
                        p++;
                    } while (*p >= 0x30 && *p <= 0x39);

                    mqtt_state.mqtt_at_type = MQTT_TYPE_QMTOPEN;
                    // 准备进入服务器过程
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME*2;
                }
                break;
            
            // 关于附着基站过程
            case MQTT_TYPE_CGATT:        // 有没有附着基站
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_CGATT_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QIREGAPP;     // 执行接入网络
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                else
                {
                    mqtt_state.mqtt_task_state_count_down = 200;       // 任务等待 1S
                }
                break;

            case MQTT_TYPE_QILOCIP:        // 查看IP地址
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QILOCIP_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QISTAT;     // 执行接入网络
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                else
                {
                    mqtt_state.mqtt_task_state_count_down = 200;       // 任务等待 1S
                }
                break;

            case MQTT_TYPE_QISTAT:        // 查看连接状态
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QISTAT_OK) == TRUE )
                {
                    if( mqtt_state.mqtt_ota_state == OTA_TYPE_UPGRADE_TASK)
                    {
                        mqtt_state.mqtt_at_type = MQTT_TYPE_QHTTPURL;     // 下载文件
                    }
                    else
                    {
                        mqtt_state.mqtt_at_type = MQTT_TYPE_VERSIN;     // 上报属性
                    }

                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                else
                {
                    mqtt_state.mqtt_task_state_count_down = 200;       // 任务等待 1S
                }
                break;

            // 以上联网配置完毕 ---------------------------------------------------------------------------------------------------

            // 以下是连接MQTT的过程 --------------------------------------------------------
            case MQTT_TYPE_QMTOPEN:     // 连接TCP
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
                }
                else if( (bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTOPEN_OK) == TRUE )
                {              
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QMTCONN;
                }
                break;

            case MQTT_TYPE_QMTCONN:     // 建立MQTT连接
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
                }
                else if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTCONN_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QMTPUB;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                else
                {
                    system_ble_lpuart0_send_string(AT_QMTDISC, strlen(AT_QMTDISC));
                    system_ble_lpuart0_send_string(AT_QMTCLOSE, strlen(AT_QMTCLOSE));
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QMTOPEN;
                }
                break;

            case MQTT_TYPE_QMTPUB:       // 上传属性
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
                }
                else if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTPUB_OK) == TRUE )
                {
                    if(_this_pointer._this_product_id_pointer->have_reported == 0 \
                                                    || bootloader_get_fail() == TRUE) // 如果没有上报就上报/或者有没有更新失败
                    {
                        mqtt_state.mqtt_at_type = MQTT_TYPE_QMTSUB;
                    }
                    else    // 上报过就查询一下有没有更新
                    {
                        mqtt_state.mqtt_at_type = MQTT_TYPE_QMTPUB_OTA;
                    }
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                break;

            // 获取OTA相关 ------------------------------------------------------------------
            case MQTT_TYPE_QMTSUB:          // 订阅OTA主动获取
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
                }
                else if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTSUB) == TRUE)
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QMTPUB_VERSION;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                break;

            case MQTT_TYPE_QMTPUB_VERSION:   // 上报版本
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
                }
                else if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTPUB_OK) == TRUE)
                {
                    // 上报成功，存入flash
                    _this_pointer._this_product_id_pointer->have_reported = 1;
                    // 重新存储信息
                    op_manage_verify_version_numbe(NULL, TRUE);

                    if(bootloader_get_fail() != TRUE)
                    {
                        mqtt_state.mqtt_at_type = MQTT_TYPE_QMTPUB_OTA;
                        gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                    }
                    else
                    {
                        // 清除更新失败标志
                        bootloader_clear_fail();
                        // 进入关机流程
                        system_gsm_ready_to_shut_down();
                    }
                }
                break;

            case MQTT_TYPE_QMTPUB_OTA:       // OTA获取
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE  \
                        || (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTPUB_OK) == TRUE)
                {
                    mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;

                    // 判断有没有连带着url链接
                    if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTSUB_OTA) == TRUE)
                    {
                        // 获取URL链接
                        system_gsm_mqtt_get_uart();              
                    }
                }
                else if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QMTSUB_OTA) == TRUE)
                {
                    // 获取URL链接
                    system_gsm_mqtt_get_uart();              
                }
                break;

            // 以下是HTTP下载流程------------------------------------------------------------
            case MQTT_TYPE_QHTTPURL: 
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QHTTPGET;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME * 2;
                }
                break;

            case MQTT_TYPE_QHTTPGET: 
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QHTTPDL;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME;
                }
                break;

            // 保存文件，会返回文件大小
            case MQTT_TYPE_QHTTPDL:  
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
					if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QHTTPDL) == TRUE)
					{
						uint8_t *p = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QHTTPDL);
						p = p + strlen(AT_REPLY_QHTTPDL);    // 移动到这个字串后面
						
						uint32_t temp_ota_bin_size = 0;
						do{
							// 不等于空格、\r、\n、','
							if(*p >= 0x30 && *p <= 0x39)
							{
								temp_ota_bin_size = temp_ota_bin_size * 10 + (*p - '0');
							}
							p++;
						} while (*p >= 0x30 && *p <= 0x39);

						// 设置OTA的文件大小
						ota_set_gsm_bin_size(temp_ota_bin_size);
						
						mqtt_state.mqtt_at_type = MQTT_TYPE_QFOPEN;
						gsm_power_restart_time = GSM_POWER_RESTART_TIME;
					}
					else
					{
						mqtt_state.mqtt_at_type = MQTT_TYPE_QHTTPDL;
						mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
						gsm_power_restart_time = GSM_POWER_RESTART_TIME * 2;
					}
                }
				else if((_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QHTTPDL) == TRUE)
				{
					uint8_t *p = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QHTTPDL);
					p = p + strlen(AT_REPLY_QHTTPDL);    // 移动到这个字串后面
					
					uint32_t temp_ota_bin_size = 0;
					do{
						// 不等于空格、\r、\n、','
						if(*p >= 0x30 && *p <= 0x39)
						{
							temp_ota_bin_size = temp_ota_bin_size * 10 + (*p - '0');
						}
						p++;
					} while (*p >= 0x30 && *p <= 0x39);

					// 设置OTA的文件大小
					ota_set_gsm_bin_size(temp_ota_bin_size);
					
					mqtt_state.mqtt_at_type = MQTT_TYPE_QFOPEN;
					gsm_power_restart_time = GSM_POWER_RESTART_TIME;
				}
                break;

            case MQTT_TYPE_QFOPEN:    
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_OK) == TRUE )
                {
                    // 提取文件位置指针
                    uint8_t *p = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QFOPEN);
                    p = p + strlen(AT_REPLY_QFOPEN);    // 移动到这个字串后面
                    
                    ota_bin_pointer = 0;
                    do{
                        if(*p >= 0x30 && *p <= 0x39)
                        {
                            ota_bin_pointer = ota_bin_pointer * 10 + (*p - '0');
                        }
                        p++;
                    } while (*p >= 0x30 && *p <= 0x39);

                    mqtt_state.mqtt_at_type = MQTT_TYPE_QFREAD;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME * 2;

                    // 先读取文件
                    ota_bin_read_size = READ_GSM_BIN_SIZE;
                }
                else if(ota_bin_pointer != 0)
                {
                    mqtt_state.mqtt_at_type = MQTT_TYPE_QFREAD;
                    gsm_power_restart_time = GSM_POWER_RESTART_TIME * 2;

                    // 先读取文件
                    ota_bin_read_size = READ_GSM_BIN_SIZE;
                }

                break;

            case MQTT_TYPE_QFREAD:   
                if( (_Bool)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QFREAD) == TRUE )
                {
                    uint8_t *bin_head = (uint8_t *)strstr(strM25_Fram_Record.Data_RX_BUF, AT_REPLY_QFREAD); 
                    bin_head = bin_head + strlen(AT_REPLY_QFREAD);  // 位移了8位！跳过开头＋0x20
                    
                    // 开始提取校验码！！！！
                    if(ota_bin_read_size == READ_GSM_BIN_CRC)
                    {
                        // 提取校验码进行升级
                        ota_set_bin_crc(bin_head, READ_GSM_BIN_CRC);
                    }
                    else   // 读bin文件
                    {
                        // 提取bin文件
                        ota_set_bin_file(bin_head);
                    }
                }

                if(ota_bin_url != NULL)
                {
                    free(ota_bin_url);
                    ota_bin_url = NULL;
                }    
                break;

            default:
                break;
        }

        // 清除接收缓存以及计时
        strM25_Fram_Record.InfBit.FramLength = 0;
        strM25_Fram_Record.InfBit.FramFinishFlag = 0;
        irq_time.timer_5_ms_count = 0;
    }

}


/********************************************************************
 ** \brief  MQTT发送处理
 ** @param  无
 ** \retval 无
********************************************************************/
void system_gsm_mqtt_send(void)
{
    if(mqtt_state.mqtt_at_type != MQTT_TYPE_NULL && mqtt_state.mqtt_task_state == MQTT_TYPE_SEND \
                                                    && mqtt_state.mqtt_task_state_count_down == 0)
    {
        // 先清空一下接收的函数
        strM25_Fram_Record.InfBit.FramLength = 0;
        strM25_Fram_Record.InfBit.FramFinishFlag = 0;
        irq_time.timer_5_ms_count = 0;

        switch (mqtt_state.mqtt_at_type)
        {
            case MQTT_TYPE_AT: 
                system_ble_lpuart0_send_string(AT, strlen(AT));
                break;

            case MQTT_TYPE_ATE0:
                system_ble_lpuart0_send_string(ATE0, strlen(ATE0));
                break;

            case MQTT_TYPE_QICSGP:  // APN设置
                system_ble_lpuart0_send_string(AT_QICSGP, strlen(AT_QICSGP));
                break;

            case MQTT_TYPE_CGATT:  // 是否附基站
                system_ble_lpuart0_send_string(AT_CGATT, strlen(AT_CGATT));
                break;

            case MQTT_TYPE_QIREGAPP:  // 执行接入网络
                system_ble_lpuart0_send_string(AT_QIREGAPP, strlen(AT_QIREGAPP));
                break;

            case MQTT_TYPE_QNITZ:  // 同步网络时间
                system_ble_lpuart0_send_string(AT_QNITZ, strlen(AT_QNITZ));
                break;

            case MQTT_TYPE_QLTS:  // 获取网络时间
                system_ble_lpuart0_send_string(AT_QLTS, strlen(AT_QLTS));
                break;

            case MQTT_TYPE_CGDCONT: // PDP
                system_ble_lpuart0_send_string(AT_CGDCONT, strlen(AT_CGDCONT));
                break;

            case MQTT_TYPE_CGACT:  // 激活PDP
                system_ble_lpuart0_send_string(AT_CGACT, strlen(AT_CGACT));
                break;

            case MQTT_TYPE_QIACT:  // 激活移动场景
                system_ble_lpuart0_send_string(AT_QIACT, strlen(AT_QIACT));
                break;

            case MQTT_TYPE_QILOCIP:  // 激活移动场景
                system_ble_lpuart0_send_string(AT_QILOCIP, strlen(AT_QILOCIP));
                break;

            case MQTT_TYPE_QISTAT:  // 查看网络连接状态
                system_ble_lpuart0_send_string(AT_QISTAT, strlen(AT_QISTAT));
                break;

            case MQTT_TYPE_CSQ:  // 获取模块信号
                system_ble_lpuart0_send_string(AT_CSQ, strlen(AT_CSQ));
                break;

            case MQTT_TYPE_VERSIN:  // MQTT版本设置
                system_ble_lpuart0_send_string(AT_QMTCFG_VERSIN, strlen(AT_QMTCFG_VERSIN));
                break;

            case MQTT_TYPE_KEEPALIVE:  // MQTT心跳
                system_ble_lpuart0_send_string(AT_QMTCFG_KEEPALIVE, strlen(AT_QMTCFG_KEEPALIVE));
                break;

            case MQTT_TYPE_ALIAUTH:   // MQTT阿里云参数
            case MQTT_TYPE_QMTOPEN: // TCP连接
            case MQTT_TYPE_QMTCONN: // MQTT 连接
            case MQTT_TYPE_QMTPUB: // MQTT发送信息
            case MQTT_TYPE_QMTPUB_VERSION:   // 上报版本
            case MQTT_TYPE_QMTSUB:           // 订阅OTA主动获取
            case MQTT_TYPE_QMTPUB_OTA:       // OTA获取
                system_gsm_mqtt_send_splicing((enum MQTT_TYPE)mqtt_state.mqtt_at_type);
                break;

            // OTA相关 -----------------------------------------------------------------
            case MQTT_TYPE_QHTTPURL: // 给它链接
                system_gsm_mqtt_send_splicing(MQTT_TYPE_QHTTPURL);
                break;

            case MQTT_TYPE_QHTTPGET:  // 获取链接资源
                system_ble_lpuart0_send_string(AT_QHTTPGET, strlen(AT_QHTTPGET));
                break;

            case MQTT_TYPE_QHTTPDL: // 保存到内存中
                system_ble_lpuart0_send_string(AT_QHTTPDL, strlen(AT_QHTTPDL));
                break;

            case MQTT_TYPE_QFOPEN: // 打开刚保存的bin   
                system_ble_lpuart0_send_string(AT_QFOPEN, strlen(AT_QFOPEN));
                break;

            case MQTT_TYPE_QFREAD: // 读取打开的文件
                system_gsm_mqtt_send_splicing(MQTT_TYPE_QFREAD);
                break;

            default:
                break;
        }

        // 上面发送完毕设置为接收模式
        mqtt_state.mqtt_task_state = MQTT_TYPE_WAIT_RX;
    }
}


// 设置进入关机流程
void system_gsm_ready_to_shut_down(void)
{
    mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_OFF;  // 关机

    system_ble_lpuart0_send_string(AT_QMTDISC, strlen(AT_QMTDISC));
    system_ble_lpuart0_send_string(AT_QMTCLOSE, strlen(AT_QMTCLOSE));

    gsm_power_restart_time = 0;
    mqtt_state.mqtt_unsent_count = 0;
}

// 设置进入重启流程
void system_gsm_ready_to_restart(void)
{
    mqtt_state.mqtt_task_state = MQTT_TYPE_POWER_RESTART;  // 重启

    system_ble_lpuart0_send_string(AT_QMTDISC, strlen(AT_QMTDISC));
    system_ble_lpuart0_send_string(AT_QMTCLOSE, strlen(AT_QMTCLOSE));

    mqtt_state.mqtt_unsent_count = 0;
}


/********************************************************************
 ** \brief  MQTT总流程控制
 ** @param  无
 ** \retval 无
********************************************************************/
void system_gsm_mqtt_manage(void)
{
    // 开机/关机
    system_gsm_enabled_state();

    // 接收控制
    system_gsm_mqtt_receive();
    
    // MQTT发送控制
    system_gsm_mqtt_send();

    // 发送倒计时 暂时取消循环发送的标志位，因为有没有PG都要发送  mqtt_state.mqtt_cyclic_send == TRUE
    if(mqtt_state.mqtt_task_state == MQTT_TYPE_NULL && gsm_power_sending_time_count == 0 && _system_paygo_state.paygo_id != FALSE)
    {
        // 重装MQTT发送事件
        gsm_power_sending_time_count = DEFINE_GSM_POWER_SEND_TIME_COUNT;
   }
}


