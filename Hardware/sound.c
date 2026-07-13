#include "stm32f10x.h"                  // Device header
#include "Timer.h"
#include "Delay.h"

extern uint16_t Time;
void sound_Init()
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//trig
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//echo
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	
}
void sound_Start()
{
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
	Delay_us(45);
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	Timer_Init();
	
}

uint16_t sound_GetValue()
{
	sound_Start();
	Delay_ms(100);

	return ((Time*0.0001)*34000)/2;
	
	
}


