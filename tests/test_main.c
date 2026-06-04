/*
 * test_main.c  --  CycleFlash 全功能单元测试
 *
 * 使用大数组虚拟 flash，通过 mock_port.c 模拟读写擦除。
 * 直接包含 cycle_flash_system.c 以访问内部静态函数。
 *
 * 编译:
 *   gcc -std=c11 -O0 -Wno-int-to-pointer-cast \
 *       -DCFS_TEST -DCFS_DEBUG=0u -DCFS_FLASH_READ_MODE=1u \
 *       -I../cycleflash -o test_cycleflash test_main.c mock_port.c
 *
 * 运行: ./test_cycleflash
 */

/* ================================================================
 * 配置 —— 所有宏必须在包含 header 之前定义
 * ================================================================ */
#define CFS_TEST

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "../cycleflash/cycle_flash_system.h"
#include "../cycleflash/cfs_port.h"
#include "mock_port.h"

/* 直接包含库源码以访问内部静态函数 */
#include "../cycleflash/cycle_flash_system.c"

/* ================================================================
 * 测试框架
 * ================================================================ */
static int g_pass = 0;
static int g_fail = 0;
static int g_test_num = 0;

#define TEST(name)    static void test_##name(void)
#define RUN(name)     do { \
    g_test_num++; \
    printf("\n═══ 测试 #%d: %s ═══\n", g_test_num, #name); \
    test_##name(); \
} while(0)
#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; printf("    ✓ %s\n", msg); } \
    else { g_fail++; printf("    ✗ FAIL @%d: %s  (条件: %s)\n", __LINE__, msg, #cond); } \
} while(0)
#define CHECKEQ(a,b, msg) CHECK((a) == (b), msg)

/* ================================================================
 * 默认测试参数
 *   sector_size = 512 (header 默认值)
 *   data_buffer_size = 8 (header) + T_DATA_SIZE
 *   FLASH_MAX_ID_COUNT = (3 * 512) / data_buffer_size = 1536 / data_buffer_size
 *   data_size=40 → buffer=48 → max=32, 每扇区 512/48=10 块余 32
 *   第 10 块 (id=10) 跨扇区 0→1
 * ================================================================ */
#define T_SECTORS     3u
#define T_DATA_SIZE   40u
#define T_BUF_SIZE    (CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN + T_DATA_SIZE) /* 48 */

/* ================================================================
 * 辅助函数
 * ================================================================ */
static void print_config(void)
{
    printf("  配置: %u 扇区 × %u 字节 = %u 字节总容量\n",
           T_SECTORS, (unsigned)CFS_FLASH_SECTOR_SIZE,
           T_SECTORS * (unsigned)CFS_FLASH_SECTOR_SIZE);
    printf("  data_size=%u  data_buffer_size=%u  FLASH_MAX_ID_COUNT=%u\n",
           T_DATA_SIZE, T_BUF_SIZE,
           (unsigned)((T_SECTORS * CFS_FLASH_SECTOR_SIZE) / T_BUF_SIZE));
}

/* ================================================================
 * A 组 —— 初始化 & ID 扫描 (8 项)
 * ================================================================ */
TEST(init_empty)
{
    printf("  场景: 全擦除的 flash 上电初始化\n");
    vflash_reset();
    cfs_object_t obj;
    printf("  操作: cfs_nv_object_init()\n");
    bool ok = cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECK(ok, "初始化应返回 true");
    CHECKEQ(obj.data_id, CFS_CONFIG_NOT_LINKED_DATA_ID, "data_id 应为 NOT_LINKED (0xFFFFFFFF)");
    CHECKEQ(obj.valid_id, 0u, "valid_id 应为 0 (无有效记录)");
    printf("  结论: 空 flash 初始化正常\n");
}

TEST(init_single_record)
{
    printf("  场景: 写入 1 条记录后模拟掉电, 重新初始化恢复状态\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[] = "single";
    printf("  操作: 写入 6 字节数据 [%s] → 模拟掉电 → 重新 init\n", d);
    int ret = cfs_nv_write(&obj, d, 6, 2);
    CHECK(ret == 6, "写入应返回 6");

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, 0u, "掉电后恢复: data_id 应为 0");
    CHECKEQ(obj2.valid_id, 0u, "掉电后恢复: valid_id 应为 0 (1条记录, 最大回溯步数=0)");
}

TEST(init_multiple_records)
{
    printf("  场景: 写入 10 条记录后模拟掉电, 重新初始化\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    printf("  操作: 连续写入 10 条记录 (id 0~9), 每条数据首字节=id\n");
    for (int i = 0; i < 10; i++) {
        memset(d, (uint8_t)i, sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, 9u, "掉电后恢复: data_id 应为 9 (最后写入的 ID)");
    CHECKEQ(obj2.valid_id, 9u, "掉电后恢复: valid_id 应为 9 (最大回溯步数)");
    printf("  结论: 10 条记录全部可回溯 (id 0~9)\n");
}

TEST(init_across_sectors)
{
    printf("  场景: 写入跨扇区记录后掉电, 验证扫描恢复\n");
    printf("  说明: buffer=48, 扇区 0 可容纳 10 个完整块 (id 0~9); id=10 跨扇区\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    printf("  操作: 写入 12 条记录 (id 0~11), id=10 触发跨扇区擦除\n");
    for (int i = 0; i < 12; i++) {
        memset(d, (uint8_t)(i + 0xA0), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, 11u, "掉电后恢复: data_id 应为 11");
    printf("  结论: 跨扇区记录掉电恢复正常\n");
}

TEST(init_bad_block_skip)
{
    printf("  场景: flash 中存在坏块 (header 被清零), 扫描应跳过坏块\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    printf("  操作: 写入 id 0~4, 然后人为标记 id=2 为坏块 (header 写 0x00)\n");
    for (int i = 0; i < 5; i++) {
        memset(d, (uint8_t)(i + 1), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    uint32_t addr2 = _data_addr(&obj, 2);
    vflash_mark_bad_block(addr2);
    printf("  已标记 id=2 (地址 0x%08X) 为坏块\n", addr2);

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, 4u, "扫描应跳过坏块 id=2, 找到最大 id=4");
}

TEST(init_bad_block_at_probe_point)
{
    printf("  场景: 扇区探头位置的块是坏块, 扫描应向前搜索最多 3 个位置\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    printf("  操作: 写入 12 条记录, id=10 跨扇区 (是扇区 1 的探头点), 将其标记为坏块\n");
    for (int i = 0; i < 12; i++) {
        memset(d, (uint8_t)(i + 10), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    uint32_t addr10 = _data_addr(&obj, 10);
    vflash_mark_bad_block(addr10);

    /* 继续写, 覆盖探头点后面的位置 */
    printf("  继续写入 id 12~13, 确保坏块后面有有效数据\n");
    for (int i = 12; i < 14; i++) {
        memset(d, (uint8_t)(i + 10), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, 13u, "扫描应跳过坏块探头点, 找到后续最大 id=13");
}

TEST(init_all_corrupted)
{
    printf("  场景: 所有记录全部损坏, 扫描应返回 NOT_LINKED\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40] = {0};
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);

    uint32_t addr0 = _data_addr(&obj, 0);
    vflash_mark_bad_block(addr0);
    printf("  操作: 写入 1 条记录后标记为坏块 → 重新扫描\n");

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, CFS_CONFIG_NOT_LINKED_DATA_ID, "全部损坏时 data_id 应为 NOT_LINKED");
}

TEST(init_after_wrap)
{
    printf("  场景: 写入 40 条记录 (超过最大容量 32), 回绕后掉电恢复\n");
    printf("  计算: max_id=32, 写入 40 条, 回绕 1 圈 + 8 条, id=39 为最新\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 40; i++) {
        memset(d, (uint8_t)(i + 50), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  操作: 已写入 40 条, 当前 id=%u, valid_id=%u\n",
           (unsigned)obj.data_id, (unsigned)obj.valid_id);

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, 39u, "掉电后 data_id 应恢复为 39");
    /* 理论值: data_cycle_int=8, tail=21, valid_id=(21+8)%32=29 */
    CHECKEQ(obj2.valid_id, 29u, "掉电后 valid_id 应为 29 (最大回溯步数, 约 30 条有效记录)");
    printf("  结论: 回绕后有效记录: 扇区0的8条(id32~39) + 扇区1旧数据(id11~20) + 扇区2旧数据(id21~31) ≈ 30 条\n");
}

/* ================================================================
 * B 组 —— 地址计算 _data_addr (6 项)
 * ================================================================ */
TEST(addr_linear_first_cycle)
{
    printf("  场景: 第一周期内, 地址线性递增\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    uint32_t bs = obj.data_buffer_size;
    printf("  data_buffer_size = %u 字节\n", bs);

    CHECKEQ(_data_addr(&obj, 0), VFLASH_BASE,             "id=0 → 基址");
    CHECKEQ(_data_addr(&obj, 1), VFLASH_BASE + bs,        "id=1 → 基址 + 1×buffer");
    CHECKEQ(_data_addr(&obj, 3), VFLASH_BASE + 3u * bs,   "id=3 → 基址 + 3×buffer");
    CHECKEQ(_data_addr(&obj, 7), VFLASH_BASE + 7u * bs,   "id=7 → 基址 + 7×buffer");
}

TEST(addr_exactly_full_cycle)
{
    printf("  场景: 恰好写满一圈时, 最后一个 ID 的地址\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    uint32_t bs = obj.data_buffer_size;
    cfs_data_id_t max_id = (cfs_data_id_t)((T_SECTORS * CFS_FLASH_SECTOR_SIZE) / bs);
    cfs_data_id_t last_id = (cfs_data_id_t)(max_id - 1);

    printf("  max_id=%u, 最后一个 id=%u, 预期地址=基址+%u×%u\n",
           max_id, last_id, last_id, bs);
    CHECKEQ(_data_addr(&obj, last_id), VFLASH_BASE + (uint32_t)last_id * bs,
            "最后一个 ID 地址应为基址 + last_id × buffer_size");
}

TEST(addr_first_wrap)
{
    printf("  场景: 第一次回绕, id=max_id 的地址应回到基址\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    uint32_t bs = obj.data_buffer_size;
    cfs_data_id_t max_id = (cfs_data_id_t)((T_SECTORS * CFS_FLASH_SECTOR_SIZE) / bs);

    printf("  max_id=%u, 预期 id=%u 地址回到基址\n", max_id, max_id);
    CHECKEQ(_data_addr(&obj, max_id), VFLASH_BASE, "回绕后第一个 ID 应在基址");
}

TEST(addr_after_wrap)
{
    printf("  场景: 回绕后, 后续 ID 地址从基址开始递增\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    uint32_t bs = obj.data_buffer_size;
    cfs_data_id_t max_id = (cfs_data_id_t)((T_SECTORS * CFS_FLASH_SECTOR_SIZE) / bs);

    CHECKEQ(_data_addr(&obj, max_id + 1), VFLASH_BASE + bs,      "max_id+1 → 基址 + buffer");
    CHECKEQ(_data_addr(&obj, max_id + 3), VFLASH_BASE + 3u * bs, "max_id+3 → 基址 + 3×buffer");
}

TEST(addr_multi_wrap)
{
    printf("  场景: 多次回绕后地址计算\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    uint32_t bs = obj.data_buffer_size;
    cfs_data_id_t max_id = (cfs_data_id_t)((T_SECTORS * CFS_FLASH_SECTOR_SIZE) / bs);
    cfs_data_id_t id = (cfs_data_id_t)(2u * max_id + 5);

    printf("  max_id=%u, 测试 id=2×max+5=%u\n", max_id, id);
    CHECKEQ(_data_addr(&obj, id), VFLASH_BASE + 5u * bs,
            "2 圈 + 5 偏移 → 基址 + 5×buffer");
}

TEST(addr_nonzero_base)
{
    printf("  场景: flash 对象基址非零 (偏移 1024 字节)\n");
    vflash_reset();
    uint32_t base = VFLASH_BASE + 1024u;
    cfs_object_t obj;
    cfs_nv_object_init(&obj, base, 2, T_DATA_SIZE);
    uint32_t bs = obj.data_buffer_size;

    CHECKEQ(_data_addr(&obj, 0), base,             "id=0 → 偏移基址");
    CHECKEQ(_data_addr(&obj, 3), base + 3u * bs,   "id=3 → 偏移基址 + 3×buffer");
}

/* ================================================================
 * C 组 —— 数据存储模式 (13 项, 含 2 项新增)
 * ================================================================ */
TEST(storage_basic_write_read)
{
    printf("  场景: 基本写入-读取往返\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t wdata[] = "Hello CycleFlash!";
    printf("  写入: [%s] (%u 字节)\n", wdata, (unsigned)strlen((char*)wdata));
    int ret = cfs_nv_write(&obj, wdata, (uint16_t)strlen((char*)wdata), 2);
    CHECK(ret > 0, "写入应返回正数 (实际写入字节数)");

    uint8_t rdata[64] = {0};
    int rret = cfs_nv_read(&obj, rdata, (uint16_t)ret, 0);
    printf("  读回 (past=0): ret=%d\n", rret);
    CHECKEQ(rret, ret, "读取字节数应与写入一致");
    CHECK(memcmp(rdata, wdata, (size_t)ret) == 0, "读回数据应与写入完全一致");
}

TEST(storage_multi_write_read)
{
    printf("  场景: 连续写入 5 条, 逐条回溯验证\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 5; i++) {
        memset(d, (uint8_t)(i + 1), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  已写入 5 条 (id 0~4), data_id=%u\n", (unsigned)obj.data_id);

    for (int p = 0; p < 5; p++) {
        uint8_t buf[40]; memset(buf, 0, sizeof(buf));
        int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, (cfs_data_id_t)p);
        char msg[64];
        snprintf(msg, sizeof(msg), "past=%d 应读到 id=%u 数据 (首字节=%u)", p, 4-p, 5-p);
        CHECKEQ(ret, (int)T_DATA_SIZE, msg);
        CHECKEQ(buf[0], (uint8_t)(5 - p), msg);
    }
}

TEST(storage_unaligned_cross_sector)
{
    printf("  场景: 非对齐跨扇区写入 (buffer=48, 第 10 块起跨扇区 0→1)\n");
    printf("  id=10 起始地址 480, 结束于 528, 跨越扇区边界 512\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i <= 11; i++) {
        memset(d, (uint8_t)(i + 100), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  已写入 12 条 (id 0~11), 其中 id=10 触发跨扇区擦除扇区 1\n");

    /* 回溯到跨扇区之前的数据, 验证扇区 0 数据未被破坏 */
    uint8_t buf[40];
    int rret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 2);
    CHECKEQ(rret, (int)T_DATA_SIZE, "past=2 (读 id=9) 应成功");
    CHECKEQ(buf[0], 109, "id=9 数据首字节应为 109 (=9+100)");
    printf("  结论: 跨扇区写入未破坏相邻扇区旧数据\n");
}

TEST(storage_aligned_end_at_boundary)
{
    printf("  场景: 对齐配置 (data_size=56→buffer=64), 块 id=7 恰好结束于扇区边界 512\n");
    printf("  每扇区恰好 8 个块, id=7 的 end=512, 触发保守擦除扇区 1\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, 3, 56);

    uint8_t d[56];
    for (int i = 0; i <= 9; i++) {
        memset(d, (uint8_t)(i + 200), sizeof(d));
        cfs_nv_write(&obj, d, 56, 2);
    }
    printf("  已写入 10 条 (id 0~9), id=7 结束于扇区边界\n");

    uint8_t buf[56];
    int ret = cfs_nv_read(&obj, buf, 56, 0);
    CHECKEQ(ret, 56, "读最新记录 (id=9) 应返回 56 字节");
    CHECKEQ(buf[0], 209, "id=9 数据首字节应为 209");
}

TEST(storage_fill_completely)
{
    printf("  场景: 恰好写满整个 flash 区域 (max_id 条记录)\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint32_t bs = obj.data_buffer_size;
    cfs_data_id_t max_id = (cfs_data_id_t)((T_SECTORS * CFS_FLASH_SECTOR_SIZE) / bs);
    printf("  max_id=%u, 将写入 %u 条记录填满 flash\n", max_id, max_id);

    uint8_t d[40];
    for (cfs_data_id_t i = 0; i < max_id; i++) {
        memset(d, (uint8_t)(i & 0xFF), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    printf("  写入完成, data_id=%u, valid_id=%u\n",
           (unsigned)obj.data_id, (unsigned)obj.valid_id);
    /* 满圈时 valid_id 应恰好等于 max_id - 1 (最后一条 id) */
    CHECKEQ(obj.valid_id, max_id - 1, "满圈时 valid_id 应 = max_id - 1 (最大回溯步数)");
}

TEST(storage_wrap_and_overwrite)
{
    printf("  场景: 写满后继续写入 (回绕覆盖旧数据)\n");
    printf("  写入 40 条 (>max=32), 最老的 id=0~7 应被覆盖而不可读\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 40; i++) {
        memset(d, (uint8_t)(i + 30), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    CHECK(obj.valid_id < 32u, "回绕后 valid_id 应小于 32 (旧数据被覆盖)");
    printf("  valid_id=%u (最大回溯步数)\n", (unsigned)obj.valid_id);

    /* 验证最新数据可读 */
    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, (int)T_DATA_SIZE, "past=0 读最新 (id=39) 应成功");
    CHECKEQ(buf[0], 69, "id=39 首字节应为 69 (=39+30)");

    /* —— 新增: 验证被覆盖的旧数据不可读 —— */
    cfs_data_id_t vid = cfs_nv_get_current_valid_id(&obj);
    printf("  验证: past=%u+1 应返回错误 (超出有效范围)\n", vid);
    uint8_t buf2[40];
    int ret2 = cfs_nv_read(&obj, buf2, T_DATA_SIZE, vid + 1u);
    CHECKEQ(ret2, CFS_RETURN_ERROR, "past > valid_id 应返回 CFS_RETURN_ERROR (旧数据已覆盖)");
}

TEST(storage_multi_cycle_3)
{
    printf("  场景: 写入 100 条记录 (~3 圈), 验证系统稳定\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 100; i++) {
        memset(d, (uint8_t)(i & 0xFF), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    CHECK(obj.valid_id > 0u, "3 圈后 valid_id 应 > 0 (仍有有效记录)");
    CHECKEQ(obj.data_id, 99u, "3 圈后 data_id 应为 99");
    printf("  valid_id=%u, 系统运行正常\n", (unsigned)obj.valid_id);
}

TEST(storage_large_block_one_sector)
{
    printf("  场景: 大数据块 (data_size=504→buffer=512), 一个块恰好占满一扇区\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, 2, 504);

    uint8_t d[504];
    memset(d, 0xAB, sizeof(d));
    printf("  写入 504 字节 (0xAB)...\n");
    int ret = cfs_nv_write(&obj, d, 504, 2);
    CHECKEQ(ret, 504, "写入 504 字节应成功");

    uint8_t buf[504];
    memset(buf, 0, sizeof(buf));
    ret = cfs_nv_read(&obj, buf, 504, 0);
    CHECKEQ(ret, 504, "读取应返回 504 字节");
    CHECKEQ(buf[0], 0xAB, "首字节应为 0xAB");
    CHECKEQ(buf[503], 0xAB, "末字节应为 0xAB");
}

TEST(storage_large_block_cross_two_pages)
{
    printf("  场景: 数据块跨越两个扇区 (data_size=300→buffer=308)\n");
    printf("  第一条从扇区 0 起始, 第二条从扇区 0 尾部跨入扇区 1\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, 2, 300);

    /* 第一条写 */
    uint8_t d[300];
    memset(d, 0x11, sizeof(d));
    printf("  写入第一条 300 字节 (0x11)...\n");
    int ret = cfs_nv_write(&obj, d, 300, 2);
    CHECKEQ(ret, 300, "第一条写入应成功");

    /* 第二条写 —— 跨扇区 */
    uint8_t d2[300];
    for (int i = 0; i < 300; i++) d2[i] = (uint8_t)(i & 0xFF);
    printf("  写入第二条 300 字节 (跨扇区, 起始地址=%u)...\n",
           (unsigned)_data_addr(&obj, 1));
    ret = cfs_nv_write(&obj, d2, 300, 2);
    CHECKEQ(ret, 300, "跨扇区写入应成功");

    /* 读回跨扇区块 */
    uint8_t buf[300];
    ret = cfs_nv_read(&obj, buf, 300, 0);
    CHECKEQ(ret, 300, "读取跨扇区块应成功");
    CHECK(memcmp(buf, d2, 300) == 0, "跨扇区块数据应与写入一致");
}

TEST(storage_no_erroneous_erase)
{
    printf("  场景: 在扇区 1 写入时, 验证扇区 0 的旧数据未被误擦除\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    printf("  先在扇区 0 写入 id 0~9 (扇区 0 全满)...\n");
    for (int i = 0; i < 10; i++) {
        memset(d, (uint8_t)(i + 70), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    printf("  再写入 id 10~13 (跨入扇区 1)...\n");
    for (int i = 10; i <= 13; i++) {
        memset(d, (uint8_t)(i + 70), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }

    /* 回读扇区 0 最老的数据 id=0 (past=13) */
    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 13);
    CHECKEQ(ret, (int)T_DATA_SIZE, "past=13 应能读到扇区 0 最老数据 (id=0)");
    CHECKEQ(buf[0], 70, "id=0 数据首字节应为 70 (未被误擦除)");
    printf("  结论: 扇区 0 数据完好, 无误擦除\n");
}

TEST(storage_middle_of_flash)
{
    printf("  场景: flash 对象基址不在 VFLASH_BASE (偏移 512 字节)\n");
    vflash_reset();
    uint32_t base = VFLASH_BASE + 512u;
    cfs_object_t obj;
    cfs_nv_object_init(&obj, base, 2, T_DATA_SIZE);

    uint8_t d[40];
    memset(d, 0xCC, sizeof(d));
    int ret = cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    CHECKEQ(ret, (int)T_DATA_SIZE, "非零基址写入应成功");

    CHECKEQ(_data_addr(&obj, 0), base, "id=0 地址应为非零基址");
    CHECKEQ(_data_addr(&obj, 1), base + obj.data_buffer_size, "id=1 地址应为基址 + buffer");
}

/* —— 新增 C11: 回绕后核实被覆盖的旧数据不可读 (强化版) —— */
TEST(storage_overwritten_unreadable)
{
    printf("  场景: 多条回绕后, 逐一验证最早被覆盖的区域不可读\n");
    printf("  max_id=32, 写 50 条 (回绕约 1.5 圈), 最老的 18 条应被覆盖\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 50; i++) {
        memset(d, (uint8_t)(i & 0xFF), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  写入 50 条完成, data_id=%u, valid_id=%u\n",
           (unsigned)obj.data_id, (unsigned)obj.valid_id);

    cfs_data_id_t vid = cfs_nv_get_current_valid_id(&obj);
    /* 尝试读超出 valid_id 的记录, 应返回错误 (已被覆盖) */
    for (cfs_data_id_t p = vid + 1; p < vid + 5; p++) {
        uint8_t buf[40];
        int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, p);
        char msg[64];
        snprintf(msg, sizeof(msg), "past=%u 应返回错误 (该记录已被覆盖)", p);
        CHECKEQ(ret, CFS_RETURN_ERROR, msg);
    }
    printf("  结论: 被覆盖区域的连续 4 条均不可读, 覆盖逻辑正确\n");
}

/* —— 新增 C12: particle=1 时 padding 始终为 0 (已验证) + 注释 —— */
TEST(storage_padding_note)
{
    printf("  场景: CFS_WRITE_MIN_PARTICLE 的 padding 逻辑验证\n");
    printf("  当前配置: CFS_WRITE_MIN_PARTICLE=1 (字节写入), padding 始终为 0\n");
    printf("  data_buffer_size = 8 (header) + data_size + _fill(data_size)\n");
    printf("  _fill(%u) = %u (应为 0, 因为 %% 1 == 0)\n",
           T_DATA_SIZE, (unsigned)(T_DATA_SIZE % 1u));
    printf("  注意: 若使用 half-word(2) 或 word(4) 写入, ");
    printf("需单独编译测试 padding 场景\n");
    printf("  (如 data_size=5, particle=4, fill=3, 数据块尾会补 3 字节 0xFF)\n");

    /* 此处只做基本验证: particle=1 时 buffer_size 不含 padding */
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    /* 预期 buffer_size = 8 + 40 + 0 = 48 */
    CHECKEQ(obj.data_buffer_size,
            CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN + T_DATA_SIZE,
            "particle=1 时 buffer_size 应 = header(8) + data_size (无 padding)");
    printf("  实际 data_buffer_size = %u\n", obj.data_buffer_size);
}

/* ================================================================
 * D 组 —— 回溯功能 (6 项)
 * ================================================================ */
TEST(past_latest)
{
    printf("  场景: past=0 读取最新记录\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40]; memset(d, 0x11, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入 id=0, 首字节=0x11\n");

    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, (int)T_DATA_SIZE, "past=0 读取应成功");
    CHECKEQ(buf[0], 0x11, "数据首字节应为 0x11");
}

TEST(past_middle)
{
    printf("  场景: past 中间值回溯\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 5; i++) {
        memset(d, (uint8_t)i, sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  写入 5 条 (id 0~4), 当前 data_id=4\n");
    printf("  past=3 → 读 id=4-3=1, 首字节应为 1\n");

    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 3);
    CHECKEQ(ret, (int)T_DATA_SIZE, "past=3 读取应成功");
    CHECKEQ(buf[0], 1, "id=1 数据首字节应为 1");
}

TEST(past_oldest_boundary)
{
    printf("  场景: past=valid_id 读取最老的有效记录\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 10; i++) {
        memset(d, (uint8_t)i, sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  写入 10 条 (id 0~9), valid_id=%u\n",
           (unsigned)cfs_nv_get_current_valid_id(&obj));

    cfs_data_id_t vid = cfs_nv_get_current_valid_id(&obj);
    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, vid);
    CHECKEQ(ret, (int)T_DATA_SIZE, "past=valid_id (最老边界) 应读取成功");
    printf("  读到 id=%u (最老记录), 首字节=%u\n",
           9u - vid, buf[0]);
}

TEST(past_beyond_valid)
{
    printf("  场景: past > valid_id (超出有效范围) 应返回错误\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40]; memset(d, 0x55, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);

    cfs_data_id_t vid = cfs_nv_get_current_valid_id(&obj);
    printf("  写入 1 条, valid_id=%u, 尝试 past=%u\n", vid, vid + 1);

    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, vid + 1u);
    CHECKEQ(ret, CFS_RETURN_ERROR, "past > valid_id 应返回 CFS_RETURN_ERROR (-1)");
}

TEST(past_across_wrap)
{
    printf("  场景: 回绕后跨周期回溯 (越过回绕点读上一周期的数据)\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 40; i++) {
        memset(d, (uint8_t)(i ^ 0x80), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  写入 40 条 (>max=32, 已回绕), data_id=39\n");

    cfs_data_id_t vid = cfs_nv_get_current_valid_id(&obj);
    CHECK(vid > 0u, "回绕后 valid_id 应 > 0");
    printf("  valid_id=%u, 尝试读 past=%u (回绕前数据)\n", vid, vid > 0 ? vid - 1 : 0);

    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, vid > 0 ? vid - 1u : 0u);
    CHECKEQ(ret, (int)T_DATA_SIZE, "跨回绕回溯应成功");
    printf("  跨回绕回溯成功, 读取到回绕前记录\n");
}

TEST(valid_id_changes)
{
    printf("  场景: 逐次写入, 验证 valid_id 递增\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];

    memset(d, 0x11, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入第 1 条: valid_id=%u\n", cfs_nv_get_current_valid_id(&obj));
    CHECKEQ(cfs_nv_get_current_valid_id(&obj), 0u, "第 1 条后 valid_id 应为 0");

    memset(d, 0x22, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入第 2 条: valid_id=%u\n", cfs_nv_get_current_valid_id(&obj));
    CHECKEQ(cfs_nv_get_current_valid_id(&obj), 1u, "第 2 条后 valid_id 应为 1");

    memset(d, 0x33, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入第 3 条: valid_id=%u\n", cfs_nv_get_current_valid_id(&obj));
    CHECKEQ(cfs_nv_get_current_valid_id(&obj), 2u, "第 3 条后 valid_id 应为 2");

    printf("  结论: valid_id 随写入线性递增\n");
}

/* ================================================================
 * E 组 —— 错误与异常处理 (5 项)
 * ================================================================ */
TEST(error_len_exceeds_data_size)
{
    printf("  场景: 写入长度超过配置的 data_size 应返回错误\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t big[100]; memset(big, 0x00, sizeof(big));
    printf("  data_size=%u, 尝试写入 100 字节\n", T_DATA_SIZE);
    int ret = cfs_nv_write(&obj, big, 100, 2);
    CHECKEQ(ret, CFS_RETURN_ERROR, "len > data_size 应返回 -1");
}

TEST(error_corrupted_checksum_read_fails)
{
    printf("  场景: 人为损坏数据字节后, 读取应校验失败\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    memset(d, 0xFF, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入 40 字节全 0xFF, 然后翻转首个数据字节 bit0 (0xFF→0xFE)\n");

    uint32_t addr = _data_addr(&obj, 0);
    vflash_corrupt_byte(addr);

    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, CFS_RETURN_ERROR, "校验和损坏后读取应返回 -1");
}

TEST(error_read_empty_flash)
{
    printf("  场景: 空 flash 上读取应返回错误\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, CFS_RETURN_ERROR, "空 flash 读取应返回 -1");
}

TEST(error_bad_block_marking)
{
    printf("  场景: 坏块标记后, 重新扫描应跳过该块\n");
    printf("  说明: 真实 flash 写失败会标记坏块 (header 写 ~CFS_FLASH_ERASURE=0x00)\n");
    printf("  此处模拟: 写入正常记录 → 人为清零 header → 重新扫描应跳过\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40]; memset(d, 0x55, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入 id=0, 然后清零其 header 8 字节 (模拟坏块标记)\n");

    uint32_t addr0 = _data_addr(&obj, 0);
    for (int i = 0; i < 8; i++)
        *(vflash_ptr(addr0) + i) = 0x00u;

    cfs_object_t obj2;
    cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
    CHECKEQ(obj2.data_id, CFS_CONFIG_NOT_LINKED_DATA_ID, "扫描应跳过坏块, data_id 应为 NOT_LINKED");
    printf("  结论: 坏块标记后扫描正确跳过 (注: 未走真实写失败→标记路径)\n");
}

TEST(error_clear_and_rewrite)
{
    printf("  场景: 清除全部数据后重新写入\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40]; memset(d, 0xAB, sizeof(d));
    cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    printf("  写入 id=0 (0xAB), 然后调用 cfs_nv_clear()\n");

    bool ok = cfs_nv_clear(&obj);
    CHECK(ok, "清除操作应返回 true");
    CHECKEQ(obj.data_id, CFS_CONFIG_NOT_LINKED_DATA_ID, "清除后 data_id 应为 NOT_LINKED");
    CHECKEQ(obj.valid_id, 0u, "清除后 valid_id 应为 0");

    memset(d, 0xCD, sizeof(d));
    int ret = cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    CHECKEQ(ret, (int)T_DATA_SIZE, "清除后重新写入应成功");
    CHECKEQ(obj.data_id, 0u, "重新写入后 data_id 应从 0 开始");

    /* 验证新数据正确 */
    uint8_t buf[40];
    ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, (int)T_DATA_SIZE, "清除后写入的数据应可读");
    CHECKEQ(buf[0], 0xCD, "数据首字节应为 0xCD (非旧数据 0xAB)");
    printf("  结论: 清除-重写循环正常\n");
}

/* ================================================================
 * F 组 —— 边界条件 (4 项)
 * ================================================================ */
TEST(edge_single_sector)
{
    printf("  场景: 单扇区配置 (sector_count=1)\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, 1, T_DATA_SIZE);
    printf("  单扇区 512 字节, buffer=48, 可存 10 个块\n");

    uint8_t d[40];
    for (int i = 0; i < 5; i++) {
        memset(d, (uint8_t)i, sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  写入 5 条, 逐条回溯...\n");

    for (int p = 0; p < 5; p++) {
        uint8_t buf[40];
        int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, (cfs_data_id_t)p);
        char msg[48];
        snprintf(msg, sizeof(msg), "单扇区 past=%d 应成功", p);
        CHECKEQ(ret, (int)T_DATA_SIZE, msg);
        CHECKEQ(buf[0], (uint8_t)(4 - p), msg);
    }
}

TEST(edge_min_data_1byte)
{
    printf("  场景: 最小数据长度 1 字节\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, 16);

    uint8_t d = 0x5A;
    printf("  写入 1 字节: 0x%02X\n", d);
    int ret = cfs_nv_write(&obj, &d, 1, 2);
    CHECKEQ(ret, 1, "1 字节写入应成功");

    uint8_t buf[16];
    ret = cfs_nv_read(&obj, buf, 1, 0);
    CHECKEQ(ret, 1, "1 字节读取应成功");
    CHECKEQ(buf[0], 0x5A, "读出应为 0x5A");
}

TEST(edge_exact_data_size)
{
    printf("  场景: 写入恰好 data_size 长度的数据 (无截断)\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40]; memset(d, 0xEE, sizeof(d));
    printf("  写入 %u 字节 (恰好等于 data_size)\n", T_DATA_SIZE);
    int ret = cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    CHECKEQ(ret, (int)T_DATA_SIZE, "精确 data_size 写入应成功");

    uint8_t buf[40];
    ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, (int)T_DATA_SIZE, "读取应返回完整数据");
    CHECK(memcmp(buf, d, T_DATA_SIZE) == 0, "数据应与写入完全一致");
}

TEST(edge_sector_count_2)
{
    printf("  场景: 2 扇区配置 (最小多扇区)\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, 2, T_DATA_SIZE);
    printf("  2 扇区共 1024 字节, 可存 21 个块 (1024/48=21)\n");

    uint8_t d[40];
    for (int i = 0; i < 10; i++) {
        memset(d, (uint8_t)(i + 60), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
    }
    printf("  写入 10 条成功\n");

    CHECK(obj.valid_id > 0u, "valid_id 应 > 0");
    uint8_t buf[40];
    int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, 0);
    CHECKEQ(ret, (int)T_DATA_SIZE, "2 扇区最新记录应可读");
}

/* ================================================================
 * G 组 —— 压力与集成测试 (3 项)
 * ================================================================ */
TEST(stress_many_writes)
{
    printf("  场景: 连续写入 200 条 (>6 圈), 验证系统稳定性\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 200; i++) {
        memset(d, (uint8_t)(i & 0xFF), sizeof(d));
        int ret = cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
        if (ret != (int)T_DATA_SIZE) {
            char msg[64];
            snprintf(msg, sizeof(msg), "第 %d 次写入失败", i);
            CHECK(false, msg);
            break;
        }
    }

    CHECK(obj.valid_id > 0u, "200 次写入后 valid_id 应 > 0");
    CHECKEQ(obj.data_id, 199u, "200 次写入后 data_id 应为 199");
    printf("  200 次写入全部成功, 系统稳定\n");
}

TEST(stress_write_read_interleaved)
{
    printf("  场景: 写-读交替, 每次写入后立即随机回溯\n");
    vflash_reset();
    cfs_object_t obj;
    cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

    uint8_t d[40];
    for (int i = 0; i < 30; i++) {
        memset(d, (uint8_t)(i & 0xFF), sizeof(d));
        cfs_nv_write(&obj, d, T_DATA_SIZE, 2);

        cfs_data_id_t vid = cfs_nv_get_current_valid_id(&obj);
        if (vid > 0) {
            uint8_t buf[40];
            int ret = cfs_nv_read(&obj, buf, T_DATA_SIZE, vid / 2u);
            char msg[64];
            snprintf(msg, sizeof(msg), "第 %d 次写后读 past=%u", i, vid/2u);
            CHECKEQ(ret, (int)T_DATA_SIZE, msg);
        }
    }
    printf("  30 次写-读交替完成, 无异常\n");
}

TEST(stress_power_loss_simulation)
{
    printf("  场景: 模拟多次掉电-上电循环 (写 3 条→掉电→重初始化, 重复 5 次)\n");
    vflash_reset();

    for (int cycle = 0; cycle < 5; cycle++) {
        cfs_object_t obj;
        cfs_nv_object_init(&obj, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);

        printf("  第 %d 轮: 初始化时 data_id=%u, 写入 3 条...\n",
               cycle + 1, (unsigned)obj.data_id);

        uint8_t d[40];
        for (int i = 0; i < 3; i++) {
            memset(d, (uint8_t)(cycle * 3 + i), sizeof(d));
            cfs_nv_write(&obj, d, T_DATA_SIZE, 2);
        }

        /* 模拟掉电后重新上电 */
        cfs_object_t obj2;
        cfs_nv_object_init(&obj2, VFLASH_BASE, T_SECTORS, T_DATA_SIZE);
        char msg[64];
        snprintf(msg, sizeof(msg), "第 %d 轮掉电后 data_id 应恢复", cycle + 1);
        CHECK(obj2.data_id != CFS_CONFIG_NOT_LINKED_DATA_ID, msg);
        CHECK(obj2.valid_id > 0u, "掉电后 valid_id 应 > 0");

        uint8_t buf[40];
        int ret = cfs_nv_read(&obj2, buf, T_DATA_SIZE, 0);
        CHECKEQ(ret, (int)T_DATA_SIZE, "掉电后应能读到最新数据");
    }
    printf("  5 轮掉电-上电循环全部正常\n");
}

/* ================================================================
 * Main
 * ================================================================ */
int main(void)
{
    setbuf(stdout, NULL);
    printf("╔══════════════════════════════════════════╗\n");
    printf("║     CycleFlash  全功能单元测试           ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    print_config();
    printf("\n");

    /* ====== A 组: 初始化 & ID 扫描 ====== */
    printf("┌──────────────────────────────────────────┐\n");
    printf("│  A 组 —— 初始化 & ID 扫描 (8 项)         │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(init_empty);
    RUN(init_single_record);
    RUN(init_multiple_records);
    RUN(init_across_sectors);
    RUN(init_bad_block_skip);
    RUN(init_bad_block_at_probe_point);
    RUN(init_all_corrupted);
    RUN(init_after_wrap);

    /* ====== B 组: 地址计算 ====== */
    printf("\n┌──────────────────────────────────────────┐\n");
    printf("│  B 组 —— 地址计算 _data_addr (6 项)       │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(addr_linear_first_cycle);
    RUN(addr_exactly_full_cycle);
    RUN(addr_first_wrap);
    RUN(addr_after_wrap);
    RUN(addr_multi_wrap);
    RUN(addr_nonzero_base);

    /* ====== C 组: 存储模式 ====== */
    printf("\n┌──────────────────────────────────────────┐\n");
    printf("│  C 组 —— 数据存储模式 (13 项)             │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(storage_basic_write_read);
    RUN(storage_multi_write_read);
    RUN(storage_unaligned_cross_sector);
    RUN(storage_aligned_end_at_boundary);
    RUN(storage_fill_completely);
    RUN(storage_wrap_and_overwrite);
    RUN(storage_multi_cycle_3);
    RUN(storage_large_block_one_sector);
    RUN(storage_large_block_cross_two_pages);
    RUN(storage_no_erroneous_erase);
    RUN(storage_middle_of_flash);
    RUN(storage_overwritten_unreadable);
    RUN(storage_padding_note);

    /* ====== D 组: 回溯 ====== */
    printf("\n┌──────────────────────────────────────────┐\n");
    printf("│  D 组 —— 回溯功能 (6 项)                  │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(past_latest);
    RUN(past_middle);
    RUN(past_oldest_boundary);
    RUN(past_beyond_valid);
    RUN(past_across_wrap);
    RUN(valid_id_changes);

    /* ====== E 组: 错误处理 ====== */
    printf("\n┌──────────────────────────────────────────┐\n");
    printf("│  E 组 —— 错误 & 异常处理 (5 项)           │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(error_len_exceeds_data_size);
    RUN(error_corrupted_checksum_read_fails);
    RUN(error_read_empty_flash);
    RUN(error_bad_block_marking);
    RUN(error_clear_and_rewrite);

    /* ====== F 组: 边界条件 ====== */
    printf("\n┌──────────────────────────────────────────┐\n");
    printf("│  F 组 —— 边界条件 (4 项)                  │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(edge_single_sector);
    RUN(edge_min_data_1byte);
    RUN(edge_exact_data_size);
    RUN(edge_sector_count_2);

    /* ====== G 组: 压力测试 ====== */
    printf("\n┌──────────────────────────────────────────┐\n");
    printf("│  G 组 —— 压力 & 集成测试 (3 项)           │\n");
    printf("└──────────────────────────────────────────┘\n");
    RUN(stress_many_writes);
    RUN(stress_write_read_interleaved);
    RUN(stress_power_loss_simulation);

    /* ====== 总结 ====== */
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║           测 试 结 果                    ║\n");
    printf("╠══════════════════════════════════════════╣\n");
    printf("║  通过: %-4d    失败: %-4d               ║\n", g_pass, g_fail);
    if (g_fail > 0)
        printf("║         ⚠ 存 在 失 败! ⚠               ║\n");
    else
        printf("║         ✓ 全 部 通 过 ✓                 ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    printf("\n按回车键退出...");
    getchar();

    return g_fail > 0 ? 1 : 0;
}
