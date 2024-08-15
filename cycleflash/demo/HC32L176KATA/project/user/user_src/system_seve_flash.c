/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	system_seve_flash.c 
* Description:	initialize FLASH
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes: 
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "system_seve_flash.h"


/********************************************************************
 ** \brief  (必须为uint8，一个字节)清除指定页
 ** @param  清除地址
 ** \retval 无
********************************************************************/
void system_write_flash_close(volatile uint32_t addr)
{
    while(Ok != Flash_SectorErase(addr)){};  
}

/********************************************************************
 ** \brief  (必须为uint8，一个字节)存储数组数据
 ** @param  存储地址
 ** @param  源数据地址
 ** @param  数组长度
 ** \retval 无
********************************************************************/
void system_flash_write_array_data(volatile uint32_t addr, volatile uint8_t * data, uint16_t len)
{
    uint16_t i;
    volatile uint32_t flash_addr = addr;
    
    for(i=0; i< len; i++)
    {
        Flash_WriteByte(flash_addr, *data);
        flash_addr += 0x01;
        data++;
    }
}


/********************************************************************
 ** \brief  (必须为uint8，一个字节)读取数组数据
 ** @param  读取地址
 ** @param  存储数据地址
 ** @param  数组长度
 ** \retval 无
********************************************************************/
void system_flash_read_array_data(volatile uint32_t addr, volatile uint8_t * data, uint16_t len)  
{
    uint16_t i;
    volatile uint32_t flash_addr = addr;
    
    for(i=0; i< len; i++)
    {
        *data = *((uint8_t *)flash_addr);
        flash_addr += 0x01;
        data++;
    }
}
 

/********************************************************************
 ** \brief  计算间隔存储时间地址，
 ** @param  _flash_date_tow的存储时间数组指针
 ** \retval 计算的存储地址
********************************************************************/
uint32_t calculate_interval_date_addr(volatile uint8_t * data)
{
    uint8_t i;
    uint8_t sum = 0;
    volatile uint32_t addr_interval_date = FLASH_ADDR_DATE_INTERVAL_DATE;

     // 处理存储存储逻辑日期地址逻辑
    for(i=0; i<7; i++)
    {
        if((*data == 0x00) && (i != 6))
        {
            sum += 7u;
            data++;
        }
        else
        {
            int j;
            for(j=0; j<8; j++)
            {
                uint8_t bit_j = (*data >> j) & 0x01;

                if( bit_j != 0x00)
                {
                    break;
                }
                sum += 1u;
            }
            break;
        }
    }
	
	sum -= 1;
    addr_interval_date = addr_interval_date + (sum * 8);

    return addr_interval_date;
}

/********************************************************************************
 ** lock  业务逻辑   
 *******************************************************************************/

/********************************************************************
 ** \brief  读取是否上锁、连续输入错误次数、上锁时间
 ** @param  否
 ** \retval 无
********************************************************************/
void system_write_flash_lock_close(void)
{
    while(Ok != Flash_SectorErase(FLASH_ADDR_LOCK_STATE)){};
}


/********************************************************************
 ** \brief  写入flash lock错误次数
 ** @param  _flash_lock的error指针
 ** \retval 无
********************************************************************/
void system_write_flash_lock_error(uint8_t * data)
{
    uint8_t temp_data = 0xff;
    temp_data = temp_data << *data;

    Flash_WriteByte(FLASH_ADDR_LOCK_ERROR, temp_data);
}


/********************************************************************
 ** \brief  写入flash所有_flash_lock
 ** @param  _flash_lock指针
 ** \retval 无
********************************************************************/
void system_write_flash_lock_state(struct flash_lock * data)
{
    uint8_t temp_data = 0xff;

    temp_data = temp_data << data->lock_error;
    Flash_WriteByte(FLASH_ADDR_LOCK_ERROR, temp_data);

    Flash_WriteHalfWord(FLASH_ADDR_LOCK_DATE, data->lock_date);
}


/********************************************************************
 ** \brief  读取flash所有_flash_lock
 ** @param  _flash_lock指针
 ** \retval 无
********************************************************************/
void system_read_flash_lock_state(struct flash_lock * data)
{
    uint8_t temp_data = 0;
    uint8_t i = 0;

    temp_data = *((uint8_t *)FLASH_ADDR_LOCK_ERROR);

    do{
        if(temp_data == 0)
        {
            i = 0;
            break;
        }
		
		uint8_t temp_bit = temp_data&0x01;
        if( temp_bit == 0)
        {
            i++;
            temp_data = temp_data >> 1;
        }
        else
        {
            break;
        }
    }
    while(1);
    
    data->lock_error = i;
    data->lock_date  = *((uint16_t *)FLASH_ADDR_LOCK_DATE); 

    if(data->lock_date == 0 || data->lock_date >= 4000)
    { 
        data->lock_date = 0;
    }
}

