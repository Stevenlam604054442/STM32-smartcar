//#include "stm32f10x.h"
//#include "Delay.h"
////红外遥控使用的GPIO及时钟
//#define IR_RCC_APB                   RCC_APB2Periph_GPIOA
//#define IR_PORT  		 			 GPIOA
//#define IR_PIN   		 			 GPIO_Pin_7
//unsigned char IR_DataFlag;
//unsigned char IR_RepeatFlag;
//unsigned char IR_Address;
//unsigned char IR_Command;
//unsigned char IR_State;
//int IR_Time;
//unsigned char IR_Data[4];
//unsigned char IR_pData;

//void IR_Configuration(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	TIM_ICInitTypeDef  TIM_ICInitStructure;  

//	RCC_APB2PeriphClockCmd(IR_RCC_APB, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	//TIM3 时钟使能
//	
//	GPIO_InitStructure.GPIO_Pin = IR_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(IR_PORT, &GPIO_InitStructure);
//	GPIO_SetBits(IR_PORT, IR_PIN);

//	TIM_DeInit(TIM3);
//	TIM_TimeBaseStructure.TIM_Period = 20000; //设定计数器自动重装值 最大20ms溢出  
//	TIM_TimeBaseStructure.TIM_Prescaler =(72-1); 	//预分频器,1M的计数频率,1us加1.	   
//	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx

//    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;  // 选择输入端 IC2映射到TI3上
//    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;	//下降沿捕获
//    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
//    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
//    TIM_ICInitStructure.TIM_ICFilter = 0x03;//IC2F=0011 配置输入滤波器 8个定时器时钟周期滤波
//    TIM_ICInit(TIM3, &TIM_ICInitStructure);//初始化定时器输入捕获通道

//	TIM_ITConfig( TIM3,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC2IE捕获中断	
//    TIM_Cmd(TIM3,ENABLE ); 	//使能定时器3

//	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //抢占优先级1级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级2级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
//	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器	
//}
///**
//  * @brief  红外遥控获取收到数据帧标志位
//  * @param  无
//  * @retval 是否收到数据帧，1为收到，0为未收到
//  */
//unsigned char IR_GetDataFlag(void)
//{
//	if(IR_DataFlag)
//	{
//		IR_DataFlag=0;
//		return 1;
//	}
//	return 0;
//}
///**
//  * @brief  红外遥控获取收到连发帧标志位
//  * @param  无
//  * @retval 是否收到连发帧，1为收到，0为未收到
//  */
//unsigned char IR_GetRepeatFlag(void)
//{
//	if(IR_RepeatFlag)
//	{
//		IR_RepeatFlag=0;
//		return 1;
//	}
//	return 0;
//}
///**
//  * @brief  红外遥控获取收到的地址数据
//  * @param  无
//  * @retval 收到的地址数据
//  */
//unsigned char IR_GetAddress(void)
//{
//	return IR_Address;
//}

///**
//  * @brief  红外遥控获取收到的命令数据
//  * @param  无
//  * @retval 收到的命令数据
//  */
//unsigned char IR_GetCommand(void)
//{
//	return IR_Command;
//}
///**
//  * @brief  红外扫描函数
//  * @param  无
//  * @retval 无
//  */
////void IR_Scan(void)
////{
////	unsigned char Command;
////	if(IR_GetDataFlag() )  //||IR_GetRepeatFlag()
////	{
////		Command=IR_GetCommand();
////		printf("Command=%x\r\n",Command);
////		switch(Command)
////		{
////			case IR_CH_MINUS:
////					break;			   
////			case IR_CH_ADD:
////					break;	    
////			case IR_MODE:
////					break;	    
////		}
////	}
////}
//void TIM3_IRQHandler(void)
//{
//	//下降沿捕获触发中断
//	if(TIM_GetITStatus(TIM3,TIM_FLAG_CC2)!=RESET) 
//	{

//		if(IR_State==0)           //状态0，空闲状态
//		{
//			TIM_SetCounter(TIM3,0);     //定时计数器清0
//			IR_State=1;	            //置状态为1
//		}
//		else if(IR_State==1)      //状态1，等待Start信号或Repeat信号
//		{
//			IR_Time=TIM_GetCapture2(TIM3);   //获取上一次中断到此次中断的时间
//			TIM_SetCounter(TIM3,0);
//			if(IR_Time>13500-500 && IR_Time<13500+500)       //如果计时为13.5ms，则接收到了Start信号
//			{
//				IR_State=2;			      //置状态为2
//			}
//			else if(IR_Time>11250-300 && IR_Time<11250+300)  //如果计时为11.25ms，则接收到了Repeat信号
//			{
//				IR_RepeatFlag=1;	    //置收到连发帧标志位为1
//				TIM_SetCounter(TIM3,0);		//定时器计数清0
//				IR_State=0;			      //置状态为0
//			}
//			else               //接收出错
//			{
//				IR_State=1;
//			}
//		}
//		else if(IR_State==2)      //状态2，接收数据
//		{
//			IR_Time=TIM_GetCapture2(TIM3);
//			TIM_SetCounter(TIM3,0);
//			if(IR_Time>1120-500 && IR_Time<1120+500)      //如果计时为1120us，则接收到了数据0
//			{
//				IR_Data[IR_pData/8]&=~(0x01<<(IR_pData%8)); //写0
//				IR_pData++;	     //数据位置指针自增
//			}
//			else if(IR_Time>2250-500 && IR_Time<2250+500) //如果计时为2250us，则接收到了数据1
//			{
//				IR_Data[IR_pData/8]|=(0x01<<(IR_pData%8));  //写1
//				IR_pData++;	
//			}
//			else
//			{
//				IR_pData=0;      //数据位置指针清0
//				IR_State=1;      //置状态为1
//			}
//			if(IR_pData>=32)	 //如果接收到了32位数据
//			{
//				IR_pData=0;	
//				if((IR_Data[0]==(uint8_t)~IR_Data[1]) && (IR_Data[2]==(uint8_t)~IR_Data[3]))	 //数据验证
//				{
//					IR_Address=IR_Data[0];	//转存数据
//					IR_Command=IR_Data[2];
//					if(IR_Address==0)IR_DataFlag=1;	//置收到连发帧标志位为1
//					else IR_DataFlag=0;
////					printf("IR_Address=%d\r\n",IR_Address);       //注意华为盒子遥控器IR_Address是0x22
//				}
//				TIM_SetCounter(TIM3,0);
//				IR_State=0;
//			}
//		}
//	}
//	TIM_ClearITPendingBit(TIM3,TIM_IT_Update|TIM_IT_CC2);
//}