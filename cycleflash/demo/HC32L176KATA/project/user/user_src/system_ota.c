#include "system_ota.h"

/*************************************************************************
 * 分配地址了,暂定
 * bootloader 9.5k sector0~29     0x00 ~ 0x25FF
 * bootloader data                0x2600 ~ 0x27FF   // 用来存储关于OTA的数据
 * APP1       40k sector20~99     0x2800 ~ 0xC7FF
 * APP2       40k sector100~179   0xC800 ~ 0x167FF
**/

// bootloader 头文件地址
#define BOOTLOADER_ADDR                         (0x0)
#define BOOTLOADER_SIZE                         9728   // 9.5k
#define BOOTLOADER_DATA_ADDR                    (0x2600)
#define BOOTLOADER_DATA_SIZE                    512    // 512byte

#define BOOTLOADER_DATA_APP_ADDR                (0x2610)   // APP   0/0xFF 为APP1，1 为APP2
// 以下的都不准！！！！
// #define BOOTLOADER_DATA_SWITCHOVER_ADDR         (0x2611)   // 是否切换APP  0/0xFF 不切换, 1 为切换
// #define BOOTLOADER_DATA_HANDOVER_PERFORMED_ADDR (0x2612)   // 是否已经切换APP  0/0xFF 没有切换, 1 为已经切换
// #define BOOTLOADER_DATA_FAIL_ADDR               (0x2613)   // 是否更新失败  0/0xFF 没问题, 1 为失败

// APP1 头文件地址  
#define APP_1_ADDR                              (0x2800)
#define APP_1_SIZE                              40960   // 40k
// APP下载缓存区
#define APP_2_ADDR                              (0xC800)
#define APP_2_SIZE                              40960   // 40k

// 下面结构体采用紧凑型的方式存储在内存
#pragma pack(1)
struct SYSTEM_OTA
{
	uint8_t app;						// 没啥用，用来作为结构体头
    uint8_t switchover;                 // 是否切换
    uint8_t handover_performed;         // 是否已经切换
    uint8_t fail;                       // 更新失败标志位  
};
#pragma pack()
// bootloader存储的信息
struct SYSTEM_OTA __system_ota = {0};

struct OTA_UPGRADE_VALUE
{
    // 下载的文件大小
    uint32_t ota_bin_size ;
    // bin文件存储地址偏移量
    uint32_t ota_bin_move;
    // bin文件校验码
    uint16_t ota_bin_crc;
};
struct OTA_UPGRADE_VALUE __ota_upgrade = {0};


/*--------------------------------------------------------------------
 *---系统复位
*---------------------------------------------------------------------*/
void __OTA_SYSTEM_RESTART(void)
{
    NVIC_SystemReset();

    while(1)
    {
        __NOP();
    }
}

/*---------------------------------------------------------------------------------------------------
 *---初始化设置
*--------------------------------------------------------------------------------------------------*/
// 读取 bootloader内容 ,结构体一定要字节对齐！！！
void bootloader_read(void)
{
    uint8_t flash_read_sate = 1; 
    
    struct SYSTEM_OTA temp_data;

    do{
        struct SYSTEM_OTA temp_data_1;

        system_flash_read_array_data(BOOTLOADER_DATA_APP_ADDR, &temp_data_1.app, sizeof(struct SYSTEM_OTA));

        system_flash_read_array_data(BOOTLOADER_DATA_APP_ADDR, &temp_data.app, sizeof(struct SYSTEM_OTA));

        flash_read_sate = memcmp(&temp_data.app, &temp_data_1.app, sizeof(struct SYSTEM_OTA));
    }
    while(flash_read_sate != 0);

    memcpy(&__system_ota, &temp_data, sizeof(struct SYSTEM_OTA));
}

