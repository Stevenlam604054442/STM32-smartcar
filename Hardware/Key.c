/**
  ******************************************************************************
  * @file    Key.c
  * @brief   按键输入驱动 - PB12/PB13 非阻塞式消抖检测
  * @note
  *     硬件: K1=PB12, K2=PB13, 内部上拉(IPU), 低电平有效(按下接地)
  *
  *     消抖状态机 (每个按键独立):
  *       IDLE → DETECTED(首次检测到低) → CONFIRMED(连续低电平>20ms) → RELEASED(松手)
  *       只有 CONFIRMED 状态触发一次按键事件, RELEASED 时清除标志
  *
  *     使用方法: 在主循环中每周期调用 Key_Scan(), 用 Key_GetPressed() 获取事件
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "Key.h"
#include "Delay.h"

/* ======================== 硬件层 ======================== */

/**
  * @brief  按键GPIO初始化
  * @note   PB12/PB13 配置为内部上拉输入, 默认高电平
  */
void Key_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;              /* 上拉输入 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13;  /* PB12 + PB13 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/* ======================== 软件消抖状态机 ======================== */

/* 单个按键的状态定义 */
typedef enum
{
    KEY_STATE_IDLE = 0,        /* 空闲: 未按下或已处理完毕 */
    KEY_STATE_DETECTED,         /* 首次检测到低电平(开始计时) */
    KEY_STATE_CONFIRMED,        /* 已确认按下(等待释放) */
    KEY_STATE_RELEASED          /* 已释放(事件已被读取) */
} KeyState_t;

/* 消抖参数 */
#define KEY_DEBOUNCE_MS    20   /* 消抖时间阈值(ms) */
#define KEY_SCAN_INTERVAL   10  /* 建议调用间隔(ms) - 与主循环Delay_ms匹配 */

/* 两个按键的独立状态 */
static KeyState_t s_KeyState[2] = { KEY_STATE_IDLE, KEY_STATE_IDLE };
static uint16_t   s_DebounceCnt[2] = { 0, 0 };      /* 消抖计数器 */
static uint8_t    s_PressedFlag[2] = { 0, 0 };        /* 待读取的按下事件标志 */

/**
  * @brief  读取单个引脚的电平 (1=未按/高电平, 0=已按/低电平)
  */
static uint8_t Key_ReadPin(uint8_t keyIndex)
{
    if (keyIndex == 0)  /* K1 = PB12 */
        return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
    else                /* K2 = PB13 */
        return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
}

/**
  * @brief  非阻塞消抖扫描 - 主循环中周期调用
  * @note   每次调用对两个按键执行一步状态转换
  *         调用频率建议: 每10ms~20ms一次 (与主循环 Delay_ms 匹配)
  */
void Key_Scan(void)
{
    uint8_t i;
    for (i = 0; i < 2; i++)
    {
        uint8_t pinLevel = Key_ReadPin(i);  /* 1=高(未按), 0=低(按下) */

        switch (s_KeyState[i])
        {
        case KEY_STATE_IDLE:
            /* 空闲态: 检测到第一次低电平 → 进入消抖确认阶段 */
            if (pinLevel == 0)
            {
                s_KeyState[i]   = KEY_STATE_DETECTED;
                s_DebounceCnt[i] = 0;
            }
            break;

        case KEY_STATE_DETECTED:
            /* 检测态: 持续低电平则累加计数, 高电平则回退为干扰 */
            if (pinLevel == 0)
            {
                s_DebounceCnt[i]++;
                if (s_DebounceCnt[i] >= KEY_DEBOUNCE_MS / KEY_SCAN_INTERVAL)
                {
                    /* 连续低电平超过消抖阈值 → 确认有效按下 */
                    s_KeyState[i]    = KEY_STATE_CONFIRMED;
                    s_PressedFlag[i] = 1;           /* 设置待读事件 */
                }
            }
            else
            {
                /* 在消抖期内变高了 → 判定为机械抖动, 回到空闲 */
                s_KeyState[i] = KEY_STATE_IDLE;
            }
            break;

        case KEY_STATE_CONFIRMED:
            /* 确认态: 保持直到用户松手 */
            if (pinLevel == 1)
            {
                s_KeyState[i] = KEY_STATE_RELEASED;
            }
            break;

        case KEY_STATE_RELEASED:
            /* 释放态: 等待下一次 Scan 清除, 防止重复触发 */
            s_KeyState[i] = KEY_STATE_IDLE;
            break;

        default:
            s_KeyState[i] = KEY_STATE_IDLE;
            break;
        }
    }
}

/**
  * @brief  获取按键按下事件 (单次触发, 自动清除)
  * @return 按下的键号: 1=K1模式切换, 2=K2急停, 0=无按键
  * @note   每次物理按下只返回一次, 必须再次物理按下才返回下次
  */
uint8_t Key_GetPressed(void)
{
    uint8_t i;
    for (i = 0; i < 2; i++)
    {
        if (s_PressedFlag[i])
        {
            s_PressedFlag[i] = 0;    /* 清除事件标志(消费掉) */
            return i + 1;             /* 返回键号: 1 或 2 */
        }
    }
    return KEY_NONE;
}
