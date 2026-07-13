/**
  ******************************************************************************
  * @file    Motor.c
  * @brief   直流电机驱动模块 - 双H桥左右轮独立控制
  * @note
  *     - 底层PWM: TIM2 输出比较模式 (见 PWM.c)
  *       CH3(PA2) = 左轮速度 / CH4(PA3) = 右轮速度
  *     - 方向控制: GPIO 推挽输出
  *       左轮组: PA6(正) + PA1(反) → Motor_SetSpeedLeft()
  *       右轮组: PA8(正) + PA9(反) → Motor_SetSpeedRight()
  *     - 速度范围: -100(全速反转) ~ +100(全速正转)
  *
  *     控制逻辑 (以左轮为例):
 *       正转(Speed>0): PA6=HIGH, PA1=LOW  → PWM输出到CH3
 *       反转(Speed<0): PA6=LOW,  PA1=HIGH → PWM输出到CH3
  *       停止(Speed=0): PWM占空比=0 (方向引脚状态不变)
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "PWM.h"

/**
  * @brief  电机硬件初始化（方向控制引脚）
  * @note   PWM底层初始化由Motor_Init()内部调用PWM_Init()完成
  */
void Motor_Init(void)
{
    /* 开启GPIOA时钟（方向控制引脚在GPIOA上） */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* 方向控制引脚配置 - 推挽输出，初始为低电平 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 所有方向引脚初始化为低电平 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9);

    /* 初始化PWM底层定时器（TIM2 + GPIO复用功能） */
    PWM_Init();
}

/**
  * @brief  设置左轮电机速度
  * @param  Speed: 速度值, 范围 -100 ~ +100
  *         正值 = 正转(前进), 负值 = 反转(后退), 0 = 停止
  * @note   通过 PA6/PA1 组合控制转动方向, TIM2_CH3(PA2) 控制转速
  */
void Motor_SetSpeedLeft(int8_t Speed)
{
    if (Speed > 0)          /* 正转 */
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_6);
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        PWM_SetCompare3(Speed);
    }
    else if (Speed < 0)     /* 反转 */
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
        PWM_SetCompare3((uint8_t)(-Speed));
    }
    else                    /* 停止: 只关PWM,不动方向脚 */
    {
        PWM_SetCompare3(0);
    }
}

/**
  * @brief  设置右轮电机速度
  * @param  Speed: 速度值, 范围 -100 ~ +100
  *         正值 = 正转, 负值 = 反转, 0 = 停止
  * @note   通过 PA8/PA9 组合控制转动方向, TIM2_CH4(PA3) 控制转速
  *         可与 SetSpeedLeft 配合实现差速转向
  */
void Motor_SetSpeedRight(int8_t Speed)
{
    if (Speed > 0)          /* 正转 */
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
        GPIO_ResetBits(GPIOA, GPIO_Pin_9);
        PWM_SetCompare4(Speed);
    }
    else if (Speed < 0)     /* 反转 */
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
        GPIO_SetBits(GPIOA, GPIO_Pin_9);
        PWM_SetCompare4((uint8_t)(-Speed));
    }
    else                    /* 停止: 只关PWM,不动方向脚 */
    {
        PWM_SetCompare4(0);
    }
}
