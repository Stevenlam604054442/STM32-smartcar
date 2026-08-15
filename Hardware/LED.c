#include "stm32f10x.h"
#include "hardware.h"

void LED_Init(void)
{

    RCC_APB2PeriphClockCmd(LED_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;          /* 推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G | LED_DP;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

    /* 初始高电平 → LED全部熄灭(共阳极) */
    GPIO_SetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G | LED_DP);
}

void LED_OFF(void){
	GPIO_SetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G | LED_DP);	
}
void LED_All_ON(void){
	GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G | LED_DP);	
}
void LED_show_num(uint8_t num){
	LED_OFF();
	switch(num)	{
		case 0:
			GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_E | LED_F );
		break;
		case 1:
			GPIO_ResetBits(LED_PORT, LED_C | LED_B );
		break;
		case 2:
			GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_G | LED_D | LED_E );
		break;
		case 3:
			GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_G );
		break;
		case 4:
			GPIO_ResetBits(LED_PORT, LED_B | LED_C | LED_F | LED_G );
		break;
		case 5:
			GPIO_ResetBits(LED_PORT, LED_A | LED_C | LED_D | LED_F | LED_G );
		break;
		case 6:
			GPIO_ResetBits(LED_PORT, LED_A | LED_C | LED_D | LED_E | LED_F | LED_G );
		break;
		case 7:
			GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_C );
		break;
		case 8:
			GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G );
		break;
		case 9:
			GPIO_ResetBits(LED_PORT, LED_A | LED_B | LED_C | LED_D | LED_F | LED_G );
		break;
	}
}


