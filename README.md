# Cycle Flash_v2

It is used to circulate data into the specified area of FLASH in MCU to ensure data safety and extend the life of FLASH area.  
> 本分支为 v2版本， 还在缓迭代中。

## 1、介绍

一个菜鸡在不断的思辨，不断的思考如何让这个项目更加易用，更加纯粹，更加有价值，每天写一粒沙子，不断的和自己斗争，复杂化，在简化。

无ID状态为 0x00000000,或0xffFFFFFF.
然后ID累加一定是从0x01开始，累加到 0xFFFFFFF0, 然后在从0x01开始累计。

第一页头8字节做循环状态显示：这里前面4字节和后面4字节做对比验证。
获取数据优先级从高到底排序：
1. 0F 0F 0F 0F (ID循环利用)
2. 0A 0A 0A 0A (开始使用)
3. 01 01 01 01 (本区域已经初始化)