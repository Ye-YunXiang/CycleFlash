#include "main_file_define.h"

#include "cycle_flash_system.h"
#include "cfs_DEMO.h"

// 用keil调试，在每个if判断的地方自己打开修改。

uint32_t id = 0;
uint8_t data[27] = "abcdefg                    "; //27
uint8_t data_1[27] = "q  w  e  a  s  d"; //27
uint8_t data_read[28];
char uart_data[100];

int32_t main(void)
{	
    uint32_t temp_id = 0;
    uint32_t temp_valid_id = 0;
    bool flag = false;
    
    system_uart0_port_init();
    system_uart0_config_init();
	
	__NOP();
	flash_filesystem_init();
    
    sprintf(&uart_data[0], "#### INIT--------------------------------------\r\n");
    system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
    temp_id = flash_filesystem_product_current_id_get(); 
    temp_valid_id = flash_filesystem_product_current_valid_id_get();
    sprintf(&uart_data[0], "ID = %d: VALID_ID = %d \r\n", temp_id, temp_valid_id);
    system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    
     sprintf(&uart_data[0], "#### WRITE FLASH--------------------------------------\r\n");
     system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
     memset(&data_read[0], 0, sizeof(data_read));
            flash_filesystem_write_product_nv(id, &data[0], 27);
            temp_id = flash_filesystem_product_current_id_get(); 
            temp_valid_id = flash_filesystem_product_current_valid_id_get();
            sprintf(&uart_data[0], "ID = %d; VALID_ID = %d \r\n", temp_id, temp_valid_id);
            system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));

    // 写入部分
    flag = false;
    __NOP();
    if(flag== true)
    {
        sprintf(&uart_data[0], "#### WRITE FLASH--------------------------------------\r\n");
        system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
        for(int i=0; i<60; i++)
        {
            memset(&data_read[0], 0, sizeof(data_read));
            flash_filesystem_write_product_nv(id, &data[0], 27);
            
            temp_id = flash_filesystem_product_current_id_get(); 
            temp_valid_id = flash_filesystem_product_current_valid_id_get();
            sprintf(&uart_data[0], "ID = %d; VALID_ID = %d \r\n", temp_id, temp_valid_id);
            system_ble_uart0_send_string(uart_data, (strlen(uart_data)+1));
            
            id++;
            if(i == 30)
            {
                __nop();
            }
        }
        
        flash_filesystem_write_product_nv(59, &data_1[0], 27);
        flash_filesystem_write_product_nv(31, &data_1[0], 27);
        flash_filesystem_write_product_nv(46, &data_1[0], 27);
        flash_filesystem_write_product_nv(35, &data_1[0], 27);
        flash_filesystem_write_product_nv(48, &data_1[0], 27);
    }
    
    //读取部分
    flag = false;
    __NOP();
    if(flag== true)
    {
        sprintf(&uart_data[0], "#### READ FLASH--------------------------------------\r\n");
        system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
        
        temp_id = flash_filesystem_product_current_id_get(); 
        temp_valid_id = flash_filesystem_product_current_valid_id_get();
        sprintf(&uart_data[0], "ID = %d: VALID_ID = %d \r\n", temp_id, temp_valid_id);
        system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
        
        for(uint32_t i = (temp_id - temp_valid_id); i <= temp_id; i++)
        {
            memset(&data_read[0], 0, sizeof(data_read));
            memset(uart_data, 0, sizeof(uart_data));
            flash_filesystem_read_product_nv(i, &data_read[0], strlen((char *)data));
            data_read[27] = '\0';
            sprintf(&uart_data[0], "ID = %d, BODY: %s \r\n", i, &data_read[0]);
            system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
        }
    }
    
    // 清除内存部分
    flag = false;
    __NOP();
    if(flag== true)
    {
        sprintf(&uart_data[0], "#### CLEAR FLASH--------------------------------------\r\n");
        system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
        
        if(flash_filesystem_erase_product_nv() == TRUE)
        {
            sprintf(&uart_data[0], ">>> CLEAR SUCCESS\r\n");
        }
        else
        {
            sprintf(&uart_data[0], ">>> CLEAR FAILURE\r\n");
        }
        system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    }
    
    // 删除数据库对象
    flag = false;
    __NOP();
    if(flag== true)
    {
        sprintf(&uart_data[0], "#### DELETE OBJECT--------------------------------------\r\n");
        system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
        
        if(flash_filesystem_product_delete_object() == TRUE)
        {
            sprintf(&uart_data[0], ">>> DELETE SUCCESS\r\n");
        }
        else
        {
            sprintf(&uart_data[0], ">>> DELETE FAILURE\r\n");
        }
        system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    }
    
    // 测试程序结束
    sprintf(&uart_data[0], "#### END OF TEST--------------------------------------\r\n");
    system_ble_uart0_send_string(&uart_data[0], (strlen(uart_data)+1));
    
	while(1)
	{ 
        
	}
}

