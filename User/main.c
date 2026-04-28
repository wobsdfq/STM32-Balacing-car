#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "BlueSerial.h"
#include "Battery.h"
#include <math.h>
#include "PID.h"
#include <string.h>
#include <stdlib.h>

float AngleAcc,AngleGyro,Angle;
uint8_t TimerErrorFlag;
uint16_t TimerCount;
uint8_t KeyNum, RunFlag;
int16_t LeftPWM, RightPWM;
int16_t AvePWM, DifPWM;
float LeftSpeed, RightSpeed;
float AveSpeed, DifSpeed;
float User_Turn_Target, User_Speed_Target;

PID_t AnglePID = {
			.Kp=3.54,
			.Ki=0.08,
			.Kd=3.38,
			.OutMax = 100,
			.OutMin = -100,
			.OutOffset = 2,
			//.ErrorIntMax = 500,
		  //.ErrorIntMin = 500,
};

PID_t SpeedPID = {
			.Kp=3.85,
			.Ki=0.05,
			.Kd=0.3,
			.OutMax = 35,
			.OutMin = -35,
	
			//.ErrorIntMax = 200,
			//.ErrorIntMin = 200,
};

PID_t TurnPID = {
			.Kp=3.8,
			.Ki=0,
			.Kd=0,
			.OutMax = 50,
			.OutMin = -50,
			//.ErrorIntMax = 20,
			//.ErrorIntMin = 20,
};

uint32_t OLED_Timer = 0;
uint8_t levelspeed =2;
uint8_t levelturn =2;
uint8_t UI =1;
int16_t Turn_FeedForward = 0;


