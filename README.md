## 单片机多机通信基于485总线

开发工具：Keil5，Proteus， Altium

开发语言：C51

文档链接：https://www.yuque.com/g/bemount/qox9wu/collaborator/join?token=CLyUbTTv2YcwoD9x# (可阅读)

文档内容概览

- 中断问题
- 相关定时器寄存器
- 相关串口寄存器
- 多机通信流程
- 单片机类论文章节

总结

本系统采用的设计为一主两从，从机与从机之间不能直接交流，信息的交流需要从主机协调，采用RS485串口来进行通信，信息的接收与发送都是通过中断的方式。


