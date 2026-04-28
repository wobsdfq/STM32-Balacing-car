#ifndef __BATTERY_H
#define __BATTERY_H

#include "stm32f10x.h"

// 定义系统电压状态枚举
typedef enum {
    BAT_NORMAL = 0,   // 电压正常 (>7.2V)
    BAT_WARNING = 1,  // 低压预警 (6.8V ~ 7.2V)
    BAT_STOP = 2      // 强制停机 (<= 6.8V)
} BatteryState_t;

// 函数声明
void Battery_Init(void);          // 初始化ADC和状态
void Battery_Update(void);        // 调用一次更新电压和状态
float Battery_GetVoltage(void);   // 获取当前滤波后的电压值
BatteryState_t Battery_GetState(void); // 获取当前系统状态

#endif
