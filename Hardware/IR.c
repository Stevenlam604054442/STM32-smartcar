/**
  ******************************************************************************
  * @file    IR.c
  * @brief   红外遥控 NEC 协议解码驱动
  * @note
  *     - 接收引脚: PA0 (上拉输入) — 连接红外接收头输出端
  *     - 定时器: TIM4 输入捕获模式 (1us计数频率)
  *     - 协议: NEC红外遥控编码协议
  *
  *     NEC 协议时序规范:
  *       引导码: 9ms低 + 4.5ms高 (Start)
  *       数据位'0': 560us低 + 560us高
  *       数据位'1': 560us低 + 1680us高
  *       连发码: 9ms低 + 2.25ms高 (Repeat)
  *       帧格式: [Address(8bit)][~Address(8bit)][Command(8bit)][~Command(8bit)]
  *
  *     定时器资源分配:
  *       TIM2 → 电机PWM / TIM3 → 超声波计时 / TIM4 → 红外解码
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "IR.h"
#include "Delay.h"

/* ======================== 全局变量 ======================== */
uint8_t IR_DataFlag;           /* 收到数据帧标志 */
uint8_t IR_RepeatFlag;         /* 收到连发帧标志 */
uint8_t IR_Address;            /* 地址码 */
uint8_t IR_Command;            /* 命令码(按键码) */

/* 内部状态变量 (static，不对外暴露) */
static uint8_t  IR_State;      /* 状态机状态 */
static uint16_t IR_Time;       /* 捕获时间差值 */
static uint8_t  IR_Data[4];    /* 接收数据缓冲区 [Addr, ~Addr, Cmd, ~Cmd] */
static uint8_t  IR_pData;      /* 当前接收位索引 (0~31) */

/* ======================== 函数实现 ======================== */

/**
  * @brief  红外遥控硬件初始化
  * @note   配置PA0为上拉输入 + TIM4_CH1输入捕获
  */
void IR_Init(void)
{
    /* 开启时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* GPIO配置: PA0 = 红外接收头上拉输入 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;              /* 上拉输入（空闲时HIGH） */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_0);                           /* 确保初始状态 */

    /* TIM4 时基单元: 1MHz计数频率 (1us/tick) */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_DeInit(TIM4);
    TIM_TimeBaseStructure.TIM_Period = 20000 - 1;              /* ARR=19999, 最大20ms溢出 */
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;               /* PSC=71, 72MHz/72=1MHz */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* TIM4 CH1 输入捕获: 下降沿捕获 + 8周期滤波 */
    TIM_ICInitTypeDef TIM_ICInitStructure;
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; /* 下降沿触发 */
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; /* 直连TI1 */
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;        /* 不分频 */
    TIM_ICInitStructure.TIM_ICFilter = 0x03;                     /* 8个定时器周期滤波 */
    TIM_ICInit(TIM4, &TIM_ICInitStructure);

    /* 使能中断: 更新溢出 + 捕获事件 */
    TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC1, ENABLE);

    /* NVIC 配置 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 使能TIM4 */
    TIM_Cmd(TIM4, ENABLE);

    /* 初始化内部状态 */
    IR_DataFlag = 0;
    IR_RepeatFlag = 0;
    IR_Address = 0;
    IR_Command = 0;
    IR_State = 0;
    IR_pData = 0;
}

/**
  * @brief  获取收到数据帧的标志位
  * @retval 1=有新数据帧, 0=无数据
  * @note   读取后自动清除标志位
  */
uint8_t IR_GetDataFlag(void)
{
    if (IR_DataFlag)
    {
        IR_DataFlag = 0;
        return IR_OK;
    }
    return IR_ERROR;
}

/**
  * @brief  获取收到连发帧的标志位
  * @retval 1=有连发帧, 0=无连发
  * @note   长按遥控按键时持续产生连发帧
  */
uint8_t IR_GetRepeatFlag(void)
{
    if (IR_RepeatFlag)
    {
        IR_RepeatFlag = 0;
        return IR_REPEAT;
    }
    return IR_ERROR;
}

/**
  * @brief  获取收到的地址码
  * @retval 8位地址码
  */
uint8_t IR_GetAddress(void)
{
    return IR_Address;
}

/**
  * @brief  获取收到的命令码(按键码)
  * @retval 8位命令码
  */
uint8_t IR_GetCommand(void)
{
    return IR_Command;
}

/**
  * @brief  扫描函数 - 在主循环中调用，处理红外按键事件
  * @note   可根据需要扩展：将按键映射为具体动作
  */
