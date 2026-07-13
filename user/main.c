/**
  ******************************************************************************
  * @file    main.c
  * @brief   STM32智能小车主程序 - 集成演示：超声波避障+OLED显示+串口调试+红外遥控
  * @note    硬件平台: STM32F103C8T6 @ 72MHz, Keil MDK-ARM v5
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Motor.h"
#include "sound.h"
#include "Serial.h"
#include "IR.h"

/* ======================== 全局变量 ======================== */
uint16_t Distance = 0;          // 超声波测距值 (单位: mm)
int8_t    MotorSpeed = 0;       // 电机速度 (-100 ~ +100)
uint8_t   Mode = 0;             // 运行模式: 0=停止 1=自动避障 2=手动控制(串口/红外)
uint8_t   RxData;               // 串口接收数据缓存
uint8_t   IR_Cmd;               // 红外遥控按键码缓存

/* ======================== 函数声明 ======================== */
void System_Init(void);         // 系统全模块初始化
void OLED_Display(void);        // OLED 状态面板刷新
void Auto_Avoidance(void);      // 自动避障逻辑
void Serial_Control(void);      // 串口指令解析与执行
void IR_Control(void);          // 红外遥控按键处理

/* ======================== 主函数 ======================== */
int main(void)
{
    System_Init();              // 全模块初始化

    OLED_ShowString(1, 1, "System Ready!");
    Delay_ms(1000);
    OLED_Clear();

    /* 主循环 */
    while (1)
    {
        /* 1. 超声波测距 (每100ms采集一次) */
        Distance = sound_GetValue();

        /* 2. OLED 刷新状态面板 */
        OLED_Display();

        /* 3. 串口数据接收与处理 */
        if (Serial_GetRxFlag() == 1)
        {
            RxData = Serial_GetRxData();
            Serial_SendByte(RxData);
            Serial_Control();
        }

        /* 4. 红外遥控扫描与处理 */
        IR_Scan();              /* 更新IR标志位 */
        if (IR_GetDataFlag() == IR_OK || IR_GetRepeatFlag() == IR_REPEAT)
        {
            IR_Cmd = IR_GetCommand();
            OLED_ShowHexNum(4, 9, IR_Cmd, 2);     /* OLED第4行显示红外键码 */
            IR_Control();
        }

        /* 5. 根据模式执行对应逻辑 */
        switch (Mode)
        {
        case 0:                         // 停止模式
            Motor_SetSpeedRL(0);
            Motor_SetSpeedUD(0);
            break;
        case 1:                         // 自动避障模式
            Auto_Avoidance();
            break;
        case 2:                         // 手动控制模式（串口/红外通用）
            break;
        default:
            Mode = 0;
            break;
        }

        Delay_ms(100);
    }
}

/* ======================== 函数实现 ======================== */

/**
  * @brief  系统全模块初始化
  * @note   按依赖顺序初始化各外设模块
  *         定时器资源分配: TIM2=PWM电机 / TIM3=超声波计时 / TIM4=红外解码
  */
void System_Init(void)
{
    OLED_Init();           // OLED显示屏 (I2C软件模拟, PB8/PB9)
    sound_Init();          // 超声波模块 (HC-SR04, PB0/PA7, TIM3中断计时)
    Motor_Init();          // 直流电机 (TIM2 PWM输出 + GPIO方向控制)
    Serial_Init();         // 串口通信 (USART3, PB10/PB11, 9600bps)
    Key_Init();            // 按键输入 (PA1/PA2 浮空输入)
    IR_Init();             // 红外遥控 (NEC协议, PA0上拉输入, TIM4输入捕获)

    Serial_Printf("STM32 Smart Car System Init OK\r\n");
}

/**
  * @brief  OLED状态面板 - 实时显示系统运行数据
  * @note   布局:
  *         Line1: 距离值 (mm)          Line2: 电机速度 / 方向
  *         Line3: 当前运行模式         Line4: 红外按键码(HEX) 或系统状态
  */
void OLED_Display(void)
{
    OLED_ShowString(1,  1, "Dist:");
    OLED_ShowNum(1, 6, Distance, 5);
    OLED_ShowString(1, 12, "mm");

    OLED_ShowString(2,  1, "Spd RL:");
    OLED_ShowSignedNum(2, 9, MotorSpeed, 4);

    OLED_ShowString(3,  1, "Mode:");
    if (Mode == 0)      OLED_ShowString(3, 6, "STOP ");
    else if (Mode == 1) OLED_ShowString(3, 6, "AUTO ");
    else                OLED_ShowString(3, 6, "MANUAL");

    OLED_ShowString(4,  1, "IR:0x");
    OLED_ShowHexNum(4, 5, IR_Cmd, 2);           /* 显示最近一次红外按键码 */
}

/**
  * @brief  自动避障逻辑 - 基于超声波距离阈值控制电机
  * @note   三段式距离策略:
  *         >200mm 全速前进 | 50~200mm 减速通过 | <50mm 后退避让
  */