int main(void)
{
		//模块初始化
		OLED_Init();		//OLED初始化
		MPU6050_Init();		//MPU6050初始化
		BlueSerial_Init(); //蓝牙模块初始化
		Timer_Init();		//定时器初始化，1ms定时中断一次
		LED_Init();
		Key_Init();
		Motor_Init();
		Encoder_Init();
		Serial_Init();
		Battery_Init();
while (1)
{

	// 1. 一次性捕获按键事件
    uint8_t event1 = Key_GetEvent(KEY1);
    uint8_t event2 = Key_GetEvent(KEY2);
    uint8_t event3 = Key_GetEvent(KEY3);
    uint8_t event4 = Key_GetEvent(KEY4);

    // KEY1 逻辑
    if(event1 & KEY_CLICK) 
		{
					if(RunFlag == 0)
					{
						PID_Init(&AnglePID);
						PID_Init(&SpeedPID);
						PID_Init(&TurnPID);
						RunFlag = 1;
					}
			else
					{
						RunFlag = 0;
					}
    }

    // KEY2 逻辑 (单击加速度，长按减速度)
    if(event2 & KEY_CLICK) { if(levelspeed < 3) levelspeed++; }
    if(event2 & KEY_LONG)  { if(levelspeed > 1) levelspeed--; } 

    // KEY3 逻辑 (单击加转向，长按减转向)
    if(event3 & KEY_CLICK) { if(levelturn < 3) levelturn++; }
    if(event3 & KEY_LONG)  { if(levelturn > 1) levelturn--; } 

    // KEY4 逻辑 (切换UI)
    if(event4 & KEY_CLICK) { if(UI < 3) UI++; }
    if(event4 & KEY_LONG)  { if(UI > 1) UI--; }
	
	
	if (BlueSerial_RxFlag == 1)
	{
		char *Tag = strtok(BlueSerial_RxPacket, ",");
		if (strcmp(Tag, "key") == 0)
		{
			char *Name = strtok(NULL, ",");
			char *Action = strtok(NULL, ",");
			
		}
		else if (strcmp(Tag, "slider") == 0)
		{
			char *Name = strtok(NULL, ",");
			char *Value = strtok(NULL, ",");
			
			if (strcmp(Name, "AngleKp") == 0)
			{
				AnglePID.Kp = atof(Value);
			}
			else if (strcmp(Name, "AngleKi") == 0)
			{
				AnglePID.Ki = atof(Value);
			}
			else if (strcmp(Name, "AngleKd") == 0)
			{
				AnglePID.Kd = atof(Value);
			}
			
			else if (strcmp(Name, "SpeedKp") == 0)
			{
				SpeedPID.Kp = atof(Value);
			}
			else if (strcmp(Name, "SpeedKi") == 0)
			{
				SpeedPID.Ki = atof(Value);
			}
			else if (strcmp(Name, "SpeedKd") == 0)
			{
				SpeedPID.Kd = atof(Value);
			}
			else if (strcmp(Name, "TurnKp") == 0)
			{
				TurnPID.Kp = atof(Value);
			}
			else if (strcmp(Name, "TurnKi") == 0)
			{
				TurnPID.Ki = atof(Value);
			}
			else if (strcmp(Name, "TurnKd") == 0)
			{
				TurnPID.Kd = atof(Value);
			}
			else if (strcmp(Name, "Offset") == 0)
			{
				AnglePID.OutOffset = atof(Value);
			}
			else if (strcmp(Name, "DZ") == 0)
			{
				DeadZ = atof(Value);
			}
		}
		else if (strcmp(Tag, "joystick") == 0)
		{
			int8_t LH = atoi(strtok(NULL, ","));
			int8_t LV = atoi(strtok(NULL, ","));
			int8_t RH = atoi(strtok(NULL, ","));
			int8_t RV = atoi(strtok(NULL, ","));
			
			User_Speed_Target = (LV / 100.0f) * (levelspeed * 1.2f);
			User_Turn_Target = RH / 75.0*levelturn;//1,2,3
			Turn_FeedForward = RH / 8*levelturn; // 直接转向力
			
		}
		
		BlueSerial_RxFlag = 0;
	}
	if(UI==1)
	{
		OLED_Clear();
		OLED_Printf(0, 0, OLED_6X8, "Angle  Speed  Turn");
		OLED_Printf(0, 10, OLED_6X8, "%+06.1f %+06.1f %+06.1f", AnglePID.Target, SpeedPID.Target, TurnPID.Target);
		OLED_Printf(0, 18, OLED_6X8, "%+06.1f %+06.1f %+06.1f", Angle, AveSpeed, DifSpeed);
		OLED_Printf(0, 26, OLED_6X8, "%+06.1f %+06.1f %+06.1f", AnglePID.Out, SpeedPID.Out, TurnPID.Out);
		
		OLED_DrawLine(0, 36, 127, 36);
		
		OLED_Printf(0, 40, OLED_6X8, "PWML:%+05d PWMR:%+05d", LeftPWM, RightPWM);
		OLED_Printf(0, 48, OLED_6X8, "SpdL:%+05.1f SpdR:%+05.1f", LeftSpeed, RightSpeed);
		OLED_Printf(0, 56, OLED_6X8, "SpdLevel:%1d Trlevel:%1d", levelspeed,levelturn);
		OLED_Update();
	}
	else if(UI==2)
	{
		  OLED_Clear();
			OLED_Printf(0,0,OLED_6X8, "Angle");
			OLED_Printf(0,8,OLED_6X8, "P:%05.2f",AnglePID.Kp);
			OLED_Printf(0,16,OLED_6X8, "I:%05.2f",AnglePID.Ki);
			OLED_Printf(0,24,OLED_6X8, "D:%05.2f",AnglePID.Kd);
			OLED_Printf(0,32,OLED_6X8, "T:%05.2f",AnglePID.Target);
			OLED_Printf(0,40,OLED_6X8, "A:%05.2f",Angle);
			OLED_Printf(0,48,OLED_6X8, "O:%05.2f",AnglePID.Out);
			OLED_Printf(0,56,OLED_6X8, "GY:%d",GY);
			//OLED_Printf(56,56,OLED_6X8, "Offset:%02.0f",AnglePID.OutOffset);
		  OLED_Printf(56,56,OLED_6X8, "DeadZ:%d",DeadZ);
			
			OLED_Printf(50,0,OLED_6X8, "Speed");
			OLED_Printf(50,8,OLED_6X8, "%05.2f",SpeedPID.Kp);
			OLED_Printf(50,16,OLED_6X8, "%05.2f",SpeedPID.Ki);
			OLED_Printf(50,24,OLED_6X8, "%05.2f",SpeedPID.Kd);
			OLED_Printf(50,32,OLED_6X8, "%05.2f",SpeedPID.Target);
			OLED_Printf(50,40,OLED_6X8, "%05.2f",AveSpeed);
			OLED_Printf(50,48,OLED_6X8, "%05.2f",SpeedPID.Out);
			
			OLED_Printf(88,0,OLED_6X8, "Turn");
			OLED_Printf(88,8,OLED_6X8, "%05.2f",TurnPID.Kp);
			OLED_Printf(88,16,OLED_6X8, "%05.2f",TurnPID.Ki);
			OLED_Printf(88,24,OLED_6X8, "%05.2f",TurnPID.Kd);
			OLED_Printf(88,32,OLED_6X8, "%05.2f",TurnPID.Target);
			OLED_Printf(88,40,OLED_6X8, "%05.2f",DifSpeed);
			OLED_Printf(88,48,OLED_6X8, "%05.2f",TurnPID.Out);
			
				/*OLED更新*/
			OLED_Update();
	}
	else if(UI==3)
	{
			// 1. 获取当前电压
			float volt = Battery_GetVoltage();
			BatteryState_t state = Battery_GetState();
		  OLED_Clear();

			// 2. 显示电压数值
			OLED_Printf(0, 56, OLED_6X8, "BATT: %.2fV", volt);

			// 3. 根据状态显示文字提示
			if (state == BAT_NORMAL) {
					OLED_ShowString(70, 56, "  OK  ", OLED_6X8);
			} else if (state == BAT_WARNING) {
					OLED_ShowString(70, 56, " LOW! ", OLED_6X8); // 闪烁提醒
			} else if (state == BAT_STOP) {
					// 强制满屏显示警告
					OLED_Clear();
					OLED_Printf(30, 24, OLED_8X16, "!!! OFF !!!");
					OLED_Printf(30, 40, OLED_8X16, "CHARGE NOW");
					OLED_Update();
					// 此时主程序通常会进入死循环或停止电机
			}
			// 4. 电池槽显示
			int bars = (int)((volt - 6.8f) / 1.6f * 10);
			if (bars < 0) bars = 0;
			if (bars > 10) bars = 10;
			OLED_ShowString(0, 48, "[", OLED_6X8);
			for(int i=0; i<10; i++) {
					if(i < bars) OLED_ShowString(6+i*6, 48, "|", OLED_6X8);
					else OLED_ShowString(6+i*6, 48, " ", OLED_6X8);
			}
			OLED_ShowString(66, 48, "]", OLED_6X8);
				/*OLED更新*/
			OLED_Update();
	}
	
	
	//BlueSerial_Printf("[plot, %f,%f]", TurnPID.Target, DifSpeed);
	BlueSerial_Printf("[plot, %f,%f,%f]", AnglePID.ErrorInt,SpeedPID.ErrorInt,TurnPID.ErrorInt);
}
}



