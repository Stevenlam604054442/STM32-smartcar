/**
  ******************************************************************************
  * @file    sound.c
  * @brief   HC-SR04 超声波测距模块驱动
  * @note
  *     - Trig: PB0 (推挽输出) — 发送触发脉冲
  *     - Echo: PA7 (下拉输入)  — 接收回波脉宽
  *     - 计时: TIM3 中断方式 (见 Timer.c)
  *
  *     测距公式:
  *       Distance(mm) = (Time × 0.0001s × 34000mm/s) / 2
  *                    = Time × 1.7 mm
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "Timer.h"
#include "Delay.h"

extern uint16_t Time;          /* Timer.c中定义的全局计时变量 */

/**
  * @brief  HC-SR04 硬件初始化
  */
void sound_Init(void)
{
    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    /* Trig引脚 (PB0) - 推挽输出，默认低电平 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;              /* PB0 = 超声波Trig */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);                     /* 默认输出低电平 */

    /* Echo引脚 (PA7) - 下拉输入（空闲时为低电平） */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;              /* PA7 = 超声波Echo */
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  发送超声波触发脉冲
  * @note   HC-SR04 要求至少 10us 的高电平触发脉冲
  *         发送后启动TIM3开始Echo脉宽计时
  */
void sound_Start(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_0);        /* Trig = HIGH */
    Delay_us(45);                            /* 保持 45us (>10us要求) */
    GPIO_ResetBits(GPIOB, GPIO_Pin_0);      /* Trig = LOW  */
    Timer_Init();                           /* 启动TIM3计时 */
}

/**
  * @brief  执行一次完整测距并返回距离值
  * @retval 距离值 (单位: mm), 范围: 20~4000 (HC-SR04有效量程)
  * @note   每次调用耗时约 100ms (等待Echo回波超时)
  */
uint16_t sound_GetValue(void)
{
    sound_Start();
    Delay_ms(100);                           /* 等待回波（含超时保护） */

    /*
     * 距离计算:
     *   Time 单位 = 0.1ms (由TIM3中断频率决定)
     *   声速 ≈ 34000 mm/s = 34 mm/ms
     *   单程距离 = Time × 0.1ms × 34 mm/ms / 2
     *            = Time × 1.7 mm
     */
    return ((uint32_t)Time * 17) / 10;
}
