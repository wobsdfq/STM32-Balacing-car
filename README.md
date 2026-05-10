# STM32 Self-Balancing Robot

### 实物展示 (Gallery)
<img width="4096" height="3072" alt="IMG_20260307_015039" src="https://github.com/user-attachments/assets/ed9ac165-b24d-4ed2-8521-8faae740fb61" />
<img width="4096" height="3072" alt="IMG_20260307_015035" src="https://github.com/user-attachments/assets/ac893a8d-3f5d-4293-82cd-034d74ed22b7" />

### 演示视频 (Demo Video)
https://github.com/user-attachments/assets/6c348aae-793f-4dab-a513-37c07ef7cacd

### 项目亮点 (Highlights)
① 项目简介 (Introduction)
基于 STM32 的串级 PID 双轮自平衡机器人
本项目是本人大三期间独立完成的嵌入式练习项目。实现了从硬件 PCB 设计、元器件焊接、底层驱动编写到闭环控制算法整定的完整闭环。

② 核心功能 (Key Features)
姿态解算：采用 MPU6050 结合一阶互补滤波算法，实现 200Hz 的稳定角度输出。
控制算法：采用串级 PID（直立环 PD + 速度环 PI + 转向环 P），实现高响应自平衡。
交互系统：集成 0.96 寸 OLED 多级菜单显示与蓝牙无线调参。
电源管理：具备两级欠压保护逻辑，实时监控 2S 锂电池电压。

③ 硬件清单 (Hardware List)
主控：STM32F103C8T6
传感器：MPU6050 (6轴 IMU)
电机：JGA25-370 编码电机 (280 RPM)
驱动：TB6612FNG
电源：MINI560 高效率降压模块 + 18650 锂电池 (2S)
