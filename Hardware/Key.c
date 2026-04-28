#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Key.h"

/*全局变量，按键状态*/
uint8_t Key_Num;
static uint8_t key_state[KEY_NUM];  //0松开1按下
static uint8_t key_event[KEY_NUM];  //保存事件
static uint16_t key_timer[KEY_NUM]; //事件计时
static uint8_t key_flag[KEY_NUM];   //状态机

/*时间参数*/
#define TIME_LONG        1000				//超过1000ms算长按
#define TIME_DBLCLICK    250        //双击间隔
#define DEBOUNCE         20         //消抖20ms

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//开启GPIOC的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将PB1和PB0引脚初始化为上拉输入
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);		//将PA5 引脚初始化为上拉输入
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);    //将PC13引脚初始化为上拉输入
}



/**
  * 函    数：获取按键状态
  * 参    数：按键i
  * 返 回 值：有按键按下，直接返回键码（非阻塞），没有按键按下，返回0
  */
uint8_t Key_GetState(uint8_t i)
{
	if (i == KEY1)
	{
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
		{
			return 1;
		}
	}
	else if (i == KEY2)
	{
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)
		{
			return 1;
		}
	}
	else if (i == KEY3)
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)
		{
			return 1;
		}
	}
	else if (i == KEY4)
	{
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0)
		{
			return 1;
		}
	}
	return 0;
}



void Key_Tick(void)
{
	uint8_t n;
	uint8_t pin_value;//按键是否按下，按下为1，松开为0
	for(n=0;n<KEY_NUM;n++)
	{
		pin_value = Key_GetState(n);//读取按键是按下还是松开
		//状态0：空闲，等待按下
		if(key_flag[n] == 0)
		{
			if(pin_value == 1)
			{
				key_timer[n] = DEBOUNCE;
				key_flag[n] = 1;
			}
		}
		//状态1：消抖
		else if(key_flag[n] == 1)
		{
			if(pin_value == 1)
			{
				if(--key_timer[n] == 0)
				{
					key_state[n] = 1;
					key_timer[n] = TIME_LONG;
					key_flag[n]  = 2;
				}
			}
			else
			{
				key_flag[n] = 0;
			}
		}
		//状态2：等待松开或者长按
		else if(key_flag[n] == 2)
		{
			if(pin_value == 0)
			{
				key_state[n] = 0;
				key_timer[n] = TIME_DBLCLICK;
				key_flag[n]  = 3;
			}
			else
			{
				if(--key_timer[n] == 0)
				{
					key_event[n] |= KEY_LONG;
					key_flag[n] = 4;
				}
			}
		}
		//状态3：等待双击
		else if(key_flag[n] ==3)
		{
			if(pin_value  == 1)
			{
				key_event[n] |= KEY_DBLCLICK;
				key_state[n] = 1;
				key_flag[n] = 4;
			}
			else 
			{
				if(--key_timer[n] == 0)
				{
					key_event[n] |= KEY_CLICK;
					key_flag[n] = 0;
				}
			}
		}
		//状态4：等待松开复位
		else if(key_flag[n] == 4)
		{
			if(pin_value ==0)
			{
				key_state[n] = 0;
				key_flag[n] = 0;
			}
		}
	}	
	
}

uint8_t Key_GetEvent(uint8_t key)
{
	uint8_t temp;
	
	if(key >=KEY_NUM) return 0;
	temp = key_event[key];
	key_event[key] = 0;
	
	return temp;
}

