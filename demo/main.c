// 260604：这里在STM32平台上测试通过。。。

/*
 * 测试用例总结：
 * - 初始化：调用 cfs_demo_init()，读取当前 product id 与有效记录数（valid_id）。
 * - 写入测试：执行 60 次写入（cfs_demo_write_product_nv），并打印每次的 ID、VALID_ID 与写入地址；
 *   用于验证环形闪存的写入、ID 自增与回环/覆盖行为。
 * - 读取测试：按索引读取 0..valid_id-1 的记录（cfs_demo_read_product_nv），打印 ID、地址与数据；
 *   用于验证读出顺序与数据完整性。
 * - 擦除测试：调用 cfs_demo_erase_product_nv()，检查返回值以验证擦除成功。
 * - 注意：NV_DATA_LEN = 25；程序通过 UART 打印结果供人工观察；测试结束后进入无限循环。
 * - 这里原先是基于一页flash：512进行测试。
 * - 适配了1字节、2字节、4字节的写入。
 */
#include "main.h"

#include <string.h>
#include <stdbool.h>

#include "bsp_usart.h"

#include "cfs_DEMO.h"

#include "cycle_flash_system.h"


#define TRUE  1
#define FALSE 0

void * comm = NULL;

/**
 * 这里两个全局变量，需要安插到文件 cycle_flash_system.c中的两个函数获取计算的地址。
 * cfs_nv_write：获取写入计算的地址。
 * cfs_nv_read：获取读取计算的地址。
*/
uint32_t addr_read = 0;
uint32_t addr_write = 0;

uint32_t id = 0;
uint8_t data[100] = "abcdefg sdf    "; //27
uint8_t data_1[100] = "q  w  e  a  s  d"; //27
uint8_t data_read[100];
char uart_data[100];

#define NV_DATA_LEN (25u)
#define __UART_PRINT(ptr, len)  comm_block_send_data_string(comm, ptr, len)


int32_t main(void)
{	
	SystemCoreClockUpdate();
	comm = bsp_uart_interface(E_BSP_UART_CHANNEL_UART1);
	comm_init(comm, 9600);  // ble默认波特率
	
    uint32_t temp_id = 0;
    uint32_t temp_valid_id = 0;

    // 初始化uart相关-----------

    // 测试逻辑 ------------------------
    
    cfs_demo_init();
   
    sprintf(&uart_data[0], "#### INIT--------------------------------------\r\n");
    __UART_PRINT(uart_data, (strlen(uart_data)+1));
    temp_id = cfs_demo_product_current_id_get(); 
    temp_valid_id = cfs_demo_product_current_valid_id_get();
    sprintf(&uart_data[0], "ID = %u: VALID_ID = %u \r\n", temp_id, temp_valid_id);
    __UART_PRINT(&uart_data[0], (strlen(uart_data)+1));

    sprintf(&uart_data[0], "#### WRITE FLASH--------------------------------------\r\n");
    __UART_PRINT(uart_data, (strlen(uart_data)+1));
    memset(&data_read[0], 0, sizeof(data_read));

    // 写入部分 -----------------------------------------------------------------
    sprintf(&uart_data[0], "#### WRITE FLASH--------------------------------------\r\n");
    __UART_PRINT(uart_data, (strlen(uart_data)+1));
    for(int i=0; i<60; i++)
    {
        addr_write = 0;
        memset(&data_read[0], 0, sizeof(data_read));
        if (0 == cfs_demo_write_product_nv(&data[0], NV_DATA_LEN))
        {
            __UART_PRINT("NULL\r\n", (strlen("NULL\r\n")+1));
            continue;
        }
        
        temp_id = cfs_demo_product_current_id_get(); 
        temp_valid_id = cfs_demo_product_current_valid_id_get();
        sprintf(&uart_data[0], "ID = %u; VALID_ID = %u; ADDR = %X\r\n", temp_id, temp_valid_id, addr_write);
        __UART_PRINT(uart_data, (strlen(uart_data)+1));
        
        id++;
        if(i == 30)
        {
            __nop();
        }
    }
       
   //读取部分 ---------------------------------------------------------------
   sprintf(&uart_data[0], "#### READ FLASH--------------------------------------\r\n");
   __UART_PRINT(&uart_data[0], (strlen(uart_data)+1));

   temp_id = cfs_demo_product_current_id_get(); 
   temp_valid_id = cfs_demo_product_current_valid_id_get();
   sprintf(&uart_data[0], "ID = %u: VALID_ID = %u\r\n", temp_id, temp_valid_id);
   __UART_PRINT(&uart_data[0], (strlen(uart_data)+1));
   
   for(uint32_t i=0; i < temp_valid_id; i++)
   {
       addr_read = 0;
       memset(&data_read[0], 0, sizeof(NV_DATA_LEN));
       memset(uart_data, 0, sizeof(NV_DATA_LEN));
       if (0 == 
           cfs_demo_read_product_nv(data_read, NV_DATA_LEN, i))
       {
           __UART_PRINT("NULL\r\n", (strlen("NULL\r\n")+1));
            continue;
       }
       data_read[NV_DATA_LEN] = '\0';

       sprintf(&uart_data[0], "ID = %u, ADDR = %X BODY: %s \r\n", (temp_id - i), addr_read, data_read);
       __UART_PRINT(uart_data, (strlen(uart_data)+1));
   }

   
   // 清除内存部分 ---------------------------------------------------------
   sprintf(&uart_data[0], "#### CLEAR FLASH--------------------------------------\r\n");
   __UART_PRINT(&uart_data[0], (strlen(uart_data)+1));
   
   if(cfs_demo_erase_product_nv() == TRUE)
   {
       sprintf(&uart_data[0], ">>> CLEAR SUCCESS\r\n");
   }
   else
   {
       sprintf(&uart_data[0], ">>> CLEAR FAILURE\r\n");
   }
   __UART_PRINT(&uart_data[0], (strlen(uart_data)+1));
   


   for(;;)
   {
        
   }

   return 0;
}
