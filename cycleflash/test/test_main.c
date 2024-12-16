#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cfs_demo.h"

uint32_t id = 0;
uint8_t data[100] = "abcdefg sdf    "; //27
uint8_t data_1[100] = "q  w  e  a  s  d"; //27
uint8_t data_read[100];
char uart_data[100];

#define NV_DATA_LEN (25u)

void main(void)
{
        uint32_t temp_id = 0;
    uint32_t temp_valid_id = 0;
    bool flag = false;
    
    system_uart0_port_init();
    system_uart0_config_init();
	
	__NOP();
	cfs_demo_init();
   
   sprintf(&uart_data[0], "#### INIT--------------------------------------\r\n");
   system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
   temp_id = cfs_demo_product_current_id_get(); 
   temp_valid_id = cfs_demo_product_current_valid_id_get();
   sprintf(&uart_data[0], "ID = %u: VALID_ID = %u \r\n", temp_id, temp_valid_id);
   system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
   
    sprintf(&uart_data[0], "#### WRITE FLASH--------------------------------------\r\n");
    system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
    memset(&data_read[0], 0, sizeof(data_read));


   // 写入部分 --------------------------------------------------------------
    sprintf(&uart_data[0], "#### WRITE FLASH--------------------------------------\r\n");
    system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
    for(int i=0; i<60; i++)
    {
        memset(&data_read[0], 0, sizeof(data_read));
        if (0 == cfs_demo_write_product_nv(&data[0], NV_DATA_LEN))
        {
            system_ble_uart0_send_string("NULL\r\n", (strlen("NULL\r\n")+1));
            continue;
        }
        
        temp_id = cfs_demo_product_current_id_get(); 
        temp_valid_id = cfs_demo_product_current_valid_id_get();
        sprintf(&uart_data[0], "ID = %u; VALID_ID = %u \r\n", temp_id, temp_valid_id);
        system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
        
        id++;
        if(i == 30)
        {
            __nop();
        }
    }
       
   
    //读取部分 ---------------------------------------------------------------------
    sprintf(&uart_data[0], "#### READ FLASH--------------------------------------\r\n");
    system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    
    temp_id = cfs_demo_product_current_id_get(); 
    temp_valid_id = cfs_demo_product_current_valid_id_get();
    sprintf(&uart_data[0], "ID = %u: VALID_ID = %u \r\n", temp_id, temp_valid_id);
    system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    
    for(uint32_t i=0; i < temp_valid_id; i++)
    {
        memset(&data_read[0], 0, sizeof(NV_DATA_LEN));
        memset(uart_data, 0, sizeof(NV_DATA_LEN));
        if (0 == 
            cfs_demo_read_product_nv(data_read, NV_DATA_LEN, i))
        {
            system_ble_uart0_send_string("NULL\r\n", (strlen("NULL\r\n")+1));
            continue;
        }
        data_read[NV_DATA_LEN] = '\0';

        sprintf(&uart_data[0], "ID = %u, BODY: %s \r\n", (temp_id - i), data_read);
        system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
    }

   
   // 清除内存部分 ----------------------------------------------------------------------
    sprintf(&uart_data[0], "#### CLEAR FLASH--------------------------------------\r\n");
    system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    
    if(cfs_demo_erase_product_nv() == TRUE)
    {
        sprintf(&uart_data[0], ">>> CLEAR SUCCESS\r\n");
    }
    else
    {
        sprintf(&uart_data[0], ">>> CLEAR FAILURE\r\n");
    }
    system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
}

