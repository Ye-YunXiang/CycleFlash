Cycle Flash
==================================

It is used to circulate data into the specified area of falsh in MCU to ensure data safety and extend the life of flash area.

存入数据说明
----------------------------------

Created: 2024.7.29

> 存入目录结构：`ID(4byte) | 数据长度(2byte) | 数据地址(4byte) | CRC8(1byte)`
> 存入数据结构：`ID(4byte) | 数据长度(2byte) | 数据(最大为1页大小-7) | CRC8(1byte)`

***没有目录的情况下***

- 不会跨页存储数据，比如本页剩余空间不够大了，换下一页存储。

***有目录的情况下***

- 数据页<=2页：数据不会进行跨页存储，一页长度不够了就换一夜存储。
- 数据页>2页：页和页之间的数据会紧密存储，比如一页的长度不够了，数据存储会存储到下一页。

***循环重复存储***

- 每次存储要写入下一页内存之前，会先擦除下一页内存，在进行存储。

系统对象类型说明
--------------------------------

Created: 2024.7.29

1、**固定数据存储(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_DATA_STORAGE)**  

- 固定数据存储没有目录，存入的每个数据空间大小是固定的。

2、**循环数据存储(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_CYCLE_DATA_LENGTH)**  

- 循环存储可以有目录，也可以没有目录；
- 如果有目录遍历的时候遍历的是目录，没有目录的时候遍历的是数据块。
- ***如果有目录***，数据是紧凑存储，***必须通过目录找数据***。
- ***无目录状态***，数据是以固定间隔存储，***必须设置 cfs对象的数据块大小***。

3、**OTA缓存区(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_OTA_BUFFER)**

> 这里只是构思，先不进行这部分

- 设置好必须设置 cfs对象的存储区域范围后，第一页为校验区域的地址。
- 大纲页(动态计算存储页数),下方为大纲的存储结构：
  - `bin文件大小(4byte) | bin文件crc16-XMODEM(2byte) | version(10byte) | 大纲数据的_crc8(1byte)`
- 数据页： `bin文件数据(按页存储，从大纲页的下一页开始)`

下方是临时记录
--------------------------------

Created: 2024.7.27
Old plans, ready for iteration, keep records for now.

定长存储类型(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_LENGTH)

- 只能存入固定数据，不做循环存储数据，存储时有风险，请用来存入设备出厂数据！！！
- 写入出错会拷贝整页信息后，擦除整页重新写入，有几率在擦除页面后正好掉电丢失数据。
- 注意：一定要考虑好内存块个数，如存入的ID计算出来的位置大于分配的区域，会擦除第一页后，循环存储，会丢失重复地址的整页旧数据。
  
不定长存储类型(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_VARIABLE_LENGTH)

- 一定要有目录，否则初始化失败；
- 目录页建议至少2页，否则目录长度超出后，会擦除第一页，循环存储，会丢失重复地址的整页旧目录数据。
