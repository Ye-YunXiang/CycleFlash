/******************************************************************************
* ProjectName: 	GSM koolboks battery controler version1 projection
* FileName: 	open_paygo_flash.c 
* Description:	Manage OpenPAYGO flash 
* 
* Author:		Poetry Cloud
* Date: 	 	2023/12/25
* Notes:        
* -----------------------------------------------------------------
* Histroy: V1.0 2023/12/25, Cloud create this file
* CPU:HC32L176KATA-LQFP64  FLASH:64K,128页每页512字节
******************************************************************************/

#include "open_paygo_flash.h"
#include <math.h>

/********************************************************************
 ** @brief  选择清除内存页数
 ** @param  enum close_flash_flag 选择要清理数据相对应的页
 ** @retval 无
********************************************************************/
void open_paygo_flash_close_flash(enum close_flash_flag flag)
{
    switch(flag)
    {
        case CLOSE_FLASH_LOCK :
            system_write_flash_lock_close();
            break;
        case CLOSE_FLASH_DATE_ONE :
            system_write_flash_date_one_close();
            break;
        case CLOSE_FLASH_DATE_TOW :
            system_write_flash_date_tow_close();
            break;
        case CLOSE_FLASH_PRODUCT_TIME :
            system_write_flash_product_time_close();
            break;
        case CLOSE_FLASH_PRODUCT_ID :
            system_write_flash_product_id_close();
            break;
    }
}


/********************************************************************
 ** @brief  选择清除内存页数
 ** @param  _Bool TRUE存入上锁时间，FALSE存储错误次数
 ** @retval 无
********************************************************************/
void open_paygo_flash_write_lock_state(__flash_lock * data, _Bool temp_flag)
{
    if(temp_flag == TRUE)
	{
        uint8_t flash_write_sate = 1; 
            
        __flash_lock temp_data;

        do{
            system_write_flash_lock_close();

            system_write_flash_lock_state(data);

            system_read_flash_lock_state(&temp_data);

            flash_write_sate = memcmp(data, &temp_data, sizeof(__flash_lock));
        }
        while(flash_write_sate != 0);
        
	}
	else
	{
		system_write_flash_lock_error(&data->lock_error);
	}
}

/********************************************************************
 ** @brief  读取上锁时间，有数据就赋值给传入的结构体，没有数据就不赋值
 ** @param  __flash_lock 上锁信息的结构体指针
 ** @retval 数据不为0xff返回1，否返回0
********************************************************************/
_Bool open_paygo_flash_read_lock_sate(__flash_lock * data)
{
    uint8_t flash_read_sate = 1; 

    __flash_lock compare_data;
    // 使用 memset 将整个结构体的内存都置为1
    memset(&compare_data, 0xFF, sizeof(compare_data));
    
    __flash_lock temp_data;

    do{
        __flash_lock temp_data_1;

        system_read_flash_lock_state(&temp_data);

        system_read_flash_lock_state(&temp_data_1);

        flash_read_sate = memcmp(&temp_data, &temp_data_1, sizeof(__flash_lock));
    }
    while(flash_read_sate != 0);

    if(memcmp(&temp_data, &compare_data, sizeof(__flash_lock)) == 0)
    {
        return FALSE;
    }
    else
    {
        memcpy(data, &temp_data, sizeof(__flash_lock));

        return TRUE;
    }
}


/********************************************************************
 ** @brief  往flash存入ID
 ** @param  __flash_product_id 结构体指针
 ** @retval 无
********************************************************************/
void open_paygo_flash_write_product_id(__flash_product_id * data)
{
    uint8_t flash_write_sate = 1; 
    
    __flash_product_id temp_data;

    do{
        system_write_flash_product_id_close();

        system_write_flash_product_id_data(data);

        system_read_flash_product_id_data(&temp_data);

        flash_write_sate = memcmp(data, &temp_data, sizeof(__flash_product_id));
    }
    while(flash_write_sate != 0);
}

/********************************************************************
 ** @brief  读取 ID ，有数据就赋值给传入的结构体，没有数据就不赋值
 ** @param  __flash_product_id 结构体指针
 ** @retval 数据不为0xff返回1，否返回0
********************************************************************/
_Bool open_paygo_flash_read_product_id(__flash_product_id * data)
{
    uint8_t flash_read_sate = 1; 

    __flash_product_id compare_data;
    memset(&compare_data, 0xff, sizeof(__flash_product_id));
	
    __flash_product_id temp_data;

    do{
        __flash_product_id temp_data_1;
		
		system_read_flash_product_id_data(&temp_data_1);

        system_read_flash_product_id_data(&temp_data);

        flash_read_sate = memcmp(&temp_data, &temp_data_1, sizeof(__flash_product_id));
    }
    while(flash_read_sate != 0);

    if(memcmp(&temp_data, &compare_data, sizeof(__flash_product_id)) == 0 \
            || temp_data.unit_number_code == 0 \
            || temp_data.unit_number_code == (pow(16,2*sizeof(temp_data.unit_number_code))-1))
    {
        return FALSE;
    }
    else
    {
        memcpy(data, &temp_data, sizeof(__flash_product_id));

        return TRUE;
    }
}


/********************************************************************
 ** @brief  往flash存入结束时间，总可用时间
 ** @param  __flash_date_one 结构体指针
 ** @retval 无
********************************************************************/
void open_paygo_flash_write_date_one(__flash_date_one * data)
{
    uint8_t flash_write_sate = 1; 
    
    __flash_date_one temp_data;

    do{
        system_write_flash_date_one_close();

        system_write_flash_date_one(data);

        system_read_flash_date_one(&temp_data);

        flash_write_sate = memcmp(data, &temp_data, sizeof(__flash_date_one));
    }
    while(flash_write_sate != 0);
}

