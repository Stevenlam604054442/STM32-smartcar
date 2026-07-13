#ifndef __PWM_H
#define __PWM_H

/**
  * @brief  PWM输出模块头文件 (TIM2 输出比较模式)
  * @note   用于直流电机速度控制:
  *         CH3(PA2) = 左轮 / CH4(PA3) = 右轮
  *         频率: 20kHz (PSC=36, ARR=99 @72MHz)
  */

void PWM_Init(void);
void PWM_SetCompare3(uint16_t Compare);
void PWM_SetCompare4(uint16_t Compare);

#endif
