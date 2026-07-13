#ifndef __MOTOR_H
#define __MOTOR_H

/**
 * @brief  直流电机驱动模块头文件
 * @note   双H桥左右轮独立控制:
 *         Motor_SetSpeedLeft() - 左轮 (TIM2_CH3/PA2, 方向PA6+PA1)
 *         Motor_SetSpeedRight() - 右轮 (TIM2_CH4/PA3, 方向PA8+PA9)
  */

void Motor_Init(void);
void Motor_SetSpeedLeft(int8_t Speed);
void Motor_SetSpeedRight(int8_t Speed);

#endif
