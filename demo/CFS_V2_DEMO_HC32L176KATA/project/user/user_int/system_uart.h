#ifndef __SYSTEM_UART_H
#define __SYSTEM_UART_H

#include "main_file_define.h"


extern struct  STRUCT_UART0_Fram
{
	_Bool set_up;		
	char * Data_RX_BUF;
    union 
    {
        __IO uint16_t InfAll;
        struct 
        {
            __IO uint16_t FramLength       :15;
            __IO uint16_t FramFinishFlag   :1;
        } InfBit;
    }; 

} UART0_Fram_Record;


// uart0
void system_uart0_port_init(void);

void system_uart0_config_init(void);

void system_uart0_config_off(void);

void system_ble_uart0_send_string(char * str, uint16_t len);
#endif	