// 写入bootloader内容  结构体一定要字节对齐！！！
void bootloader_write(void)
{
    uint8_t flash_write_sate = 1; 
    
    struct SYSTEM_OTA temp_data;

    do{
        system_write_flash_close(BOOTLOADER_DATA_APP_ADDR);

        system_flash_write_array_data(BOOTLOADER_DATA_APP_ADDR, &__system_ota.app, sizeof(struct SYSTEM_OTA));

        system_flash_read_array_data(BOOTLOADER_DATA_APP_ADDR, &temp_data.app, sizeof(struct SYSTEM_OTA));  

        flash_write_sate = memcmp(&__system_ota, &temp_data, sizeof(struct SYSTEM_OTA));
    }
    while(flash_write_sate != 0);
}

// 处理 bootloader内的信息
// 这里就是要判断是否有升级，是否有切换新程序等等
void bootloader_manage(void)
{
    if(__system_ota.switchover == TRUE && __system_ota.handover_performed == TRUE)
    {
        // 如果切换成功就清除标志位，否者回到bootloader发现标志位还在是会回档的
        __system_ota.switchover = FALSE;
        __system_ota.handover_performed = FALSE;

        bootloader_write();
    }
}

// 这个感觉没必要，因为就不会留到这里，但是以防万一，留着吧。
// 清除所有可能会发送内存泄漏的全局动态内
void ota_close_global_interrupt(void)
{
	system_uart0_config_off();

    // 存入计时状态下的间接时间
    op_manage_write_date_tow(NULL);

}

/*---------------------------------------------------------------------------------------------------
 *---OTA 文件验证处理
*-------------------------------------------------------------------------------------------------*/
/********************************************************************
 ** \brief  (必须为uint8，一个字节)存入bin文件的数据
 ** @param  源数据地址
 ** @param  数组长度
 ** @param  地址偏移
 ** \retval 无
********************************************************************/
void ota_write_bin_data(uint8_t * data, uint16_t len, uint32_t addr_move)
{
    // 直接进缓存区
    uint32_t write_addr = APP_2_ADDR + addr_move;

    uint8_t flash_write_sate = 1; 

    do{
        system_write_flash_close(write_addr);

        system_flash_write_array_data(write_addr, data, len);

        flash_write_sate = memcmp(data, ((uint8_t *)write_addr), len);
    }
    while(flash_write_sate != 0);
}


/********************************************************************
 ** \brief  校验bin文件
 ** \retval 无
********************************************************************/
//@@@@@ 校验文件和切换程序 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void ota_bin_checkout(void)
{
    // 直接验证缓存区
    volatile uint32_t write_addr = APP_2_ADDR;

    // 验证
    uint16_t crc = CRC_16_XMODEM(0,0x1021,0,false,false, ((uint8_t *)write_addr), __ota_upgrade.ota_bin_size);

   if(crc == __ota_upgrade.ota_bin_crc && __ota_upgrade.ota_bin_size > 1024)     // 校验通过
   {
        __system_ota.switchover = TRUE;

        bootloader_write();
	   
	    // 关闭所有使用到中断的服务
	 	ota_close_global_interrupt();

        // 软件重启
        __OTA_SYSTEM_RESTART();
   }
   else   // 校验失败
   {
        __system_ota.fail = TRUE;
        bootloader_write();

        // 设置进入重启流程
        system_gsm_ready_to_restart();
   }
}


// 设置bin文件校验码
void ota_set_bin_crc(uint8_t * bin_head, uint8_t crc_length)
{
    uint8_t i = 0;
    // 指针先跳过这个区域根据不同模块回复的消息进行适配
    do{
        bin_head++;
    } while (*bin_head != 0x0a && *bin_head != 0x0d && *bin_head != 0x20);
    // 跳过\r\n
    bin_head = bin_head + 2;

    __ota_upgrade.ota_bin_crc = 0;

    // 开始提取校验码！！！！
    for(i = 0; i < crc_length; i++)
    {
        __ota_upgrade.ota_bin_crc = (__ota_upgrade.ota_bin_crc * 0x100) + *bin_head;
        bin_head++;
    }

    // 实际bin文件去掉校验码大小
    __ota_upgrade.ota_bin_size = __ota_upgrade.ota_bin_size - crc_length;

    // 开始校验升级
    ota_bin_checkout();
}


