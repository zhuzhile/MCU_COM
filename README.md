## 单片机多机通信协议设计与实现——基于485总线

开发工具：Keil5，Proteus， Altium

单片机型号：STC89C52

开发语言：C51

文档链接：https://www.yuque.com/docs/share/29a2f5a3-8533-4fe9-81b0-f587883a25c4?# 《单片机多机通信协议设计与实现——基于485总线》

文档内容概览

- 中断介绍
- 定时器相关寄存器
- 串口通信相关寄存器
- 多机通信流程
- 单片机类论文章节

总结

本系统采用的设计为一主两从，从机与从机之间不能直接交流，信息的交流需要从主机协调，采用RS485串口来进行通信，信息的接收与发送都是通过中断的方式。


