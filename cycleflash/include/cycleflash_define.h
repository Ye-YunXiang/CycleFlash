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
// Encoding:GB2312

#ifndef __CYCLEFLASH_DEFINE_H__
#define __CYCLEFLASH_DEFINE_H__

#include <stdint.h>

#define CFS_LIST_FUNCTION_DISABLE   0
#define CFS_FIXED_LENGTH_FUNCTION_DISABLE   0

enum cycle_object_type
{
    // �洢�����ݲ���������ҪĿ¼
    CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_VARIABLE_LENGTH = 1,
    // �洢�����ݶ���������ҪĿ¼
    CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_LENGTH = 2,
};

/*ϵͳ�Ĵ洢���󣬲����������¼ÿ���洢�����������*/
typedef struct 
{
    uint32_t adde_handle;           // �ļ�ϵͳ��flash�еľ��
    uint16_t sector_size;           // ������С
    uint16_t data_size;             // ���ݴ�С, ֻ�ж�����Ч
    uint8_t list_sector_count;      // Ŀ¼ҳ��,��������2ҳ
    uint8_t data_sector_count;      // ������������������3ҳ
    enum cycle_object_type struct_type;            // �ṹ������
}cfs_system;

/*�洢���� - �������� - ����ID �������ֵ��*/
typedef struct 
{
    void *object_handle;                                    // �洢����
    uint32_t data_id;                                       // ���ݿ�ID
    struct cfs_fixed_length_system *next;                   // ������һ���ڵ�
}cfs_object_linked_list;


/*ϵͳ�����ڴ�����ݿ�*/
// �������ݣ�����-crcУ����
typedef struct 
{
    void *data_pointer;             // �������ݵ�ָ��
    uint8_t data_crc8;              // ���ݵ�crc8У����
    uint16_t data_len;              // ���ݳ���
}cfs_data_block;

/*ϵͳ�����ڴ��Ŀ¼��*/
// �������ݣ� ����ID-���ݵ�ַ-���ݳ���-crcУ����
typedef struct 
{
    uint32_t data_id;               // ���������ID
    uint32_t data_addr;             // ��Ӧ�����ݿ��ַ
    uint16_t data_len;              // ���ݿ鳤��
    uint8_t  list_crc8;             // ��������ݿ������id����ַ�����ȣ���CRC8У����
}cfs_list_block;


#endif /* __CYCLEFLASH_DEFINE_H__ */
