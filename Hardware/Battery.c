#include "stm32f10x.h"
#include "Battery.h"

// 私有变量
static float g_BatteryVoltage = 8.4f;   // 滤波后的电压值
static BatteryState_t g_BatState = BAT_NORMAL; // 系统状态
//static uint16_t g_FlashCount = 0;       // LED闪烁计数器

/**
  * 函    数：ADC初始化 (PA4)
  */
void Battery_Init(void)
{
    // 1. 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72MHz / 6 = 12MHz

    // 2. 配置PA4为模拟输入
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置PC13为推挽输出 (LED灯)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // 4. 配置ADC1
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    // 5. ADC校准
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

/**
  * 函    数：读取ADC原始值并换算电压
  */
static float Battery_ReadRawVoltage(void)
{
    uint16_t AD_Value;
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    AD_Value = ADC_GetConversionValue(ADC1);

    // 换算公式：
    // (AD值 / 4095 * 3.3V) 是PA4感受到的电压
    // 乘以分压比 (10+5.1)/5.1 = 2.96078
    return (float)AD_Value * (3.3f / 4095.0f) * 2.96078f;
}

/**
  * 函    数：电压监测与报警逻辑 
  */
void Battery_Update(void)
{
    float current_v = Battery_ReadRawVoltage();

    // 1. 一阶低通滤波 (数值越小越平滑，0.02代表新权值)
    g_BatteryVoltage = g_BatteryVoltage * 0.98f + current_v * 0.02f;

    // 2. 状态判定阈值
    if (g_BatteryVoltage <= 6.8f) {
        g_BatState = BAT_STOP;
    } else if (g_BatteryVoltage <= 7.2f) {
        g_BatState = BAT_WARNING;
    } else {
        g_BatState = BAT_NORMAL;
    }

//     // 3. LED 视觉反馈 (PC13 低电平点亮)
//    g_FlashCount++;
//    if (g_BatState == BAT_NORMAL) {
//        // 正常：500ms翻转一次 (心跳感)
//        if (g_FlashCount % 100 == 0) GPIO_WriteBit(GPIOC, GPIO_Pin_13, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)));
//    } 
//    else if (g_BatState == BAT_WARNING) {
//        // 预警：100ms翻转一次 (急促闪烁)
//        if (g_FlashCount % 20 == 0) GPIO_WriteBit(GPIOC, GPIO_Pin_13, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13)));
//    } 
//    else if (g_BatState == BAT_STOP) {
//        // 停机：LED常亮 (RESET即点亮)
//        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
//    }
}

float Battery_GetVoltage(void) {
    return g_BatteryVoltage;
}

BatteryState_t Battery_GetState(void) {
    return g_BatState;
}
