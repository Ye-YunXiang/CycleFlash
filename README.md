# cycle_flash_system
It is used to circulate data into the specified area of falsh in MCU to ensure data safety and extend the life of flash area.

- 定长存储类型(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_FIXED_LENGTH)
    - 只能存入固定数据，不做循环存储数据，存储时有风险，请用来存入设备出厂数据！！！
    - 写入出错会拷贝整页信息后，擦除整页重新写入，有几率在擦除页面后正好掉电丢失数据。
    - 注意：一定要考虑好内存块个数，如存入的ID计算出来的位置大于分配的区域，会擦除第一页后，循环存储，会丢失重复地址的整页旧数据。
- 不定长存储类型(CYCLE_FLASH_FILESYSTEM_OBJECT_TYPE_VARIABLE_LENGTH)
    - 一定要有目录，否则初始化失败；
    - 目录页建议至少2页，否则目录长度超出后，会擦除第一页，循环存储，会丢失重复地址的整页旧目录数据。