/********************************************************************************
 ** date  业务逻辑   
 *******************************************************************************/

/********************************************************************
 ** \brief  清除页252_flash_date_one
 ** @param  无
 ** \retval 无
********************************************************************/
void system_write_flash_date_one_close(void)
{
    while(Ok != Flash_SectorErase(FLASH_ADDR_DATE_FINAL_DATE)){};      // 252
}


/********************************************************************
 ** \brief  存储flash结束时间、总可用时间
 ** @param  _flash_date_one指针
 ** \retval 无
********************************************************************/
void system_write_flash_date_one(struct flash_date_one * data)
{
    // 写入一个字，时间戳
    Flash_WriteWord(FLASH_ADDR_DATE_FINAL_DATE, data->final_date);

    // 写入一个字，总可用时间
    Flash_WriteWord(FLASH_ADDR_DATE_SUM_AVAILABLE_TIME, data->available_time);
}


/********************************************************************
 ** \brief  读取flash内存中结束时间、总可用时间
 ** @param  _flash_date_one指针
 ** \retval 无
********************************************************************/
void system_read_flash_date_one(struct flash_date_one * data)
{
    // 读取时间戳
    data->final_date = *((uint32_t *)FLASH_ADDR_DATE_FINAL_DATE);

    // 读取半个字
    data->available_time = *((uint32_t *)FLASH_ADDR_DATE_SUM_AVAILABLE_TIME);
}


/********************************************************************
 ** \brief  清除页253
 ** @param  无
 ** \retval 无
********************************************************************/
void system_write_flash_date_tow_close(void)
{
    while(Ok != Flash_SectorErase(FLASH_ADDR_DATE_LOCATION_DATE)){};   // 253
}


/********************************************************************
 ** \brief  存储间接存储时间标志位
 ** @param  1
 ** \retval 无
********************************************************************/
void system_write_flash_date_tow_flag(uint8_t * add)
{
    volatile uint32_t addr_interval_date = 0;

    addr_interval_date = calculate_interval_date_addr(add);
    addr_interval_date += 4u;

    Flash_WriteByte(addr_interval_date, 0x01);
}


/********************************************************************
 ** \brief  存储间接存储地址、间接存储时间
 ** @param  _flash_date_tow指针
 ** \retval 无
********************************************************************/
void system_write_flash_date_tow(struct flash_date_tow * data)
{
    volatile uint32_t addr_interval_date = 0;

    system_flash_write_array_data(FLASH_ADDR_DATE_LOCATION_DATE,\
                                    &data->location_date[0], sizeof(data->location_date));

    addr_interval_date = calculate_interval_date_addr(data->location_date);

    // 写入时间戳
    Flash_WriteWord(addr_interval_date, data->interval_date);
}


/********************************************************************
 ** \brief  读取间接存储地址、间接存储时间
 ** @param  _flash_date_tow指针
 ** @param  是否验证有标志位TRUE/FALSE
 ** \retval 无
********************************************************************/
void system_read_flash_date_tow(struct flash_date_tow * data, _Bool temp_flag)
{
    uint8_t flash_read_state = 1;
    uint8_t interval_date_flag;
    volatile uint32_t addr_interval_date = 0;
    volatile uint32_t addr_interval_date_temp;

    system_flash_read_array_data(FLASH_ADDR_DATE_LOCATION_DATE,\
                                    &data->location_date[0], sizeof(data->location_date));

    addr_interval_date = calculate_interval_date_addr(data->location_date);

    // 读取时间，是否验证标志位，如果没有标志位就往后退
    if(temp_flag)
    {
        do{ 
            addr_interval_date_temp = addr_interval_date + 4u;
            interval_date_flag = *((uint8_t *)addr_interval_date_temp);

            if( interval_date_flag == 0xff )
            {
                if(addr_interval_date != FLASH_ADDR_DATE_INTERVAL_DATE)
                {
                    addr_interval_date -= 8; 
                }
                else
                {
                    data->interval_date = *((uint32_t *)addr_interval_date);
                    flash_read_state = 0;
                }
            }
            else
            {
                data->interval_date = *((uint32_t *)addr_interval_date);
                flash_read_state = 0;
            }
        }
        while(flash_read_state != 0);
    }
    else
    {
        data->interval_date = *((uint32_t *)addr_interval_date);
    }
}

/********************************************************************************
 ** product time  业务逻辑   
 *******************************************************************************/

