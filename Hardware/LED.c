/**
  ******************************************************************************
  * @file    LED.c
  * @brief   LED指示灯驱动 - PB4/PB5 推挽输出, 低电平点亮(共阳)
  * @note
  *     LED1 (PB4) = 运行指示灯: 电机工作时亮起
  *     LED2 (PB5) = 状态指示灯:  避障告警/模式反馈
  ******************************************************************************
  */

#include "stm32f10x.h"

/**
  * @brief  LED硬件初始化
  * @note   PB4=LED1, PB5=LED2, 推挽输出, 初始高电平(LED熄灭)
  */
void LED_Init(void)
{
    /* 开启GPIOB时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          /* 推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5;   /* PB4 + PB5 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 初始高电平 → LED全部熄灭(共阳极) */
    GPIO_SetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_5);
}

/* ======================== LED1 (PB4) 运行指示灯 ======================== */

void LED1_ON(void)  { GPIO_ResetBits(GPIOB, GPIO_Pin_4); }      /* 低电平点亮 */
void LED1_OFF(void) { GPIO_SetBits(GPIOB, GPIO_Pin_4); }       /* 高电平熄灭 */

void LED1_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_4) == 0)
        GPIO_SetBits(GPIOB, GPIO_Pin_4);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_4);
}

/* ======================== LED2 (PB5) 状态/告警灯 ======================== */

void LED2_ON(void)  { GPIO_ResetBits(GPIOB, GPIO_Pin_5); }      /* 低电平点亮 */
void LED2_OFF(void) { GPIO_SetBits(GPIOB, GPIO_Pin_5); }       /* 高电平熄灭 */

void LED2_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_5) == 0)
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
}
