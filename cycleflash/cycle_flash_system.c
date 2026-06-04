/*
 * This file is part of the cycle_flash_system Library.
 *
 * Copyright (c) 2026, YeYunXiang, <poetrycloud@foxmail.com>
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
 */
// Encoding:UTF-8

#include <assert.h>
#include <string.h>
#include "cycle_flash_system.h"
#include "cfs_port.h"

// 本库当使用内部falsh时，没有任何的全局静态变量

// ===========================================================================
// CRC / 校验
// ===========================================================================

#if CFS_CHECK == 0

static uint16_t _check_sum(const uint8_t *data, uint32_t data_length, uint8_t *out)
{
    uint16_t checksum = 0;
    for (uint32_t i = 0; i < data_length; i++) 
    { 
        if (out != NULL) 
        {
            out[i] = data[i]; 
        }
        checksum += data[i];
     }
    return ~checksum + 1;
}

#elif CFS_CHECK == 1

static uint16_t _crc16(const uint8_t *data, uint32_t data_length, uint8_t *out)
{
    uint16_t crc = 0;
    while (data_length--) 
    {
        uint8_t c = *data++;
        if (out != NULL) 
        {
            *out = c; 
            out++; 
        }
        crc ^= (uint16_t)c << 8;
        for (int i = 0; i < 8; i++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ (0x1021);
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc;
}

#elif CFS_CHECK == 2

static const uint16_t __crc16_xmodem_tab[256] = {
    0x00,0x1021,0x2042,0x3063,0x4084,0x50A5,0x60C6,0x70E7,
    0x8108,0x9129,0xA14A,0xB16B,0xC18C,0xD1AD,0xE1CE,0xF1EF,
    0x1231,0x210,0x3273,0x2252,0x52B5,0x4294,0x72F7,0x62D6,
    0x9339,0x8318,0xB37B,0xA35A,0xD3BD,0xC39C,0xF3FF,0xE3DE,
    0x2462,0x3443,0x420,0x1401,0x64E6,0x74C7,0x44A4,0x5485,
    0xA56A,0xB54B,0x8528,0x9509,0xE5EE,0xF5CF,0xC5AC,0xD58D,
    0x3653,0x2672,0x1611,0x630,0x76D7,0x66F6,0x5695,0x46B4,
    0xB75B,0xA77A,0x9719,0x8738,0xF7DF,0xE7FE,0xD79D,0xC7BC,
    0x48C4,0x58E5,0x6886,0x78A7,0x840,0x1861,0x2802,0x3823,
    0xC9CC,0xD9ED,0xE98E,0xF9AF,0x8948,0x9969,0xA90A,0xB92B,
    0x5AF5,0x4AD4,0x7AB7,0x6A96,0x1A71,0xA50,0x3A33,0x2A12,
    0xDBFD,0xCBDC,0xFBBF,0xEB9E,0x9B79,0x8B58,0xBB3B,0xAB1A,
    0x6CA6,0x7C87,0x4CE4,0x5CC5,0x2C22,0x3C03,0xC60,0x1C41,
    0xEDAE,0xFD8F,0xCDEC,0xDDCD,0xAD2A,0xBD0B,0x8D68,0x9D49,
    0x7E97,0x6EB6,0x5ED5,0x4EF4,0x3E13,0x2E32,0x1E51,0xE70,
    0xFF9F,0xEFBE,0xDFDD,0xCFFC,0xBF1B,0xAF3A,0x9F59,0x8F78,
    0x9188,0x81A9,0xB1CA,0xA1EB,0xD10C,0xC12D,0xF14E,0xE16F,
    0x1080,0xA1,0x30C2,0x20E3,0x5004,0x4025,0x7046,0x6067,
    0x83B9,0x9398,0xA3FB,0xB3DA,0xC33D,0xD31C,0xE37F,0xF35E,
    0x2B1,0x1290,0x22F3,0x32D2,0x4235,0x5214,0x6277,0x7256,
    0xB5EA,0xA5CB,0x95A8,0x8589,0xF56E,0xE54F,0xD52C,0xC50D,
    0x34E2,0x24C3,0x14A0,0x481,0x7466,0x6447,0x5424,0x4405,
    0xA7DB,0xB7FA,0x8799,0x97B8,0xE75F,0xF77E,0xC71D,0xD73C,
    0x26D3,0x36F2,0x691,0x16B0,0x6657,0x7676,0x4615,0x5634,
    0xD94C,0xC96D,0xF90E,0xE92F,0x99C8,0x89E9,0xB98A,0xA9AB,
    0x5844,0x4865,0x7806,0x6827,0x18C0,0x8E1,0x3882,0x28A3,
    0xCB7D,0xDB5C,0xEB3F,0xFB1E,0x8BF9,0x9BD8,0xABBB,0xBB9A,
    0x4A75,0x5A54,0x6A37,0x7A16,0xAF1,0x1AD0,0x2AB3,0x3A92,
    0xFD2E,0xED0F,0xDD6C,0xCD4D,0xBDAA,0xAD8B,0x9DE8,0x8DC9,
    0x7C26,0x6C07,0x5C64,0x4C45,0x3CA2,0x2C83,0x1CE0,0xCC1,
    0xEF1F,0xFF3E,0xCF5D,0xDF7C,0xAF9B,0xBFBA,0x8FD9,0x9FF8,
    0x6E17,0x7E36,0x4E55,0x5E74,0x2E93,0x3EB2,0xED1,0x1EF0,
};

static uint16_t _crc16_tab(const uint8_t *data, uint32_t data_length, uint8_t *out)
{
    uint16_t crc = 0x0000;
    for (uint32_t i = 0; i < data_length; i++) 
    {
        uint8_t c = data[i];
        
        if (out != NULL) 
        {
            out[i] = c;
        }
        crc = (crc << 8) ^ __crc16_xmodem_tab[((crc >> 8) ^ c) & 0xFF];
    }
    return crc;
}

#elif CFS_CHECK == 3   // 用户自定义，自己去实现
    // 用户自定义实现相关的校验函数
#endif


static uint16_t _calc_check(const uint8_t *data, uint32_t data_length, uint8_t *out)
{
#if CFS_CHECK == 0 // CHECK_SUM
    return _check_sum(data, data_length, out);
#elif CFS_CHECK == 1 // CRC16_XMODEM
    return _crc16(data, data_length, out);
#elif CFS_CHECK == 2 // CRC16_XMODEM 查表法(占用256Byte的RAM)
    return _crc16_tab(data, data_length, out);
#elif CFS_CHECK == 3   // 用户自定义，自己去实现
    // 用户自定义实现相关的校验函数
    (void)data; (void)n; (void)out; return 0;
#else
    #error "No checksum algorithm has been set yet, that's not acceptable!!! Dami"
    #error "啥也没定校验算法啊，不行！！！大咩"
#endif
}

// ===========================================================================
// 底层 Flash 操作（通过 port 接口）
// ===========================================================================

#if CFS_FLASH_READ_MODE == 1
static void _flash_read(uint32_t addr, uint8_t *buf, uint16_t n)
{
    cfs_port_read(addr, buf, n);
}
#endif

// 读缓存（CFS_FLASH_READ_MODE == 1 时使用）
#if CFS_FLASH_READ_MODE == 1
    #if CFS_FLASH_READ_BUFFER_SIZE  < 1
        #error "妈耶，设定为用户自定义读取模式，必须设置缓存区大小，不能为0"
    #endif // CFS_FLASH_READ_BUFFER_SIZE

    uint8_t _tmp_read_buffer[CFS_FLASH_READ_BUFFER_SIZE] = {0};
    #define _TMP_BUF_CLEAN()    // 清空缓存操作，暂时不定义清空
    #define _TMP_BUF_PTR        _tmp_read_buffer
#else
    #define _TMP_BUF_CLEAN()
    #define _TMP_BUF_PTR  NULL
#endif

static void _flash_write(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    if (len == 0) 
    {
        return;
    }
    cfs_port_lock_enable();

    while (len) {
#if CFS_WRITE_PORT_DOUBLE_WORD != 0
        if ((addr & 7) == 0 && len >= 8) 
        {
            uint16_t c = len / 8;
            cfs_port_write_double_word(addr, buf, c);
            buf += c * 8; 
            addr += c * 8; 
            len -= c * 8; 
            continue;
        }
#endif
#if CFS_WRITE_PORT_ONE_WORD != 0
        if ((addr & 3) == 0 && len >= 4) 
        {
            uint16_t c = len / 4;
            cfs_port_write_word(addr, buf, c);
            buf += c * 4;
            addr += c * 4; 
            len -= c * 4; 
            continue;
        }
#endif
#if CFS_WRITE_PORT_HALF_WORD != 0
        if ((addr & 1) == 0 && len >= 2) 
        {
            uint16_t c = len / 2;
            cfs_port_write_half_word(addr, buf, c);
            buf += c * 2; 
            addr += c * 2; 
            len -= c * 2; 
            continue;
        }
#endif

#if CFS_WRITE_PORT_ONE_BYTE != 0
        // 基本到这步，能被上面优先存储的已经被存完了，剩下的一次性存储就好
        cfs_port_write_byte(addr, buf, len);
        break;
#endif // CFS_WRITE_PORT_ONE_BYTE

    }
    cfs_port_lock_disable();
}

static void _flash_erase(uint32_t addr, uint16_t pages)
{
    cfs_port_lock_enable();
    cfs_port_erase_page(addr, pages);
    cfs_port_lock_disable();
}

// ===========================================================================
// 内存层 —— 地址计算 & 数据读写
// ===========================================================================

// 计算需要填充的字节数
static uint8_t _fill(uint16_t size)
{
    uint8_t data_fill_len = size % CFS_WRITE_MIN_PARTICLE;
    if (data_fill_len == 0)
    {
        return 0;
    }
    else
    {
        return (CFS_WRITE_MIN_PARTICLE - data_fill_len);
    }
}

// 根据 ID 计算 flash 地址
#ifdef CFS_TEST
uint32_t _data_addr(const cfs_object_t *obj, cfs_data_id_t id)
#else
static uint32_t _data_addr(const cfs_object_t *obj, cfs_data_id_t id)
#endif
{
    cfs_data_id_t FLASH_MAX_ID_COUNT = 
        (obj->sector_count * CFS_FLASH_SECTOR_SIZE) / obj->data_buffer_size;
    cfs_data_id_t data_cycle = (id + 1) / FLASH_MAX_ID_COUNT;
    cfs_data_id_t data_cycle_int = (id + 1) % FLASH_MAX_ID_COUNT;
    uint32_t off = 0;

    if (data_cycle < 1 || (data_cycle == 1 && data_cycle_int == 0))
    {
        off = id * obj->data_buffer_size;
    }
    else if (data_cycle >= 1 && data_cycle_int != 0)
    {
        off = (id - data_cycle * FLASH_MAX_ID_COUNT) * obj->data_buffer_size;
    }
    else
    {
        off = (id - (data_cycle - 1) * FLASH_MAX_ID_COUNT) * obj->data_buffer_size;
    }

    return off + obj->address;
}

/**
 * 260604
 * _valid_id  off-by-one（cycle_flash_system.c:314）：
 * 非回绕时返回 id + 1 (记录数) 应为 id (最大可回溯步数)，
 * 导致 cfs_nv_read(past=valid_id) 读取越界崩溃。
 * 已修复为 return id 和 return data_cycle_int - 1。
 */
// 计算有效记录数
#ifdef CFS_TEST
cfs_data_id_t _valid_id(const cfs_object_t *obj, cfs_data_id_t id)
#else
static cfs_data_id_t _valid_id(const cfs_object_t *obj, cfs_data_id_t id)
#endif
{
    if (id == CFS_CONFIG_NOT_LINKED_DATA_ID) 
    {
        return CFS_CONFIG_NOT_LINKED_VALID_DATA_ID;
    }
    const cfs_data_id_t FLASH_MAX_ID_COUNT = 
        (obj->sector_count * CFS_FLASH_SECTOR_SIZE) / obj->data_buffer_size;
    // 计算循环存储了几次后，剩余几个可回溯ID
    cfs_data_id_t data_cycle_int = (id + 1) % FLASH_MAX_ID_COUNT;

    // XXX: 这里需要注意，只要正好存满或者还未开始循环，都直接进入返回
    if (((id + 1) / FLASH_MAX_ID_COUNT) < 1 || data_cycle_int == 0) 
    {
        return id;
    }

    // 判断页数多于一页
    if (obj->sector_count > 1) 
    {
        uint32_t end = _data_addr(obj, id) + obj->data_buffer_size;

        cfs_data_id_t tail = FLASH_MAX_ID_COUNT - ((((end / CFS_FLASH_SECTOR_SIZE) + 1)
            * CFS_FLASH_SECTOR_SIZE - obj->address) / obj->data_buffer_size) - 1;

        return (tail + data_cycle_int) % FLASH_MAX_ID_COUNT;
    }
    return data_cycle_int - 1;
}

// 写入一个完整数据块（头部 + 数据 + 填充）
static void _write_block(uint32_t addr, const cfs_data_block_t *b)
{
    uint16_t fill = _fill(b->data_len);
    _flash_write(addr, (const uint8_t *)b, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    addr += CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN;
    if (fill == 0)
    {
        _flash_write(addr, b->data_ptr, b->data_len);
    }
    else 
    {
        uint8_t pad[CFS_WRITE_MIN_PARTICLE];
        memset(pad, CFS_FLASH_ERASURE, CFS_WRITE_MIN_PARTICLE);
        uint16_t m = b->data_len - (CFS_WRITE_MIN_PARTICLE - fill);
        _flash_write(addr, b->data_ptr, m);
        memcpy(pad, b->data_ptr + m, b->data_len - m);
        _flash_write(addr + m, pad, CFS_WRITE_MIN_PARTICLE);
    }
}

// 校验写入内容
static bool _verify_write(const cfs_data_block_t *b, uint32_t addr, uint8_t *tmp)
{
#if CFS_FLASH_READ_MODE == 0
    (void)tmp;

    // 运行过程中，忽略内部flash读取错误的可能性，错了这位就不要咯
    if (0 != memcmp((void *)addr, b, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN)) 
    {
        return false;
    }
    return (0 == memcmp((void *)(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
                        b->data_ptr, 
                        b->data_len));
#else
    for (int i = 0; i < 2; i++) 
    {
        _flash_read(addr, tmp, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        if (memcmp(tmp, b, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN)) 
        {
            continue;
        }
        _flash_read(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, tmp, b->data_len);
        if (memcmp(tmp, b->data_ptr, b->data_len) == 0) 
        {
            return true;
        }
    }
    return false;
#endif
}

// 检查是否已擦除
static bool _is_erased(uint32_t addr, uint16_t len, uint8_t *tmp)
{
#if CFS_FLASH_READ_MODE == 0
    for (uint16_t j = 0; j < len; j++)
        if (((uint8_t *)addr)[j] != CFS_FLASH_ERASURE) return false;
    return true;
#else
    for (int i = 0; i < 2; i++) {
        bool ok = true;
        _flash_read(addr, tmp, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        for (uint16_t j = 0; j < CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN; j++)
            if (tmp[j] != CFS_FLASH_ERASURE) { ok = false; break; }
        if (!ok) continue;
        _flash_read(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, tmp,
                    len - CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        for (uint16_t j = 0; j < len - CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN; j++)
            if (tmp[j] != CFS_FLASH_ERASURE) { ok = false; break; }
        if (ok) return true;
    }
    return false;
#endif
}

// 读取指定地址的数据块 ID（带校验）
// 用于初始化步骤，初始化很重要，不容许出错，所以无论是内置还是外置都强行强制重试一次
static cfs_data_id_t _read_id(const cfs_object_t *obj, uint32_t addr)
{
    _TMP_BUF_CLEAN();
    
    for (int i = 0; i < 2; i++) 
    {
        cfs_data_block_t read_block;
        memset(&read_block, 0, sizeof(read_block));
#if CFS_FLASH_READ_MODE == 0
        memcpy(&read_block, (void *)addr, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
#else
        _flash_read(addr, (uint8_t *)&read_block, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
#endif
    
        if (read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID) 
        {
            continue;
        }
        if (read_block.data_len == 0 || read_block.data_len > obj->data_size) 
        {
            continue;
        }

        uint16_t ck = _calc_check(
            (uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);

#if CFS_FLASH_READ_MODE == 0
        ck += _calc_check((void *)(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN),
                    read_block.data_len, NULL);
#else
        _flash_read(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, _TMP_BUF_PTR, read_block.data_len);
        ck += _calc_check(_TMP_BUF_PTR, read_block.data_len, NULL);
#endif

        if (ck == read_block.data_check) return read_block.data_id;
    }

    return CFS_CONFIG_NOT_LINKED_DATA_ID;
}

// 读取数据到用户缓冲区
static int _read_data(
    const cfs_object_t *obj, uint32_t addr, uint16_t max_len, uint8_t *out)
{
#if CFS_FLASH_READ_MODE == 0
    cfs_data_block_t read_block;
    memset(out, 0, max_len);
    memset(&read_block, 0, sizeof(read_block));
    memcpy(&read_block, (void *)addr, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);

    if (read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
    { 
        return CFS_RETURN_ERROR;
    }
    if (read_block.data_len == 0 || read_block.data_len > max_len) 
    {
        return CFS_RETURN_ERROR;
    }

    uint16_t ck = _calc_check((uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
    ck += _calc_check(
        (void *)(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN), read_block.data_len, out);

    return (ck == read_block.data_check) ? (int)read_block.data_len : CFS_RETURN_ERROR;
#else
    for (int i = 0; i < 2; i++) 
    {
        cfs_data_block_t read_block;
        memset(out, 0, max_len);
        memset(&read_block, 0, sizeof(read_block));

        _flash_read(addr, (uint8_t *)&read_block, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        if (read_block.data_id == CFS_CONFIG_NOT_LINKED_DATA_ID) 
        {
            continue;
        }
        if (read_block.data_len == 0 || read_block.data_len > max_len) 
        {
            continue;
        }

        uint16_t ck = _calc_check((uint8_t *)&read_block, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
        _flash_read(addr + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, out, read_block.data_len);
        ck += _calc_check(out, read_block.data_len, NULL);
        if (ck == read_block.data_check) 
        { 
            return read_block.data_len;
        }
    }
    return CFS_RETURN_ERROR;
#endif
}

// 写入固定数据（含擦除判断、写入校验、错误标记）
static int _write_fixed(const cfs_object_t *obj, uint32_t addr,
                        cfs_data_id_t id, uint16_t len, const uint8_t *data)
{
    if ((addr + obj->data_buffer_size) > (obj->address + CFS_FLASH_SECTOR_SIZE * obj->sector_count))
    {
        return CFS_RETURN_ERROR;
    }

    _TMP_BUF_CLEAN();

    cfs_data_block_t wb = {0};
    wb.data_ptr  = (uint8_t *)data;
    wb.data_id   = id;
    wb.data_len  = len;
    wb.data_check  = _calc_check((uint8_t *)&wb, CFS_DATA_BLOCK_READ_USER_DATA_OFFSET_LEN, NULL);
    wb.data_check += _calc_check(data, len, NULL);

    // 判断是否擦除
    if (addr == obj->address || (addr % CFS_FLASH_SECTOR_SIZE) == 0)
    {
        // 情况一：循环的第一个数据。
        // 情况二：数据小于一页，但是正好在新的一页的第一个。
        _flash_erase(addr, (addr + obj->data_buffer_size - addr) / CFS_FLASH_SECTOR_SIZE + 1);
    }
    else if ((addr / CFS_FLASH_SECTOR_SIZE) !=
             ((addr + obj->data_buffer_size) / CFS_FLASH_SECTOR_SIZE)) 
    {
        // 情况三：存入的数据长度跨页了,这里要确定有出现跨页了才能进来
        uint32_t CLEAR_START_ADDR = ((addr / CFS_FLASH_SECTOR_SIZE) + 1) * CFS_FLASH_SECTOR_SIZE;
        _flash_erase(CLEAR_START_ADDR, ((addr + obj->data_buffer_size - CLEAR_START_ADDR) / CFS_FLASH_SECTOR_SIZE) + 1);
    }

    // 写入数据 ------
    _write_block(addr, &wb);
    int ret = CFS_RETURN_ERROR;
    if (_verify_write(&wb, addr, _TMP_BUF_PTR)) 
    {
        ret = len;
    } 
    else if (_is_erased(addr, len + CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN, _TMP_BUF_PTR)) 
    {
        // 写入后数据不对但区域为空 → 再写一次
        _write_block(addr, &wb);
        if (_verify_write(&wb, addr, _TMP_BUF_PTR)) 
        {
            ret = len;
        }
    }

    if (ret == CFS_RETURN_ERROR) 
    {
        memset(&wb, ~CFS_FLASH_ERASURE, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
        _flash_write(addr, (const uint8_t *)&wb, CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN);
    }
    return ret;
}

// ===========================================================================
// ID 扫描（初始化时恢复运行状态）
// ===========================================================================

// 往内存中遍历最大id进行初始化
static cfs_data_id_t _scan_max_id(const cfs_object_t *obj)
{
    /** 计算本存储区能存储的ID总数，这里指的是能存几个。
     * ID是从0开始的，所以得小于它。
     * */
    cfs_data_id_t FLASH_MAX_ID_COUNT = 
        ((obj->sector_count * CFS_FLASH_SECTOR_SIZE) / obj->data_buffer_size) - 1;
    // 创建数组，数组结构 { 遍历时的检索ID，遍历到的最大ID }
    cfs_data_id_t data_max_id[2] = {0, CFS_CONFIG_NOT_LINKED_DATA_ID};
    cfs_data_id_t data_traversal_id[2]  = {0, CFS_CONFIG_NOT_LINKED_DATA_ID};

    for (uint32_t p = 0; p < obj->sector_count; p++) 
    {
        cfs_data_id_t probe = (p * CFS_FLASH_SECTOR_SIZE) / obj->data_buffer_size;
        if (probe <= data_traversal_id[0] && p != 0) 
        {
            continue;
        }
        data_traversal_id[0] = probe;

        // 遍历区，如果错误，在往下遍历一位
        for (int j = 0; j < 3; j++) 
        {
            if (data_traversal_id[0] > FLASH_MAX_ID_COUNT) 
            {
                // 这里表示ID号超过了弹出结束
                break;
            }
            uint32_t a = _data_addr(obj, data_traversal_id[0]);
            data_traversal_id[1] = _read_id(obj, a);
            if (data_traversal_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID) 
            {
                break;
            }
            data_traversal_id[0]++; 
            data_traversal_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;
        }

        if ((data_max_id[1] < data_traversal_id[1] 
                || (data_max_id[1] == CFS_CONFIG_NOT_LINKED_DATA_ID
                    && data_traversal_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID))
            && data_traversal_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID
        ){
            data_max_id[0] = data_traversal_id[0]; 
            data_max_id[1] = data_traversal_id[1];
        }
        data_traversal_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;
    }

    // 如果读出来的结果是有ID的，遍历ID最大的这一页，寻找ID的最大值
    if (data_max_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID) 
    {
        const cfs_data_id_t LASR_TRAVERSE_MAX_ID = 
            (CFS_FLASH_SECTOR_SIZE / obj->data_buffer_size) + data_max_id[0] + 1;
        data_traversal_id[0] = data_max_id[0] + 1;

        while (data_traversal_id[0] < FLASH_MAX_ID_COUNT 
            && data_traversal_id[0] < LASR_TRAVERSE_MAX_ID) 
        {
            uint32_t a = _data_addr(obj, data_traversal_id[0]);
            data_traversal_id[1] = _read_id(obj, a);
            if (data_traversal_id[1] != CFS_CONFIG_NOT_LINKED_DATA_ID) 
            {
                if (data_traversal_id[1] >= data_max_id[1]) 
                {
                    data_max_id[1] = data_traversal_id[1]; 
                }
                else 
                {
                    break;
                }
            }
            data_traversal_id[0]++; 
            data_traversal_id[1] = CFS_CONFIG_NOT_LINKED_DATA_ID;
        }
    }
    return data_max_id[1];
}




// ===========================================================================
// 公共 API
// ===========================================================================

bool cfs_nv_object_init(cfs_object_t *obj,
                        uint32_t address, 
                        uint32_t sector_count,
                        uint16_t data_size)
{
    CFS_ASSERT(obj && sector_count >= 1 && data_size > 0);

    CFS_ASSERT(address % CFS_FLASH_SECTOR_SIZE == 0);   // 必须地址按页对齐，否则出错

#if CFS_FLASH_READ_MODE == 1
    // 如果自定义读取，缓存区要比读取的数大
    // 并且这个缓存至少至少也要比数据块自带的头要大
    CFS_ASSERT(data_size <= CFS_FLASH_READ_BUFFER_SIZE 
        &&  CFS_FLASH_READ_BUFFER_SIZE >= CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN
    );
#endif

    memset(obj, 0, sizeof(*obj));
    obj->address      = address;
    obj->sector_count = sector_count;
    obj->data_size    = data_size;
    obj->data_buffer_size = CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN
                            + data_size + _fill(data_size);

    CFS_ASSERT(obj->data_buffer_size < sector_count * CFS_FLASH_SECTOR_SIZE);

    // 遍历填充ID
    obj->data_id  = _scan_max_id(obj);
    obj->valid_id = _valid_id(obj, obj->data_id);
    return true;
}

int cfs_nv_write(cfs_object_t *obj, 
                const uint8_t *data,
                uint16_t len, 
                uint8_t retry)
{
    CFS_ASSERT(obj && data && len);
    if (len > obj->data_size) 
    {
        return CFS_RETURN_ERROR;
    }

    int result = CFS_RETURN_ERROR;
    do {
        if (obj->data_id < CFS_CONFIG_DATA_ID_UPPER_LIMIT)
        {
            obj->data_id++;
        }
        else if (obj->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID)
        {
            obj->data_id = 0;
        }
        else 
        {
            _flash_erase(obj->address, obj->sector_count);
            obj->data_id  = 0;
            obj->valid_id = 0;
        }

        uint32_t addr = _data_addr(obj, obj->data_id);
        result = _write_fixed(obj, addr, obj->data_id, len, data);
        if (result > 0)
        {
            obj->valid_id = _valid_id(obj, obj->data_id);
        }
    } while (result == CFS_RETURN_ERROR && retry-- > 0);

    return result;
}

int cfs_nv_read(cfs_object_t *obj, 
                uint8_t *data,
                uint16_t len, 
                cfs_data_id_t past)
{
    CFS_ASSERT(obj && data && len);
    if (obj->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID || past > obj->valid_id)
    {
        return CFS_RETURN_ERROR;
    }

    uint32_t addr = _data_addr(obj, obj->data_id - past);
    int result = _read_data(obj, addr, len, data);
    if (result == CFS_RETURN_ERROR) 
    {
        memset(data, 0, len);
    }
    return result;
}

bool cfs_nv_clear(cfs_object_t *obj)
{
    CFS_ASSERT(obj);
    if (obj->data_id == CFS_CONFIG_NOT_LINKED_DATA_ID) 
    {
        return true;
    }

    obj->data_id  = CFS_CONFIG_NOT_LINKED_DATA_ID;
    obj->valid_id = 0;
    _flash_erase(obj->address, obj->sector_count);
    return true;
}

cfs_data_id_t cfs_nv_get_current_id(const cfs_object_t *obj)
{
    CFS_ASSERT(obj);
    return obj->data_id;
}

cfs_data_id_t cfs_nv_get_current_valid_id(const cfs_object_t *obj)
{
    CFS_ASSERT(obj);
    return obj->valid_id;
}