/********************************************************************
 ** \brief  清除页254
 ** @param  无
 ** \retval 无
********************************************************************/
void system_write_flash_product_time_close()
{
    while(Ok != Flash_SectorErase(FLASH_ADDR_PRODUCT_TIME_ADD_TIME)){};
}


/********************************************************************
 ** \brief  一次写入累计时间和count数据
 ** @param  _flash_product_time指针
 ** \retval 无
********************************************************************/
void system_write_flash_product_time_data(struct flash_product_time * data)
{
    // 数据都是半字长度的
    Flash_WriteHalfWord(FLASH_ADDR_PRODUCT_TIME_ADD_TIME, data->add_time);

    Flash_WriteHalfWord(FLASH_ADDR_PRODUCT_TIME_TOKEN_COUNT, data->token_count);
}


/********************************************************************
 ** \brief  一次读取累计时间和count数据
 ** @param  _flash_product_time指针
 ** \retval 无
********************************************************************/
void system_read_flash_product_time_data(struct flash_product_time * data)
{
    data->add_time = *((uint16_t *)FLASH_ADDR_PRODUCT_TIME_ADD_TIME);
    data->token_count = *((uint16_t *)FLASH_ADDR_PRODUCT_TIME_TOKEN_COUNT);
}

/********************************************************************************
 ** product id  业务逻辑   
 *******************************************************************************/
 
/********************************************************************
 ** \brief  清除页255
 ** @param  无
 ** \retval 无
********************************************************************/
void system_write_flash_product_id_close(void)
{
    while(Ok != Flash_SectorErase(FLASH_ADDR_PRODUCT_ID_STARTING_CODE)){};
}


/********************************************************************
 ** \brief  一次写入ID数据
 ** @param  _flash_product_id指针
 ** \retval 无
********************************************************************/
void system_write_flash_product_id_data(struct flash_product_id * data)
{
    system_flash_write_array_data(FLASH_ADDR_PRODUCT_ID_SECRET_KEY, \
                                    &data->secret_key[0], sizeof(data->secret_key));

    Flash_WriteWord(FLASH_ADDR_PRODUCT_ID_STARTING_CODE, data->starting_code);

    Flash_WriteWord(FLASH_ADDR_PRODUCT_ID_UNIT_NUMBER_CODE, data->unit_number_code);

    // 写入MQTT
    system_flash_write_array_data(FLASH_ADDR_PRODUCT_ID_DEVICE_SECRET, \
                                    &data->device_secret[0], sizeof(data->device_secret));

    system_flash_write_array_data(FLASH_ADDR_PRODUCT_ID_PRODUCT_KEY, \
                                    &data->product_key[0], sizeof(data->product_key));

    // 写入OTA相关的
    Flash_WriteByte(FLASH_ADDR_PRODUCT_ID_HAVE_REPORTED, data->have_reported);
    system_flash_write_array_data(FLASH_ADDR_PRODUCT_ID_VERSION, \
                                    &data->version[0], sizeof(data->version));
}


/********************************************************************
 ** \brief  一次读取ID数据
 ** @param  _flash_product_id指针
 ** \retval 无
********************************************************************/
void system_read_flash_product_id_data(struct flash_product_id * data)
{
    system_flash_read_array_data(FLASH_ADDR_PRODUCT_ID_SECRET_KEY,\
                                    &data->secret_key[0], sizeof(data->secret_key));

    data->starting_code = *((uint32_t *)FLASH_ADDR_PRODUCT_ID_STARTING_CODE);
    data->unit_number_code = *((uint32_t *)FLASH_ADDR_PRODUCT_ID_UNIT_NUMBER_CODE);

    // 读出MQTT
    system_flash_read_array_data(FLASH_ADDR_PRODUCT_ID_DEVICE_SECRET, \
                                    &data->device_secret[0], sizeof(data->device_secret));

    system_flash_read_array_data(FLASH_ADDR_PRODUCT_ID_PRODUCT_KEY, \
                                    &data->product_key[0], sizeof(data->product_key));

    // 读取OTA相关
    data->have_reported = *((uint8_t *)FLASH_ADDR_PRODUCT_ID_HAVE_REPORTED);
    system_flash_read_array_data(FLASH_ADDR_PRODUCT_ID_VERSION, \
                                    &data->version[0], sizeof(data->version));
}

/********************************************************************
 ** \brief  写入有上报数据
 ** @param  uint8_t 1或0
 ** \retval 无
********************************************************************/
void system_write_flash_product_id_have_reported(uint8_t *data)
{
    Flash_WriteByte(FLASH_ADDR_PRODUCT_ID_HAVE_REPORTED, *data);
}
