/**
  ******************************************************************************
  * @file    IR.h
  * @brief   红外遥控模块头文件 - NEC协议解码
  * @note    使用TIM4输入捕获进行NEC红外信号解码
  ******************************************************************************
  */

#ifndef __IR_H
#define __IR_H

#include <stdint.h>

/* 状态码定义 */
#define IR_OK          1     /* 成功收到数据帧 */
#define IR_REPEAT      2     /* 收到连发帧(按键长按) */
#define IR_ERROR       0     /* 无数据或接收错误 */

void IR_Init(void);
uint8_t  IR_GetDataFlag(void);       /* 获取数据帧标志 (读取后自动清除) */
uint8_t  IR_GetRepeatFlag(void);     /* 获取连发帧标志 (读取后自动清除) */
uint8_t  IR_GetAddress(void);        /* 获取地址码 */
uint8_t  IR_GetCommand(void);        /* 获取命令码(按键码) */
void IR_Scan(void);                  /* 扫描函数(在主循环中调用) */

#endif
