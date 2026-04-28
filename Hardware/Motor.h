#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"                  // Device header

void Motor_Init(void);
void Motor_SetPWM(uint8_t n, int16_t PWM);

extern int8_t DeadZ; 

#endif
