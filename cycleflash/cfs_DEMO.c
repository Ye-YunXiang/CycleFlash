/*
 * This file is part of the CycleFlash Library.
 *
 * Copyright (c) 2024, YeYunXiang, <poetrycloud@foxmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the definitions head file for this library.
 * Created on: 2024-7-26
 */

#include <assert.h>
#include "cycleflash.h"
// nexus�Ŀ����ļ�
#include "nx_common.h"

// flashд���豸״̬��ַ��ʼ
#define FLASH_ADDR_PRODUCT_NV_ID_MANAGER						(0x00012000u)	//62 sector
// д���豸ʣ����������ַ��ʼ
#define FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING		(0x00012200u)	
// д��nexus״̬��ʼ
#define FLASH_ADDR_NEXUS_NV_START									(0x0001C000u)

// ���ڹ���洢nexus����Ϣ��ר������Nexus��NV��д�����Ŀռ䡣
static cfs_system nexus_filesystem;
// ���ڹ���洢����������Ϣ������ʣ�������������ܹ�������֮��ġ�
static cfs_system product_filesystem;
// ���ڹ���洢�豸�������Ϣ��ר������洢�豸ID֮�ࡣ
static cfs_system identity_filesystem;

static bool filesystem_success_init = false;
// �ڴ��ʼ��
bool flash_filesystem_init(void)
{
    if (filesystem_success_init == true)
    {
        return true;
    }

    bool result = false;

    // �洢������nexus����Ϣ����
    nexus_filesystem.adde_handle = FLASH_ADDR_NEXUS_NV_START;
    nexus_filesystem.sector_size = 512;
    nexus_filesystem.list_sector_count = 2;
    nexus_filesystem.data_sector_count = 4;

    
    // �洢������product����Ϣ����
    product_filesystem.adde_handle = FLASH_ADDR_PRODUCT_NV_PAYG_MANAGER_CREDIT_REMAINING;
    product_filesystem.sector_size = 512;
    product_filesystem.list_sector_count = 2;
    product_filesystem.data_sector_count = 4;

    // �洢����identity����Ϣ����
    identity_filesystem.adde_handle = FLASH_ADDR_PRODUCT_NV_ID_MANAGER;
    identity_filesystem.sector_size = 512;
    identity_filesystem.data_sector_count = 1;
    identity_filesystem.data_size = 20;


    return result;
}
