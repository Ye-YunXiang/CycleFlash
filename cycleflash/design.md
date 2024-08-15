# 说明

## 1、系统对象类型说明

> 所有数据都是紧凑存储，就是会跨页存储

Created: 2024.7.29

1、**固定数据存储(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE)**  

> *不能循环存入，如果存满了直接软件报错中断系统运行！*

- 存入的每个数据空间大小是固定的，如果重复存入，会直接更改原来的数据，有数据安全风险。
- 提供了保护机制，ID增长不能超过已有的空间，避免丢失原来的数据。

2、**循环数据存储(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH)**

> *建议要有2页数据页循环存储，一页数据存储会有丢失数据的风险！*
> 每次循环超出这页的部分就擦除下次存储的那一页。

- 存入的每个数据空间大小是固定的。
- 不要重复存储原来的ID，如果ID存在会读取整页修改在写入，有丢失数据的风险。
- 最好向前累加存储，保证所有扇叶擦写均衡。

3、**OTA缓存区(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_OTA_BUFFER)**

> *这里只是构思，先不进行这部分*

- 设置好必须设置 cfs对象的存储区域范围后，第一页为校验区域的地址。
- 大纲页(动态计算存储页数),下方为大纲的存储结构：
  - `bin文件大小(4byte) | bin文件crc16-XMODEM(2byte) | version(10byte) | 大纲数据的_crc16(2byte)`
- 数据页： `bin文件数据(按页存储，从大纲页的下一页开始)`

## 2、存入数据说明

Created: 2024.7.29

**存入的ID从0开始，FFFFFFFF代表内存中没有数据。**  

> 存入数据结构：`ID(4byte) | 数据长度(2byte) | 数据(最大为1页大小-7) | CRC16(2byte)`
> 存入数据紧密存储，一页长度不够会跨页存储。

### 2.1 计算可回溯的数据

> 存入的数据的ID是从 "0" 开始的，但是有效数据是要统计可用的个数，所以要 "ID + 1" 去计算个数。
> 要注意，循环存储的数据是推荐2页起存的，只要页数>=2页，会有一页且只有一页的数据为缓存，不算在有效数据。

一定要注意，结构体输入的是要存入的数据大小，但是实际存入内存中会加入包头包尾，所以实际存入长度比设定的长度大8byte。
★★★***数据块大小***： 这里的数据块大小为`cfs_system.data_size + 8`
上面计算结果用于下面的计算。

- 先计算分配的区域能存储多少数据块：`all_data = (扇叶大小 * 总共页数) / 数据块大小`。
- 接下来根据ID计算循环存储了多少次 `data_cycle = (id + 1) / all_data)`
- 计算数据的余数，看看是不是正好存满：`data_cycle_int = (id + 1) % all_data`
- 接下来分两种总情况：
  - 如果 data_cycle < 1 || (data_cycle == 1 && data_cycle_int == 0)：
    - 得出结论：`result_id = id + 1`
  - 否则：
    - 如果分配的总页数 > 1:
      - 先得出ID对应的数据结束地址：`id_end_addr = id对应的存储地址 + 数据块大小`
      - 判断是否刚好存满 `if((id+1) % all_data)`
        - 为真得出结果: `result_id = all_data;`
        - 为假结果：`result_id =  (all_data - ((((id_end_addr / 扇叶大小) + 1) * 扇叶大小 - 分配区域的起始地址) / 数据块大小) - 1) + (id + 1) % all_data`
    - 如果分配的总页数 = 1:
      - 得出结论：`result_id = data_cycle_int == 0 ? all_data : data_cycle_int`

### 2.2 计算ID位置

★★★***数据块大小***： 这里的数据块大小为`cfs_system.data_size + 8`

- 分配的内存能存多少个ID：`all_data = (扇叶大小 * 总共多少页) / 数据块大小`
- 计算数据循环次数：`data_cycle = (id + 1) / all_data`
- 计算数据的余数，看看是不是正好存满：`data_cycle_int = (id + 1) % all_data`
- 接下来分三总情况：
  - 如果 data_cycle < 1 || (data_cycle == 1 && data_cycle_int == 0)：
    - 得出结论：`result_addr = id * data_size + addr`
  - 如果 data_cycle >= 1 && data_cycle_int != 0 ：
    - 得出结论：`result_addr = (id - data_cycle * all_data) * data_size + addr`
  - 如果 data_cycle > 1 && data_cycle_int == 0 ：
    - 得出结论：`result_addr = (id - (data_cycle - 1) * all_data) * data_size + addr`
- 最后输出地址：`result_addr`

### 2.3 存储方式

> 建议自己要做好ID记录，只能前进不能后退，因为如果存储的数据覆盖在已有的ID上面，会正整页拷贝、修改、重写。
> 这里可能会导致数据丢失，或者重启后读取的数据错误的情况。

***固定存储类型***

- 都是紧密存储，会进行跨页存储，都是固定地址存储，id有固定数量，用完了空间就不能继续涨了。

***循环存储类型***

- 开放ID无限增长权限，可以循环进行存储，会擦除循环的重复扇叶，有效数据需要计算。
- 每次存储要写入下一页内存之前，会先擦除下一页内存，在进行存储。
