#include "stm32f10x.h"                  // Device header
#include "hardware.h"
#include "FreeRTOS.h"
#include "task.h"

void IRtracking_Init(void)
{
    /* ¿ªÆôGPIOÊ±ÖÓ */
    RCC_APB2PeriphClockCmd(IRTRACKING_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = IRTRACKING_L|IRTRACKING_LL|IRTRACKING_M|IRTRACKING_RR|IRTRACKING_R;              
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IRTRACKING_PORT, &GPIO_InitStructure); 

}

uint8_t IRtracking_L(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_L)==0){
		//vTaskDelay(10);
//		if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_L)==0)
		return 1;
	}
	return 0;
}

uint8_t IRtracking_LL(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_LL)==0){
		//vTaskDelay(10);
//		if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_LL)==0)
		return 1;
	}
	return 0;
}
uint8_t IRtracking_M(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_M)==0){
		//vTaskDelay(10);
//		if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_M)==0)
		return 1;
	}
	return 0;
}
uint8_t IRtracking_R(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_R)==0){
		//vTaskDelay(10);
//		if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_R)==0)
		return 1;
	}
	return 0;
}
uint8_t IRtracking_RR(void)
{
    if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_RR)==0){
		//vTaskDelay(10);
//		if(GPIO_ReadInputDataBit(IRTRACKING_PORT, IRTRACKING_RR)==0)
		return 1;
	}
	return 0;
}