#ifndef __LED_H
#define __LED_H

/**
 * @brief  LED指示灯模块头文件
 * @note   双色状态指示:
 *         LED1 (PB4) - 运行指示灯: 电机转动时亮, 停车时灭
 *         LED2 (PB5) - 状态指示灯:  避障告警闪烁 / 模式指示
 *         低电平点亮(共阳极接法)
 */

void LED_Init(void);

/* LED1 - 运行指示 */
void LED1_ON(void);
void LED1_OFF(void);
void LED1_Turn(void);

/* LED2 - 状态/告警指示 */
void LED2_ON(void);
void LED2_OFF(void);
void LED2_Turn(void);

#endif