// 获取bin文件
void ota_set_bin_file(uint8_t * bin_head)
{
    uint16_t temp_bin_size = 0;
    do{
        // 不等于空格、\r、\n
        if(*bin_head != 0x0a && *bin_head != 0x0d && *bin_head != 0x20)
        {
            temp_bin_size = temp_bin_size * 10 + (*bin_head - '0');
        }
        bin_head++;
    } while (*bin_head != 0x0a && *bin_head != 0x0d && *bin_head != 0x20);
    // 跳过\r\n
    bin_head = bin_head + 2;

    if(temp_bin_size != 0)   // 存入bin文件
    {
        // 开始获取bin文件  // 创建一个和获取文件一样大的动态空间
        uint8_t *temp_bin_file = (uint8_t *)malloc(temp_bin_size);
        memcpy(temp_bin_file, bin_head, temp_bin_size);
        
        ota_write_bin_data(temp_bin_file, temp_bin_size, __ota_upgrade.ota_bin_move);
        __ota_upgrade.ota_bin_move += temp_bin_size;
        
        free(temp_bin_file);
    }
    else   // 读完了bin文件
    {
        ota_bin_checkout();
    }
}

// CRC校验拿到的文件
// 参数: 初始值、多项式、结果异或值、输入反转、输出反转、起始指针、数据长度
// 参数中前面的要查表，后面两个数据才是自己给的
// 这里使用的是XMODEM的标准：ota_bin_crc_check(0,0x1021,0,false,false,puchMsg,usDataLen)
uint16_t ota_bin_crc_check(uint16_t wCRCin, uint16_t wCPoly, uint16_t wResultXOR, \
                            _Bool input_invert, _Bool ouput_invert, const uint8_t *puchMsg, uint32_t usDataLen)
{
    uint8_t wChar = 0;
    while (usDataLen--)
    {
        wChar = *(puchMsg++);
        if(input_invert)//输入值反转
        {
            uint8_t temp_char = wChar;
            wChar = 0;
            for(int i = 0; i < 8; ++i)
            {
                if(temp_char&0x01)
                    wChar |= 0x01<<(7-i);
                temp_char>>=1;
            }
        }
        wCRCin ^= (wChar << 8);
        for (int i = 0; i < 8; i++)
        {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    if(ouput_invert)
    {
        uint16_t temp_short = wCRCin;
        wCRCin=0;
        for(int i=0;i<16;++i)
        {
            if(temp_short&0x01)
                wCRCin|=0x01<<(15-i);
            temp_short>>=1;
        }
    }
    return (wCRCin^wResultXOR);
}

// 设置下载的bin文件大小
void ota_set_gsm_bin_size(uint32_t size)
{
    __ota_upgrade.ota_bin_size = size;
    // 偏移量指令
    __ota_upgrade.ota_bin_move = 0;
}

/*---------------------------------------------------------------------------------------------
 *--get end set bootloader 的数据
*--------------------------------------------------------------------------------------------*/
// 拿到偏移量
uint32_t ota_get_ota_bin_move(void)
{
    return __ota_upgrade.ota_bin_move;
}

// 得到文件大小
uint32_t ota_get_ota_bin_size(void)
{
    return __ota_upgrade.ota_bin_size;
}

// 获得是否已经切换程序
uint8_t bootloader_get_handover_performed(void)
{
    return __system_ota.handover_performed;
}

// 获得是否更新失败
uint8_t bootloader_get_fail(void)
{
    return __system_ota.fail;
}

// 清除更新失败标志
void bootloader_clear_fail(void)
{
    __system_ota.fail = FALSE;

    bootloader_write();
}
