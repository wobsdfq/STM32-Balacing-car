#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

//按键编号
#define KEY1  	 0
#define KEY2  	 1
#define KEY3  	 2
#define KEY4  	 3
#define KEY_NUM  4

//按键事件
#define KEY_CLICK			  0x01   			//单击0000 0001
#define KEY_DBLCLICK    0x02   			//双击0000 0010
#define KEY_LONG  			0x04	 			//长按0000 0100

void Key_Init(void);   				 			//初始化
uint8_t Key_GetEvent(uint8_t key);	//获取按键事件
void Key_Tick(void);					 			//1ms调用一次

#endif
