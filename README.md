# Cycle Flash_v1

It is used to circulate data into the specified area of FLASH in MCU to ensure data safety and extend the life of FLASH area.  
> 已经开始迭代 v2 版本，本版本做了很多无用的功夫，以及存入flash的数据结构也有些不满意，V2版本重点关注解决存入的数据结构问题等。
> v2 版本请切换当前项目的分支到 CycleFlash_v2。

## 1、介绍

CycleFlash 是一个用于管理MCU内置flash的轻量级管理系统，有简陋的循环写入某个片区的功能，只要实现了相关的接口，就可以实现通过ID把数据存入内存中，做了写平衡、数据错误校验、内存可用数据回溯的功能。

主要是为了实现循环存入数据功能，方便用于于需要不断往flash中写入暂存数据的需求。  
比如记录一天的临时数据，后面在统一取出；或者防止断电丢失系统状态而间断存系统数据。

## 2、使用介绍

> **注意：**使用的时候要预留够两页flash大小 + 0.1K大小的RAM。
> 两页FLASH大小是因为如果跨页修改数据会动态申请两页大小的内存，0.1K大小的RAM是初始化的时候固定动态申请使用。

使用时先实现 cycleflash\port_device_flash.h 的接口。
使用时包含头文件 #include "cycle_flash_system.h" 即可。

- 初始化函数 `cfs_system_handle_t cfs_nv_object_init( cfs_system *temp_object, const char * const name)`
  - 需要填充结构体 `cfs_system cfs_system_init` 的系统配置，在通过函数初始化内存。
  - 函数会返回类型为 `cfs_system_handle_t` cfs对象句柄，后续对数据库的操作都要通过它来使用。

初始化完毕后不需要考虑存入地址在哪里，只要给它ID就行。
ID从0开始，如果ID为 FFFF FFFF 就是无ID。
ID要自己记录，可以通过接口得到内部ID记录到哪里了，只能加不能减。

**写入的数据大小请和自己设定的数据大小相同，如太短，缺少地方会用 0xFF 补齐。**

如果使用的是固定存储类型，要自己计算可使用的ID个数，不能超过了分配的内存页，否者直接报错。
计算方式：根据 `cfs_system_define.h` 的 `CFS_DATA_BLOCK_ACCOMPANYING_DATA_BLOCK_LEN` 加初始化时分配的数据大小，就为没次存入数据块的数据大小。

## 3、文档

- 关于系统设计的文档介绍：[design.md](./cycleflash/design.md)