// 存入有上报版本号标志
void open_paygo_flash_write_product_id_have_reported(uint8_t *data)
{
    system_write_flash_product_id_have_reported(data);
}

/********************************************************************
 ** @brief  读取结束时间、总可用时间 ，有数据就赋值给传入的结构体，没有数据就不赋值
 ** @param  __flash_date_one 结构体指针
 ** @retval 数据不为0xff返回1，否返回0
********************************************************************/
_Bool open_paygo_flash_read_date_one(__flash_date_one * data)
{
    uint8_t flash_read_sate = 1; 
    
    __flash_date_one temp_data;

    do{
        __flash_date_one temp_data_1;

        system_read_flash_date_one(&temp_data);

        system_read_flash_date_one(&temp_data_1);

        flash_read_sate = memcmp(&temp_data, &temp_data_1, sizeof(__flash_date_one));
    }
    while(flash_read_sate != 0);

    if(temp_data.available_time == (pow(16,2*sizeof(temp_data.available_time))-1) \
                    && temp_data.final_date == (pow(16,2*sizeof(temp_data.final_date))-1) )
    {
        data->available_time = 0;

        data->final_date = 0;

        return FALSE;
    }
    else
    {
        memcpy(data, &temp_data, sizeof(__flash_date_one));

        return TRUE;
    }
}



/********************************************************************
 ** @brief  存储间接时间
 ** @param  __flash_date_tow 结构体指针
 ** @retval 无
********************************************************************/
void open_paygo_flash_write_date_tow(__flash_date_tow * data)
{
    uint8_t i;

    uint8_t flash_write_sate = 1; 
    
    __flash_date_tow temp_data;

    do{

        // 计算存储位置（最高位为指示位，如为0，代表已经存满）
        for(i=0; i<7; i++)  // 0~6，对应内存中存储时间位置的7个地址
        {
            if(data->location_date[i] == 255 && i == 0)
            {
                data->location_date[i] = data->location_date[i] << 1;

                system_write_flash_date_tow_close();

                break;
            }
            else if(data->location_date[i] > 128) 
            {
                data->location_date[i] = data->location_date[i] << 1;

                break;
            }
            else if(data->location_date[i] == 128 && i != 6)
            {
                data->location_date[i] = data->location_date[i] << 1;
                i++;
                data->location_date[i] = data->location_date[i] << 1;

                break;
            }
            else if(data->location_date[i] == 128 && i == 6)
            {
                memset(data->location_date, 255, sizeof(data->location_date));     // ##############################

                data->location_date[0] = data->location_date[0] << 1;

                system_write_flash_date_tow_close();
                break;
            }
        }

        system_write_flash_date_tow(data);

        system_read_flash_date_tow(&temp_data, 0);

        flash_write_sate = memcmp(data, &temp_data, sizeof(__flash_date_tow));
    }
    while(flash_write_sate != 0);

    // 存入没问题的标志位
    system_write_flash_date_tow_flag(&data->location_date[0]);
}


/********************************************************************
 ** @brief  读取间接时间
 ** @param  __flash_date_tow 结构体指针
 ** @retval 无
********************************************************************/
void open_paygo_flash_read_date_tow(__flash_date_tow * data)
{
    uint8_t flash_read_sate = 1; 
    
    __flash_date_tow temp_data;

    do{
        __flash_date_tow temp_data_1;

        system_read_flash_date_tow(&temp_data, 1);

        system_read_flash_date_tow(&temp_data_1, 1);

        flash_read_sate = memcmp(&temp_data, &temp_data_1, sizeof(__flash_date_tow));
    }
    while(flash_read_sate != 0);

    memcpy(data, &temp_data, sizeof(__flash_date_tow));
}


/********************************************************************
 ** @brief  存入累计时间、count
 ** @param  __flash_product_time 结构体指针
 ** @retval 无
********************************************************************/
void open_paygo_flash_write_product_time(__flash_product_time * data)
{
    uint8_t flash_write_sate = 1; 
    
    __flash_product_time temp_data;

    do{
        system_write_flash_product_time_close();

        system_write_flash_product_time_data(data);

        system_read_flash_product_time_data(&temp_data);

        flash_write_sate = memcmp(data, &temp_data, sizeof(__flash_product_time));
    }
    while(flash_write_sate != 0);
}

/********************************************************************
 ** @brief  读取可用时间、count ，有数据就赋值给传入的结构体，没有数据就不赋值
 ** @param  __flash_product_time 结构体指针
 ** @retval 数据不为0xff返回1，否返回0
********************************************************************/
_Bool open_paygo_flash_read_product_time(__flash_product_time * data)
{
    uint8_t flash_read_sate = 1; 

    __flash_product_time compare_data;
    // 使用 memset 将整个结构体的内存都置为1
    memset(&compare_data, 0xFF, sizeof(compare_data));
    
    __flash_product_time temp_data;

    do{
        __flash_product_time temp_data_1;

        system_read_flash_product_time_data(&temp_data);

        system_read_flash_product_time_data(&temp_data_1);

        flash_read_sate = memcmp(&temp_data, &temp_data_1, sizeof(__flash_product_time));
    }
    while(flash_read_sate != 0);

    if(memcmp(&temp_data, &compare_data, sizeof(__flash_product_time)) == 0)
    {
        data->add_time = 0;

        data->token_count = 0;

        return FALSE;
    }
    else
    {
        memcpy(data, &temp_data, sizeof(__flash_product_time));

        return TRUE;
    }
}
