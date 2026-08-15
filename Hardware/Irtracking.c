#include "stm32f10x.h"                  // Device header
#include "hardware.h"

void IRtracking_Init(void)
{
    /* ¿ªÆôGPIOÊ±ÖÓ */
    RCC_APB2PeriphClockCmd(IRTRACKING_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = IRTRACKING_Lo|IRTRACKING_Ro;              
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRTRACKING_PORT, &GPIO_InitStructure); 

}

uint8_t IRtracking_Lo(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_Lo)==1)return 1;
	return 0;
}

uint8_t IRtracking_Ro(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_Ro)==1)return 1;
	return 0;
}


