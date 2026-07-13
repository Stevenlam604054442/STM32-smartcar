#ifndef __MOTOR_H
#define __MOTOR_H

/**
  * @brief  直流电机驱动模块头文件
  * @note   双H桥四轮独立控制:
  *         Motor_SetSpeedRL() - 左右方向电机 (TIM2_CH3/PA2)
  *         Motor_SetSpeedUD() - 上下方向电机 (TIM2_CH4/PA3)
  */

void Motor_Init(void);
void Motor_SetSpeedRL(int8_t Speed);
void Motor_SetSpeedUD(int8_t Speed);

#endif
