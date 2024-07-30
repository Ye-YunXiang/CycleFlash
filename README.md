# Cycle Flash

It is used to circulate data into the specified area of falsh in MCU to ensure data safety and extend the life of flash area.

## 1、系统对象类型说明

Created: 2024.7.29

1、**固定数据存储(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE)**  

> *不能循环存入，如果存满了直接软件报错中断系统运行！*

- 存入的每个数据空间大小是固定的，如果重复存入，会直接更改原来的数据，有数据安全风险。

2、**循环数据存储(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH)**

> *至少要有2页数据页才能循环存储，少于2页就报错，初始化系统失败！*

- 存入的每个数据空间大小是固定的，不能重复存储原来的数据，必须向前累加存储，保证所有扇叶擦写均衡。
- ***如果有目录***，数据是紧凑存储，***必须通过目录找数据***。
- 

3、**OTA缓存区(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_OTA_BUFFER)**

> *这里只是构思，先不进行这部分*

- 设置好必须设置 cfs对象的存储区域范围后，第一页为校验区域的地址。
- 大纲页(动态计算存储页数),下方为大纲的存储结构：
  - `bin文件大小(4byte) | bin文件crc16-XMODEM(2byte) | version(10byte) | 大纲数据的_crc16(2byte)`
- 数据页： `bin文件数据(按页存储，从大纲页的下一页开始)`

## 2、存入数据说明

Created: 2024.7.29

存入的ID从1开始，0代表内存中没有数据。  

> 存入数据结构：`ID(4byte) | 数据长度(2byte) | 数据(最大为1页大小-7) | CRC16(2byte)`

### 2.1有效数据计算

#### 2.1.1 固定数据存储类型

采用紧密存储数据，能存入的数据个数为：`(总页数 * 每页大小) / 数据块大小`。
存入是固定数据，只用来存入出场固定信息，就不需要回溯有效数据了，需要的时候自己检查有没有数据就行。

#### 2.1.2 循环数据存储类型（一定至少有2页的数据页才能使用）

> 分配页数<=2页的情况
这里使用的是非紧密存储，一页剩余的大小不足以存入数据块，就存在下一页；防止循环存储时擦除一个跨页的数据。
- 先计算一页能存入几个数据：`all_data = 一页大小/固定数据长度`；
- 接下来根据ID计算共有多少页`data_pages = id % all_data)`
- 接下来根据总共页数做裁切，得到结果
  - 如果 data_pages>=pages时，开始循环存储了：
    - `result_id = ((id % all_data) == 0) ? (all_data * 总共的页数) : (all_data * (总共的页数 - 1) + data_pages)`  
  - 如果 data_pages < pages时，代表还没存满过：
    - `result_id =id`
- 最后得到的有效数据为：`result_id`

2、分配页数>2页。

### 2.2 存储方式

***没有目录的情况下***

- 不会跨页存储数据，比如本页剩余空间不够大了，换下一页存储。

***有目录的情况下***

- 数据页<=2页：数据不会进行跨页存储，一页长度不够了就换一页存储。
- 数据页>2页：页和页之间的数据会紧密存储，比如一页的长度不够了，数据存储会存储到下一页。

***循环重复存储***

- 每次存储要写入下一页内存之前，会先擦除下一页内存，在进行存储。

## 3、下方是临时记录

Created: 2024.7.27
Old plans, ready for iteration, keep records for now.

定长存储类型(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_LENGTH)

- 只能存入固定数据，不做循环存储数据，存储时有风险，请用来存入设备出厂数据！！！
- 写入出错会拷贝整页信息后，擦除整页重新写入，有几率在擦除页面后正好掉电丢失数据。
- 注意：一定要考虑好内存块个数，如存入的ID计算出来的位置大于分配的区域，会擦除第一页后，循环存储，会丢失重复地址的整页旧数据。
  
不定长存储类型(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_VARIABLE_LENGTH)

- 一定要有目录，否则初始化失败；
- 目录页建议至少2页，否则目录长度超出后，会擦除第一页，循环存储，会丢失重复地址的整页旧目录数据。
