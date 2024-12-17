#include "system_uart.h"

struct STRUCT_UART0_Fram UART0_Fram_Record;
 
void system_uart0_port_init(void)
{
	stc_gpio_cfg_t stcGpioCfg;

    DDL_ZERO_STRUCT(stcGpioCfg);

    Sysctrl_SetPeripheralGate(SysctrlPeripheralGpio,TRUE);

    stcGpioCfg.enDir = GpioDirOut;
    Gpio_Init(UART0_MCU_TX_PORT, UART0_MCU_TX_PIN, &stcGpioCfg);
    Gpio_SetAfMode(UART0_MCU_TX_PORT, UART0_MCU_TX_PIN, GpioAf1);

    stcGpioCfg.enDir = GpioDirIn;
    Gpio_Init(UART0_MCU_RX_PORT, UART0_MCU_RX_PIN, &stcGpioCfg);
    Gpio_SetAfMode(UART0_MCU_RX_PORT, UART0_MCU_RX_PIN, GpioAf1);        
}


void system_uart0_config_init(void) 
{
	stc_uart_cfg_t    stcCfg;
    DDL_ZERO_STRUCT(stcCfg);
    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0,TRUE);
    
    stcCfg.enRunMode        = UartMskMode1;
    stcCfg.enStopBit        = UartMsk1bit;   
    stcCfg.stcBaud.u32Baud  = 9600;
    stcCfg.stcBaud.enClkDiv = UartMsk8Or16Div;  
    stcCfg.stcBaud.u32Pclk  = Sysctrl_GetPClkFreq(); 
    Uart_Init(M0P_UART0, &stcCfg);          

    Uart_ClrStatus(M0P_UART0,UartRC); 
    Uart_ClrStatus(M0P_UART0,UartTC);
    Uart_EnableIrq(M0P_UART0,UartRxIrq);             
    Uart_EnableIrq(M0P_UART0,UartTxIrq);            



    if(UART0_Fram_Record.set_up != TRUE)
    {
        UART0_Fram_Record.Data_RX_BUF = NULL;
        UART0_Fram_Record.Data_RX_BUF = (char *)malloc(256);
        memset(&UART0_Fram_Record.Data_RX_BUF[0], 0, sizeof(struct STRUCT_UART0_Fram));

        UART0_Fram_Record.set_up = TRUE;  
    }
}


void system_uart0_config_off(void) 
{
    Uart_ClrStatus(M0P_UART0,UartRC);
    Uart_ClrStatus(M0P_UART0,UartTC);             

    Uart_DisableIrq(M0P_UART0,UartRxIrq);  
    Uart_DisableIrq(M0P_UART0,UartTxIrq);            
    EnableNvic(UART0_2_IRQn, IrqLevel3, FALSE);     

    Sysctrl_SetPeripheralGate(SysctrlPeripheralUart0, FALSE);

    if(UART0_Fram_Record.Data_RX_BUF != NULL)
    {
        memset(&UART0_Fram_Record.Data_RX_BUF[0], 0, sizeof(struct STRUCT_UART0_Fram));
        free(UART0_Fram_Record.Data_RX_BUF);
        UART0_Fram_Record.Data_RX_BUF = NULL;

        UART0_Fram_Record.set_up = FALSE;
    }
	
	Gpio_SetAfMode(UART0_MCU_TX_PORT, UART0_MCU_TX_PIN, GpioAf0);
	Gpio_SetAfMode(UART0_MCU_RX_PORT, UART0_MCU_RX_PIN, GpioAf0);
}


void system_ble_uart0_send_string(char * str, uint16_t len)
{
   uint16_t i;

   for(i=0; i<len; i++)
   {
       Uart_SendDataIt(M0P_UART0,*str); 
       str++;
       delay1ms(1);
   }
}
