#ifndef _SYSTEM_GPIO_DEFINE_H_
#define _SYSTEM_GPIO_DEFINE_H_

#include "gpio.h"


/*-- M26部分 ---------------------------------*/
#define GSM_EN_PORT                 (GpioPortA)
#define GSM_EN_PIN                  (GpioPin11)

#define GSM_PWRKEY_PORT             (GpioPortB)
#define GSM_PWRKEY_PIN              (GpioPin12)

/*-- UART部分 ---------------------------------*/
#define UART0_MCU_TX_PORT           (GpioPortA)
#define UART0_MCU_TX_PIN            (GpioPin9)

#define UART0_MCU_RX_PORT           (GpioPortA)
#define UART0_MCU_RX_PIN            (GpioPin10)

#define LPUART0_MCU_TX1_PORT        (GpioPortB)
#define LPUART0_MCU_TX1_PIN         (GpioPin10)

#define LPUART0_MCU_RX1_PORT        (GpioPortB)
#define LPUART0_MCU_RX1_PIN         (GpioPin11)

/*-- 键盘部分 ---------------------------------*/
#define	KEY_R1_PORT		    	    (GpioPortB)
#define	KEY_R1_PIN		    	    (GpioPin9)

#define	KEY_R2_PORT		    	    (GpioPortB)
#define	KEY_R2_PIN		    	    (GpioPin8)

#define	KEY_R3_PORT		    	    (GpioPortB)
#define	KEY_R3_PIN		    	    (GpioPin7)

#define KEY_R4_PORT		    	    (GpioPortB)
#define	KEY_R4_PIN	    		    (GpioPin6)

#define	KEY_C1_PORT		    	    (GpioPortB)
#define	KEY_C1_PIN		    	    (GpioPin5)

#define	KEY_C2_PORT	    		    (GpioPortB)
#define	KEY_C2_PIN	    		    (GpioPin4)

#define	KEY_C3_PORT		    	    (GpioPortB)
#define	KEY_C3_PIN	    		    (GpioPin3)

/*-- LCD部分 ---------------------------------*/
#define LCDRS_PORT                  (GpioPortA)	
#define LCDRS_PIN                   (GpioPin1)

#define LCDRW_PORT	    	        (GpioPortA)		
#define LCDRW_PIN                   (GpioPin2)

#define LCDEN_PORT		            (GpioPortA)		
#define LCDEN_PIN		            (GpioPin3)

#define LCDD4_PORT	    	        (GpioPortA)		
#define LCDD4_PIN	    	        (GpioPin4)

#define LCDD5_PORT	    	        (GpioPortA)		
#define LCDD5_PIN	    	        (GpioPin5)

#define LCDD6_PORT	    	        (GpioPortA)			
#define LCDD6_PIN	    	        (GpioPin6)

#define LCDD7_PORT	    	        (GpioPortA)			
#define LCDD7_PIN   		        (GpioPin7)

#define	LCD_LIGHT_PORT	            (GpioPortB)		
#define LCD_LIGHT_PIN	            (GpioPin1)

/*-- LED灯部分 ---------------------------------*/
#define LED_RED_PORT                (GpioPortB)
#define LED_RED_PIN                 (GpioPin0)

#define LED_GREEN_PORT              (GpioPortB)
#define GREEN_PIN                   (GpioPin2)

/*-- 冰箱控制部分 ---------------------------------*/
#define REFRIGERATOR_PORT           (GpioPortF)
#define REFRIGERATOR_PIN            (GpioPin6)

/*-- ADC检测部分 ---------------------------------*/
#define ADC_24V_PORT                (GpioPortA)     
#define ADC_24V_PIN                 (GpioPin0)      // 外部供电

#define ADC_BAT_PORT                (GpioPortB)
#define ADC_BAT_PIN                 (GpioPin14)     // 小电池低功耗供电

#define ADC_TEMP_PORT               (GpioPortB)
#define ADC_TEMP_PIN                (GpioPin15)     // 温度检测

#define BAT_ADC_EN_PORT             (GpioPortA)     
#define BAT_ADC_EN_PIN              (GpioPin12)     // 打开ADC检测

#define ADC_5V_PORT                 (GpioPortA)
#define ADC_5V_PIN                  (GpioPin8)      // 外部电源输入检测 < 电平检测 >

#define ADC_TAMPER_PORT             (GpioPortA)
#define ADC_TAMPER_PIN              (GpioPin15)     // 后盖打开检测     < 电平检测 >

/*-- 蓝牙控制部分 ---------------------------------*/
#define BT_STATE_PORT               (GpioPortB)     
#define BT_STATE_PIN                (GpioPin13)      // 蓝牙状态检测

#define BT_EN_PORT                  (GpioPortC)     
#define BT_EN_PIN                   (GpioPin13)      // 打开蓝牙供电

#define BT_TX_PORT                  (GpioPortF)     
#define BT_TX_PIN                   (GpioPin0)   

#define BT_RX_PORT                  (GpioPortF)     
#define BT_RX_PIN                   (GpioPin1)      



#endif
