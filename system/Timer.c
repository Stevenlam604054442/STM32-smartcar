/**
  ******************************************************************************
  * @file    Timer.c
  * @brief   超声波测距定时器 - TIM3 输入捕获/中断模式
  * @note    使用TIM3而非TIM2，因为TIM2已被PWM模块(电机控制)占用
  *          定时器频率: 72MHz / (PSC+1) / (ARR+1) = 72MHz / 1 / 7200 = 10kHz
  *          即每 0.1ms 触发一次中断，用于HC-SR04 Echo脉宽计时
  ******************************************************************************
  */

#include "stm32f10x.h"

/**
  * @brief  超声波Echo脉宽计数值（全局变量，供sound.c读取）
  * @note   每计数一次代表 0.1ms
  */
uint16_t Time;

/**
  * @brief  TIM3 初始化 - 用于超声波测距的Echo信号高电平计时
  */
void Timer_Init(void)
{
    Time = 0;

    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);     // 使能TIM3时钟

    /* 配置时钟源为内部时钟 */
    TIM_InternalClockConfig(TIM3);

    /* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;      // 不分频（用于滤波器时钟）
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 7199;                      // ARR: 计数周期 0~7199
    /*
     * 计数器溢出频率:
     *   CK_CNT_OV = CK_PSC / (PSC + 1) / (ARR + 1)
     *             = 72MHz / (0 + 1) / (7199 + 1)
     *             = 10kHz (每 0.1ms 溢出一次)
     */
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;                     // PSC: 不预分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器（仅高级定时器用）
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    /* 清除更新标志位（防止开启中断后立刻进入一次中断） */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);

    /* 使能更新中断 */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* NVIC 中断配置（分组2: 抢占优先级0~3, 响应优先级0~3） */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                 // TIM3全局中断通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  // 使能该通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;        // 抢占优先级 = 2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;               // 响应优先级 = 1
    NVIC_Init(&NVIC_InitStructure);

    /* 使能TIM3定时器 */
    TIM_Cmd(TIM3, ENABLE);
}

/**
  * @brief  TIM3 中断服务函数 - Echo高电平期间持续计时
  * @note   当PA7(Echo)为高电平时，Time++；低电平时停止计数
  *         sound_GetValue()读取Time后计算距离并清零
  */
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        /* 仅在Echo引脚为高电平时累加计时 */
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == 1)
        {
            Time++;
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