void IR_Scan(void)
{
    uint8_t Command;

    if (IR_GetDataFlag())
    {
        Command = IR_GetCommand();
        /* 此处可扩展按键功能映射 */
        /*
        switch (Command)
        {
            case 0x45: break;  // CH-
            case 0x46: break;  // CH
            case 0x47: break;  // CH+
            case 0x44: break;  // |<<
            case 0x40: break;  // >>|
            case 0x43: break;  // >||
            case 0x07: break;  // VOL-
            case 0x15: break;  // VOL+
            case 0x09: break;  // EQ
            case 0x16: break;  // 0
            case 0x19: break;  // 200+
            case 0x0D: break;  // 100+
            case 0x0C: break;  // 200-
            case 0x18: break;  // 1
            case 0x5E: break;  // 2
            case 0x08: break;  // 3
            case 0x1C: break;  // 4
            case 0x5A: break;  // 5
            case 0x42: break;  // 6
            default: break;
        }
        */
    }

    if (IR_GetRepeatFlag())
    {
        /* 处理长按连发事件 */
    }
}

/**
  * @brief  TIM4 中断服务函数 - NEC协议状态机解码
  *
  *  状态机流程:
  *  ┌──────┐   Start(13.5ms)   ┌──────┐   Data(32bits)    ┌──────┐
  *  │State0├─────────────────→│State1├──────────────────→│State2│
  *  │ 空闲 │←─────────────────│等待  │←─────────────────│接收  │
  *  └──────┘  Repeat/Error    └──────┘   Error           └──────┘
  */
void TIM4_IRQHandler(void)
{
    /* ===== 捕获中断 (下降沿触发) ===== */
    if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
    {
        switch (IR_State)
        {
        case 0:  /* 空闲状态 - 等待第一个下降沿(引导码开始) */
            TIM_SetCounter(TIM4, 0);     /* 计数器清零 */
            IR_State = 1;                 /* 进入等待Start信号状态 */
            break;

        case 1:  /* 等待 Start 或 Repeat 信号 */
            IR_Time = TIM_GetCapture1(TIM4);  /* 读取从上次到这次的计数值 */
            TIM_SetCounter(TIM4, 0);          /* 计数器清零 */

            if (IR_Time > 13000 && IR_Time < 14000)  /* 13.5ms ±500us → Start信号 */
            {
                IR_State = 2;                          /* 进入数据接收状态 */
            }
            else if (IR_Time > 10750 && IR_Time < 11750)  /* 11.25ms ±500us → Repeat信号 */
            {
                IR_RepeatFlag = 1;                   /* 设置连发帧标志 */
                IR_State = 0;                         /* 回到空闲 */
            }
            else
            {
                IR_State = 1;                         /* 时序错误，重新等待 */
            }
            break;

        case 2:  /* 接收32位数据 */
            IR_Time = TIM_GetCapture1(TIM4);
            TIM_SetCounter(TIM4, 0);

            if (IR_Time > 620 && IR_Time < 1620)   /* 1120±500us → 数据位'0' */
            {
                IR_Data[IR_pData / 8] &= ~(0x01 << (IR_pData % 8));  /* 写0 */
                IR_pData++;
            }
            else if (IR_Time > 1750 && IR_Time < 2750)  /* 2250±500us → 数据位'1' */
            {
                IR_Data[IR_pData / 8] |= (0x01 << (IR_pData % 8));   /* 写1 */
                IR_pData++;
            }
            else
            {
                /* 时序错误，重置 */
                IR_pData = 0;
                IR_State = 1;
                break;
            }

            /* 32位数据全部接收完毕 */
            if (IR_pData >= 32)
            {
                IR_pData = 0;

                /* NEC校验: Addr==~Addr 且 Cmd==~Cmd */
                if ((IR_Data[0] == (uint8_t)~IR_Data[1]) &&
                    (IR_Data[2] == (uint8_t)~IR_Data[3]))
                {
                    IR_Address = IR_Data[0];
                    IR_Command = IR_Data[2];

                    /* 标准NEC地址码为0x00表示有效数据帧 */
                    if (IR_Address == 0x00 || IR_Address == 0xFF)
                    {
                        IR_DataFlag = 1;             /* 有效数据帧标志 */
                    }
                }

                IR_State = 0;  /* 回到空闲 */
            }
            break;

        default:
            IR_State = 0;
            break;
        }

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    }

    /* ===== 更新溢出中断 (20ms超时保护) ===== */
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
