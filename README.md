# Cycle Flash_v2

It is used to circulate data into the specified area of FLASH in MCU to ensure data safety and extend the life of FLASH area.  
> 本分支为 v2版本， 还在缓迭代中。

## 1、介绍

CycleFlash V2 从上 V1 版本重构过来，用于管理32位MCU内置flash的轻量级管理系统，主要实现类时序数据库循环写入某个片区的功能，但是分区管理数存入数据，目前主要用于离线的计时时钟、用户设置用户设置数据记录功能。
实现了写平衡、数据错误校验、内存可用数据回溯功能，针对不同的32位MCU写入颗粒，在 "cfs_user_config.h" 可配置不同的写入颗粒大小、flash一页大小、内存校验方式，详情请看看具体配置。

## 2、具体

使用方式请参考DEMO中的 "demo\CFS_V2_DEMO_HC32L176KATA\cfs\cfs_demo.c"。

这里存入数据格式为 ： ID_4byte + len_2byte + 校验_2byte + 用户数据_自定义byte
1. ID： 范围 0~ (类型最大值-10u), ID的累加管理由数据库自己完成，用户不需要关心。
2. len： 用户存入数据长度。
3. 校验: 根据用户配置的校验方式，对 "ID + len" 校验的值 和 "用户数据" 校验的值进行相加得到校验值。（用户存入数据只校验有效数据）
4. 用户数据: 这里的最大长度在初始化数据库分区的时候就设置好。

用户使用步骤：
1. 对 "cycleflash\include\cfs_user_config.h" 根据使用的32位MCU进行配置。
2. 根据上面的配置实现 "cycleflash\port\cfs_port_device_flash.c" 相对应的flash写入读取接口。
3. 在要使用的文件添加 "cycleflash\include\cycle_flash_system.h" 头文件，使用其中的API。
4. 要先初始化不同的数据库分区后，在使用其他API。
