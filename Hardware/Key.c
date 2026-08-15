#include "stm32f10x.h"
#include "hardware.h"
#include "FreeRTOS.h"
#include "task.h"

void KEY_Init(void)
{
    /* 开启GPIO时钟 */
    RCC_APB2PeriphClockCmd(KEY_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;              
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
	
}

uint8_t Get_key(void)
{
	if(GPIO_ReadInputDataBit(KEY_PORT,KEY_PIN)==0){
			vTaskDelay(40);
			if(GPIO_ReadInputDataBit(KEY_PORT,KEY_PIN)==0){
				while(GPIO_ReadInputDataBit(KEY_PORT,KEY_PIN)==0){}
				vTaskDelay(40);
				return 1;
			}
	}
		return 0;
}

