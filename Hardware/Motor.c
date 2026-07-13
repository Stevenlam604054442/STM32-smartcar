/**
  ******************************************************************************
  * @file    Motor.c
  * @brief   直流电机驱动模块 - 双H桥四轮独立控制
  * @note
  *     - 底层PWM: TIM2 输出比较模式 (见 PWM.c)
  *       CH3(PA2) = 左轮速度 / CH4(PA3) = 右轮速度
  *     - 方向控制: GPIO 推挽输出
  *       RL组: PA6(左前) + PA7(左后) → Motor_SetSpeedRL()
  *       UD组: PA8(右前) + PA9(右后) → Motor_SetSpeedUD()
  *     - 速度范围: -100(全速反转) ~ +100(全速正转)
  *
  *     控制逻辑 (以RL组为例):
  *       正转(Speed>0): PA6=HIGH, PA7=LOW  → PWM输出到CH3
  *       反转(Speed<0): PA6=LOW,  PA7=HIGH → PWM输出到CH3
  *       停止(Speed=0): PWM占空比=0 (方向引脚状态无关)
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
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 所有方向引脚初始化为低电平 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

    /* 初始化PWM底层定时器（TIM2 + GPIO复用功能） */
    PWM_Init();
}

/**
  * @brief  设置左右方向电机速度
  * @param  Speed: 速度值, 范围 -100 ~ +100
  *         正值 = 正转(前进), 负值 = 反转(后退), 0 = 停止
  * @note   通过 PA6/PA7 组合控制转动方向, TIM2_CH3(PA2) 控制转速
  */
void Motor_SetSpeedRL(int8_t Speed)
{
    if (Speed >= 0)
    {
        /* 正转: PA6=HIGH(正向), PA7=LOW(反向) */
        GPIO_SetBits(GPIOA, GPIO_Pin_6);
        GPIO_ResetBits(GPIOA, GPIO_Pin_7);
        PWM_SetCompare3(Speed);              /* CCR3 = 占空比 */
    }
    else
    {
        /* 反转: PA6=LOW, PA7=HIGH(正向) — 与正转时引脚极性互换 */
        GPIO_ResetBits(GPIOA, GPIO_Pin_6);
        GPIO_SetBits(GPIOA, GPIO_Pin_7);
        PWM_SetCompare3((uint8_t)(-Speed));   /* 取绝对值为占空比 */
    }
}

/**
  * @brief  设置上下方向电机速度
  * @param  Speed: 速度值, 范围 -100 ~ +100
  *         正值 = 正转, 负值 = 反转, 0 = 停止
  * @note   通过 PA8/PA9 组合控制转动方向, TIM2_CH4(PA3) 控制转速
  *         可与 SetSpeedRL 配合实现差速转向
  */
void Motor_SetSpeedUD(int8_t Speed)
{
    if (Speed >= 0)
    {
        /* 正转: PA8=HIGH(正向), PA9=LOW(反向) */
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
        GPIO_ResetBits(GPIOA, GPIO_Pin_9);
        PWM_SetCompare4(Speed);              /* CCR4 = 占空比 */
    }
    else
    {
        /* 反转: PA8=LOW, PA9=HIGH(正向) — 与正转时引脚极性互换 */
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
        GPIO_SetBits(GPIOA, GPIO_Pin_9);
        PWM_SetCompare4((uint8_t)(-Speed));   /* 取绝对值为占空比 */
    }
}
