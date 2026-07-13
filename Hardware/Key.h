#ifndef __KEY_H
#define __KEY_H

/**
 * @brief  按键输入模块头文件 - 非阻塞式消抖检测
 * @note   引脚分配:
 *         K1 (PB12) = 模式切换键: STOP → AUTO → MANUAL → STOP ...
 *         K2 (PB13) = 急停键:     任何模式下立即停车
 *
 *         使用方式:
 *           1. 调用 Key_Init() 初始化硬件
 *           2. 在主循环中调用 Key_Scan() (建议每10ms~20ms一次)
 *           3. 用 Key_GetPressed() 检测是否有按键按下事件(单次触发)
 */

void Key_Init(void);
void Key_Scan(void);              /* 主循环中周期调用, 非阻塞 */
uint8_t Key_GetPressed(void);      /* 返回按下的键号(1=K1, 2=K2, 0=无) */

/* 键值宏定义 */
#define KEY_NONE    0
#define KEY_MODE    1              /* K1 模式切换 */
#define KEY_ESTOP   2              /* K2 急停 */

#endif
