

#include "stm32f10x.h"
#include "Timer.h"
#include "hardware.h"
#include "Serial.h"
/**
  * @brief  电机硬件初始化（方向控制引脚）
  * @note   PWM底层初始化由Motor_Init()内部调用PWM_Init()完成
  */
void Motor_Init(void)
{
    /* 开启GPIOA时钟（方向控制引脚在GPIOA上） */
    RCC_APB2PeriphClockCmd(MOTOR_CLK, ENABLE);

    /* 方向控制引脚配置 - 推挽输出，初始为低电平 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = MOTOR_In4  | MOTOR_In2 | MOTOR_In1 | MOTOR_In3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_PORT, &GPIO_InitStructure);

//    /* 所有方向引脚初始化为低电平 */
//    GPIO_ResetBits(MOTOR_PORT, MOTOR_In4 | MOTOR_In3 | MOTOR_In2 | MOTOR_In1);

    /* 初始化PWM底层定时器 */
    PWM_Timer_Init();
}

void Motor_SetSpeedLeft(int8_t Speed)
{
	if(Speed > 0)
	{
		if(Speed > 100)
		{
			Speed = 100;
		}
		TIM_SetCompare1(TIM4,Speed);
		TIM_SetCompare2(TIM4,0);
	}
	else if(Speed < 0)
	{
		if(Speed < -100)
		{
			Speed = -100;
		}
		Speed = -Speed;
		TIM_SetCompare1(TIM4,0);
		TIM_SetCompare2(TIM4,Speed);
	}
	else {
		TIM_SetCompare1(TIM4,0);
		TIM_SetCompare2(TIM4,0);
	}
}


void Motor_SetSpeedRight(int8_t Speed)
{
	if(Speed > 0)
	{
		if(Speed > 100)
		{
			Speed = 100;
		}
		TIM_SetCompare3(TIM4,Speed);
		TIM_SetCompare4(TIM4,0);
	}
	else if(Speed < 0)
	{
		if(Speed < -100)
		{
			Speed = -100;
		}
		Speed = -Speed;
		TIM_SetCompare3(TIM4,0);
		TIM_SetCompare4(TIM4,Speed);
	}
	else {
		TIM_SetCompare3(TIM4,0);
		TIM_SetCompare4(TIM4,0);
	}
}
