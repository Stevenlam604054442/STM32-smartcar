#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Motor.h"
#include "sound.h"
#include "IR.h"
#include "remote.h"
uint8_t RxData;			//定义用于接收串口数据的变量
int main(void)
{

//	uint8_t KeyRL;		//定义用于接收按键键码的变量
//	int8_t Speed=50;		//定义速度变量
//	/*模块初始化*/
//	OLED_Init();		//OLED初始化
//	Remote_Init;
//	IR_Init();
//	IR_Configuration();
//	Serial_Init();
//	Key_Init();
//	Motor_Init();		//直流电机初始化
//	/*OLED显示*/
//	
//	//OLED_ShowString(1, 3, "HelloWorld!");	//1行3列显示字符串HelloWorld!
//	
//	Remote_ON();
//	int add=9,com=9;
//	unsigned char Command;
//	OLED_ShowHexNum(1, 8, add, 2);
//	while (1)
//	{

//		if (Serial_GetRxFlag() == 1)			//检查串口接收数据的标志位
//		{
//			RxData = Serial_GetRxData();		//获取串口接收的数据
//			Serial_SendByte(RxData);			//串口将收到的数据回传回去，用于测试
//			OLED_ShowHexNum(1, 8, add, 2);	//显示串口接收的数据
//		}

//		//Motor_SetSpeed(30);
//		KeyRL = Key_GetRL();	//获取按键键码
//		
//		if (KeyRL == 1)			//按键1按下
//		{
//		
//		if(IR_GetDataFlag()||IR_GetRepeatFlag()){
//			 Command=IR_GetCommand();
//			 OLED_ShowHexNum(2,3,Command,9);
//		}
//		
//		
//		OLED_ShowNum(3,3,com,5);
//		
//		
//		
//		
//		
//		
//	}
		//			Motor_SetSpeedRL(50);				//设置直流电机的速度为速度变量
//		}
//		if (KeyRL == 0)			//按键1按下
//		{
//			OLED_ShowString(1, 3, "0000");
//		}
//		if (KeyRL == 2)			//按键2按下
//		{
//			OLED_ShowString(1, 3, "2222");
//			Motor_SetSpeedRL(-40);
//		}
//		if (KeyRL == 3)			//按键2按下
//		{
//			OLED_ShowString(1, 3, "3333");
//			Motor_SetSpeedUD(40);
//		}
//		if (KeyRL == 4)			//按键2按下
//		{
//			OLED_ShowString(1, 3, "4444");
//			Motor_SetSpeedUD(-40);
//		}
//		Delay_ms(8000);
//		KeyRL =0;
//		Motor_SetSpeedRL(0);
//		Motor_SetSpeedUD(0);
//	}
uint16_t T;
OLED_Init();		//OLED初始化
sound_Init();
while(1)
{
	
	
	T = sound_GetValue();
	OLED_ShowNum(1,2,T,10);
	Delay_ms(100);
	
}
//uint8_t RxData;			//定义用于接收串口数据的变量
/*模块初始化*/
//	OLED_Init();		//OLED初始化
//	
//	/*显示静态字符串*/
//	OLED_ShowString(1, 1, "RxData:");
//	
//	/*串口初始化*/
//	Serial_Init();		//串口初始化
//	
//	while (1)
//	{
//		if (Serial_GetRxFlag() == 1)			//检查串口接收数据的标志位
//		{
//			RxData = Serial_GetRxData();		//获取串口接收的数据
//			Serial_SendByte(RxData);			//串口将收到的数据回传回去，用于测试
//			OLED_ShowNum(1, 8, RxData, 2);	//显示串口接收的数据
//		}
//	}

}
