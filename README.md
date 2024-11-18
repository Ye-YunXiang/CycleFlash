# Cycle Flash_v2

It is used to circulate data into the specified area of FLASH in MCU to ensure data safety and extend the life of FLASH area.  
> 本分支为 v2版本， 还在缓迭代中。

## 1、介绍

一个菜鸡在不断的思辨，不断的思考如何让这个项目更加易用，更加纯粹，更加有价值，每天写一粒沙子，不断的和自己斗争，复杂化，在简化。

这里没有做ID回收机制，请自己盘算一下，flash能不能活到循环的把uint32_t的ID用完，如果真的可以，这里提供了了个备用选项，使用uint64_t作为ID计数，如果要使用请在 cfs_user_config.h 修改宏。

第一页头8字节做循环状态显示：这里前面4字节和后面4字节做对比验证。
获取数据优先级从高到底排序：
1. 0F 0F 0F 0F (开始使用)
2. 01 01 01 01 (本区域已经初始化)

每次存入的数据块大小，最大数据长度为 UINT16_T_MAX