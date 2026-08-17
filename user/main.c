#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sound.h"
#include "Serial.h"
#include "queue.h"
#include "semphr.h"
#include "Timer.h"
#include "Irtracking.h"
#include "Motor.h"
#include "Key.h"
#include "LED.h"
#include "hardware.h"

//Sound_distance
void Sound_distance(void* pvParam);
#define Sound_distance_STACK_SIZE 128
#define Sound_distance_PRIORITY 1
TaskHandle_t Sound_distance_handle;

//SendMsg
void SendMsg(void* pvParam);
#define SendMsg_STACK_SIZE 256
#define SendMsg_PRIORITY 1
TaskHandle_t SendMsg_handle;
QueueHandle_t Distance_SendQueue;

//FindLine
void FindLine(void* pvParam);
#define FindLine_STACK_SIZE 256
#define FindLine_PRIORITY 1
TaskHandle_t FindLine_handle;
SemaphoreHandle_t FindLine_Semaphore;
SemaphoreHandle_t FindLine_getline_Semaphore;
uint8_t find_line;

//FollowLine
void FollowLine(void* pvParam);
#define FollowLine_STACK_SIZE 256
#define FollowLine_PRIORITY 1
TaskHandle_t FollowLine_handle;
#define right 2
#define left 1
#define no_turn 0
uint8_t last_line_turn;

void Sound_distance(void * pvParam)
{
	
	while(1)
	{

		sound_GetValue();
		xQueueSendToBack(Distance_SendQueue,&distance,portMAX_DELAY);
		vTaskDelay(100);
			
	}
		
}

void SendMsg(void * pvParam)
{
	int f=0;
	int8_t sp=0;
//	uint16_t getData;
	while(1)
	{
		
		
		
//		if(Get_key()==1){
//			f=!f;
//		}
////		if(xQueueReceive(Distance_SendQueue,&getData,portMAX_DELAY)==pdPASS)
////		{
		Serial_Printf("%u%u%u%u%u\r\n",IRtracking_LL(),IRtracking_L(),IRtracking_M(),IRtracking_R(),IRtracking_RR());
////			
////		}
//		if(f==1){
//			Motor_SetSpeedLeft(sp);
//			Motor_SetSpeedRight(sp);
//			vTaskDelay(10);
//			if(sp<70)sp++;
//		}
//		else{
//			Motor_SetSpeedLeft(0);
//			Motor_SetSpeedRight(0);
//		}

		
	}
		
}


void FindLine(void * pvParam){
	while(1)
	{
		if(xSemaphoreTake(FindLine_Semaphore,portMAX_DELAY)==pdPASS)
		{
			//LED_show_num(0);
			Motor_SetSpeedLeft(20);
			Motor_SetSpeedRight(-20);
			if(xSemaphoreTake(FindLine_getline_Semaphore,3000)==pdPASS)
			{
				//LED_show_num(1);
				
			}
			find_line=0;
			Motor_SetSpeedLeft(0);
			Motor_SetSpeedRight(0);
		}		
	}	
}

void FollowLine(void * pvParam)
{
	int8_t sp=100;
	int8_t err=0;
	int8_t K=100;
	int8_t adjust;
	last_line_turn=no_turn;
	while(1)
	{
		adjust=K*err;
		if(IRtracking_LL()==0&&IRtracking_L()==0&&IRtracking_RR()==0&&IRtracking_R()==0&&IRtracking_M()==0){
			err=0;
			sp=0;
			LED_show_num(0);
		}
		else if((IRtracking_LL()==1)&&(IRtracking_RR()==0||IRtracking_R()==0)){
			err=1;
			sp=0;
			K=100;
			LED_show_num(1);
			last_line_turn=left;
		}
		else if((IRtracking_L()==1)&&(IRtracking_RR()==0||IRtracking_R()==0)){
			err=1;
			sp=0;
			K=40;
			LED_show_num(2);
			last_line_turn=left;
		}
		else if((IRtracking_RR()==1)&&(IRtracking_LL()==0||IRtracking_L()==0)){
			err=-1;
			sp=0;
			K=100;
			LED_show_num(3);
			last_line_turn=right;
		}
		else if((IRtracking_R()==1)&&(IRtracking_LL()==0||IRtracking_L()==0)){
			err=-1;
			sp=0;
			K=40;
			LED_show_num(4);
			last_line_turn=right;
		}
		else if(IRtracking_R()==0&&IRtracking_LL()==0&&IRtracking_L()==0&&IRtracking_RR()==0&&IRtracking_M()==1){
			if(last_line_turn==right){err=-1;K=20;}
			else if(last_line_turn==left){err=1;K=20;}
			else err=0;
			sp=100;
			LED_show_num(5);
		}
		else{
			err=0;
			sp=50;
			LED_show_num(6);
		}	
		Motor_SetSpeedLeft(sp-adjust);
		Motor_SetSpeedRight(sp+adjust);
//		vTaskDelay(50);
	}	
	
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	//初始化
	sound_Init();
	Serial_Init();
	Sound_Timer_Init();
	Delay_Timer_Init();
	IRtracking_Init();
	Motor_Init();
	KEY_Init();
	LED_Init();
	
	//队列
	Distance_SendQueue=xQueueCreate( 3,sizeof(uint16_t));
	
	//信号量
	FindLine_Semaphore=xSemaphoreCreateBinary();
	FindLine_getline_Semaphore=xSemaphoreCreateBinary();
	
	//任务
	//xTaskCreate(Sound_distance,"Sound_distance",Sound_distance_STACK_SIZE,NULL,Sound_distance_PRIORITY,&Sound_distance_handle);
//	xTaskCreate(SendMsg,"SendMsg",SendMsg_STACK_SIZE,NULL,SendMsg_PRIORITY,&SendMsg_handle);
//	xTaskCreate(FindLine,"FindLine",FindLine_STACK_SIZE,NULL,FindLine_PRIORITY,&FindLine_handle);
	xTaskCreate(FollowLine,"FollowLine",FollowLine_STACK_SIZE,NULL,FollowLine_PRIORITY,&FollowLine_handle);
	
	vTaskStartScheduler();

	while(1)
	{
	}
}

