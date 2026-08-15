#include "stm32f10x.h"

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~65535
  * @retval 无
  */
void Delay_us(uint16_t xus)
{
	TIM2->CNT = 0x00;					//清空当前计数值
	while(!(TIM2->CNT == xus));	
}