void Auto_Avoidance(void)
{
    if (Distance > 200)
    {
        MotorSpeed = 50;
        Motor_SetSpeedRL(MotorSpeed);
        Motor_SetSpeedUD(0);
    }
    else if (Distance > 50)
    {
        MotorSpeed = 25;
        Motor_SetSpeedRL(MotorSpeed);
        Motor_SetSpeedUD(0);
    }
    else
    {
        MotorSpeed = -40;
        Motor_SetSpeedRL(MotorSpeed);
        Motor_SetSpeedUD(0);
        Delay_ms(500);
    }
}

/**
  * @brief  串口指令解析与执行
  * @note   指令集:
  *         '0' 停止 / '1' 自动避障 / '2' 手动控制
  *         'w'前 's'后 'a'左 'd'右 ' '急停
  */
void Serial_Control(void)
{
    switch (RxData)
    {
    case '0':
        Mode = 0; MotorSpeed = 0;
        Serial_Printf("Mode: STOP\r\n");
        break;
    case '1':
        Mode = 1;
        Serial_Printf("Mode: AUTO Avoidance\r\n");
        break;
    case '2':
        Mode = 2;
        Serial_Printf("Mode: MANUAL Control\r\n");
        break;
    case 'w': case 'W':
        if (Mode == 2) { MotorSpeed = 60; Motor_SetSpeedRL(MotorSpeed); Motor_SetSpeedUD(0); }
        Serial_Printf("CMD: Forward\r\n");
        break;
    case 's': case 'S':
        if (Mode == 2) { MotorSpeed = -60; Motor_SetSpeedRL(MotorSpeed); Motor_SetSpeedUD(0); }
        Serial_Printf("CMD: Backward\r\n");
        break;
    case 'a': case 'A':
        if (Mode == 2) { Motor_SetSpeedRL(40); Motor_SetSpeedUD(40); }   /* 差速转向 */
        Serial_Printf("CMD: TurnLeft\r\n");
        break;
    case 'd': case 'D':
        if (Mode == 2) { Motor_SetSpeedRL(-40); Motor_SetSpeedUD(-40); }
        Serial_Printf("CMD: TurnRight\r\n");
        break;
    case ' ':
        MotorSpeed = 0; Motor_SetSpeedRL(0); Motor_SetSpeedUD(0);
        Serial_Printf("CMD: E-STOP!\r\n");
        break;
    default:
        Serial_Printf("ERR: Unknown cmd '%c'(0x%02X)\r\n", RxData, RxData);
        break;
    }
}

/**
  * @brief  红外遥控按键处理
  * @note   按键映射 (常见NEC遥控器):
  *         0x45 CH-  / 0x46 CH   / 0x47 CH+
  *         0x44 <<   / 0x40 >>   / 0x43 >||
  *         0x07 VOL- / 0x15 VOL+ / 0x09 EQ
  *         0x16 0    / 0x19 200+ / 0x0D 100+
  *         0x0C 200- / 0x18 1    / 0x5E 2
  *         0x08 3    / 0x1C 4    / 0x5A 5
  *         0x42 6    / 0x52 7    / 0x4A 8
  *         0x52 9
  */
void IR_Control(void)
{
    switch (IR_Cmd)
    {
    /* 模式切换 */
    case 0x16:  /* 按键"0" -> 停止 */
        Mode = 0; MotorSpeed = 0;
        Serial_Printf("[IR] STOP\r\n");
        break;

    /* 方向控制（需在手动模式下生效） */
    case 0x18:  /* 按键"1" -> 前进 */
        if (Mode != 2) Mode = 2;
        MotorSpeed = 60; Motor_SetSpeedRL(MotorSpeed); Motor_SetSpeedUD(0);
        Serial_Printf("[IR] Forward\r\n");
        break;
    case 0x5E:  /* 按键"2" -> 后退 */
        if (Mode != 2) Mode = 2;
        MotorSpeed = -60; Motor_SetSpeedRL(MotorSpeed); Motor_SetSpeedUD(0);
        Serial_Printf("[IR] Backward\r\n");
        break;
    case 0x08:  /* 按键"3" -> 左转 */
        if (Mode != 2) Mode = 2;
        Motor_SetSpeedRL(40); Motor_SetSpeedUD(40);
        Serial_Printf("[IR] TurnLeft\r\n");
        break;
    case 0x1C:  /* 按键"4" -> 右转 */
        if (Mode != 2) Mode = 2;
        Motor_SetSpeedRL(-40); Motor_SetSpeedUD(-40);
        Serial_Printf("[IR] TurnRight\r\n");
        break;
    case 0x15:  /* VOL+ -> 急停 */
        MotorSpeed = 0; Motor_SetSpeedRL(0); Motor_SetSpeedUD(0);
        Serial_Printf("[IR] E-STOP!\r\n");
        break;

    default:
        /* 未定义的按键仅显示键码，不执行动作 */
        break;
    }
}