void TIM1_UP_IRQHandler(void)
{
static uint16_t Count0,Count1,Count2;

if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
{
	/*定时中断函数1ms自动执行一次*/

	/*进入中断函数后，立刻清标志位*/
	/*如果中断函数退出前，标志位又置1了，说明中断函数执行时间超过了定时时间（1ms）*/
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	
	Key_Tick();
	
	Count0 ++;
	if(Count0>=10)
	{
		Count0=0;
		
		/*在中断里读取MPU6050，可以保证读取间隔严格为1ms*/
		/*但要保证MPU6050_GetData执行时间不超过1ms*/
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
		float Alpha = 0.01;
		AngleAcc = -atan2(AX,AZ)/3.14159*180+9.75;
		AngleGyro = Angle + GY/32768.0*2000*0.01;
		Angle = Alpha * AngleAcc + (1-Alpha) * AngleGyro; //互补滤波
		
		if(Angle > 50 || Angle < -50)
		{
			RunFlag = 0;
		}
		if(RunFlag)
		{
			AnglePID.Actual = Angle;
			PID_Update(&AnglePID);
			AvePWM = -AnglePID.Out;
			
			LeftPWM = AvePWM + DifPWM / 2;
			RightPWM = AvePWM - DifPWM / 2;
			
			if (LeftPWM > 100) {LeftPWM = 100;} else if (LeftPWM < -100) {LeftPWM = -100;}
			if (RightPWM > 100) {RightPWM = 100;} else if (RightPWM < -100) {RightPWM = -100;}
			
			Motor_SetPWM(1, LeftPWM);
			Motor_SetPWM(2, RightPWM);
		}
		else
		{
			Motor_SetPWM(1, 0);
			Motor_SetPWM(2, 0);
		}
	}
	
	Count1++;
	if(Count1 >= 50)
	{
		Count1=0;
		
		LeftSpeed = Encoder_Get(1) /44.0 /0.05 /21.3;
		RightSpeed = Encoder_Get(2) /44.0 /0.05 /21.3;
		
		AveSpeed = (LeftSpeed + RightSpeed) / 2.0;
		DifSpeed = LeftSpeed - RightSpeed;
		
		if(RunFlag)
		{
			// --- 在计算 PID 前，平滑目标值 ---
			// 每 50ms 更新一次目标，alpha 给 0.2 左右
			SpeedPID.Target = SpeedPID.Target * 0.8f + User_Speed_Target * 0.2f;
			TurnPID.Target  = TurnPID.Target  * 0.8f + User_Turn_Target  * 0.2f;
			
			SpeedPID.Actual = AveSpeed;
			PID_Update(&SpeedPID);
			AnglePID.Target = SpeedPID.Out;
			
			TurnPID.Actual = DifSpeed;
			PID_Update(&TurnPID);
			DifPWM = TurnPID.Out + Turn_FeedForward; 
		}
	}

	if (++Count2 >= 100)
					{
							Count2 = 0;
							Battery_Update(); // 内部包含 ADC 采样和一阶滤波
					}

	/*中断函数退出前，再次检查标志位*/
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		/*标志位又置1了，说明中断函数执行时间超过了定时时间（1ms）*/
		/*置TimerErrorFlag为1，表示定时中断错误*/
		TimerErrorFlag = 1;

		/*清标志位，避免中断连续触发，导致主函数完全无法执行*/
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}

	/*中断函数退出前，读取计数器的值，此值可用于测量中断函数的具体执行时间*/
	TimerCount = TIM_GetCounter(TIM1);
}

}


