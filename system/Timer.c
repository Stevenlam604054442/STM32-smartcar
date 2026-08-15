#include "stm32f10x.h"

/**
  * @brief  TIM3 初始化 - 用于超声波测距的Echo信号高电平计时
  */
void Sound_Timer_Init(void)
{
	
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);     // 使能TIM3时钟
    
	/* 配置时钟源为内部时钟 */
    TIM_InternalClockConfig(TIM3);
    
	/* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;      // 不分频（用于滤波器时钟）
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 7199;                      // ARR: 计数周期 0~7199
    /*
     * 计数器溢出频率:
     *   CK_CNT_OV = CK_PSC / (PSC + 1) / (ARR + 1)
     *             = 72MHz / (0 + 1) / (7199 + 1)
     *             = 10kHz (每 0.1ms 溢出一次)
     */
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;                     // PSC: 不预分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器（仅高级定时器用）
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    /* 清除更新标志位（防止开启中断后立刻进入一次中断） */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);

    /* 使能更新中断 */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);


    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                 // TIM3全局中断通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                  // 使能该通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;        // 抢占优先级 = 6
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;               // 响应优先级 = 1
    NVIC_Init(&NVIC_InitStructure);

    /* 使能TIM3定时器 */
    TIM_Cmd(TIM3, ENABLE);
}


void Delay_Timer_Init(void)
{
	
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);     // 使能TIM2时钟
    
	/* 配置时钟源为内部时钟 */
    TIM_InternalClockConfig(TIM2);
    
	/* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;      // 不分频（用于滤波器时钟）
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 0xffff;                   
 
    TIM_TimeBaseInitStructure.TIM_Prescaler = 72-1;                     
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器（仅高级定时器用）
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    /* 使能TIM2定时器 */
    TIM_Cmd(TIM2, ENABLE);
}

void PWM_Timer_Init(void)
{
	
    /* 开启时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);     // 使能TIM4时钟
    
	/* 配置时钟源为内部时钟 */
    TIM_InternalClockConfig(TIM4);
    
	/* 时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;      // 不分频（用于滤波器时钟）
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 100-1;                   
 
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200-1;                     
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            // 重复计数器（仅高级定时器用）
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	
	/*输出比较初始化*/ 
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);                         //结构体初始化，若结构体没有完整赋值
	                                                                //则最好执行此函数，给结构体所有成员都赋一个默认值
	                                                                //避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //输出比较模式，选择PWM模式1，CNT < CCR：输出 有效电平，CNT >= CCR：输出 无效电平
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //输出极性，选择为高，若选择极性为低，则输出高低电平取反
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;								//初始的CCR值
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);                        //将结构体变量交给TIM_OC3Init，配置TIM2的输出比较通道3
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	
	//使能4个通道的预装载寄存器
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);//OC1
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);//OC2
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);//OC3
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);//OC4
    TIM_ARRPreloadConfig(TIM4, ENABLE); //使能重装寄存器
	  
    /* 使能TIM4定时器 */
    TIM_Cmd(TIM4, ENABLE);
}


