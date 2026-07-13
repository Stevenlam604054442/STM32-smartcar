#ifndef __KEY_H
#define __KEY_H

/**
  * @brief  按键输入模块头文件
  * @note   PA1/PA2 浮空输入, 返回键码 1或2 (0=无按键)
  */

void Key_Init(void);
uint8_t Key_GetRL(void);

#endif
