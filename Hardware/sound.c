/**
  ******************************************************************************
  * @file    sound.c
  * @brief   HC-SR04 超声波测距模块驱动
  * @note
  *     - Trig: PB15 (推挽输出) — 发送触发脉冲
  *     - Echo: PB14 (下拉输入)  — 接收回波脉宽
  *     - 计时: TIM3 中断方式 (见 Timer.c)
  *
  *     测距公式:
  *       Distance(mm) = (Time × 0.0001s × 34000mm/s) / 2
  *                    = Time × 1.7 mm
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "Delay.h"
#include "hardware.h"
#include "Timer.h"
#include "FreeRTOS.h"
#include "task.h"

uint16_t Sound_Time;         
uint16_t distance;
uint8_t get_distance_flag;
/**
  * @brief  HC-SR04 硬件初始化
  */
void sound_Init(void)
{
    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(SOUND_CLK, ENABLE);
	/* 开启AFIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = SOUND_Trig;              
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SOUND_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(SOUND_PORT, SOUND_Trig);                     /* 默认输出低电平 */


    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = SOUND_Echo;              
    GPIO_Init(SOUND_PORT, &GPIO_InitStructure);
	
	// 把 Echo 映射到中断线
	GPIO_EXTILineConfig(SOUND_EXIT_PORT, SOUND_EXIT_Echo);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = SOUND_EXIT_Line;               
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;      // 中断模式（另一种是事件模式，几乎不用）
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   // 下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;                // 使能该中断线
	EXTI_Init(&EXTI_InitStructure);
	

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = SOUND_EXIT_Line_IRQ; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能该通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // 抢占优先级 = 5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // 响应优先级 = 0
	NVIC_Init(&NVIC_InitStructure); 

}

/**
  * @brief  发送超声波触发脉冲
  * @note   HC-SR04 要求至少 10us 的高电平触发脉冲
  *         发送后启动TIM3开始Echo脉宽计时
  */
void sound_Start(void)
{
	taskENTER_CRITICAL();//进入临界区

    GPIO_SetBits(SOUND_PORT, SOUND_Trig);        /* Trig = HIGH */
    Delay_us(45);                            /* 保持 25us (>10us要求) */
	
    GPIO_ResetBits(SOUND_PORT, SOUND_Trig);      /* Trig = LOW  */
	
	Sound_Time=0;
	TIM3->CNT=0;
	taskEXIT_CRITICAL();//退出临界区
}

/**
  * @brief  执行一次完整测距并返回距离值
  * @retval 距离值 (单位: mm), 范围: 20~4000 (HC-SR04有效量程)
  * @note   每次调用耗时约 100ms (等待Echo回波超时)
  */
uint16_t sound_GetValue(void)
{
	get_distance_flag=0;
    sound_Start();
	while(get_distance_flag==0){}
    return distance;
}
