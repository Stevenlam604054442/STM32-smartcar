#ifndef __HARDWARE_H
#define __HARDWARE_H
#include "stm32f10x.h"


/* ===== SOUND ===== */
#define SOUND_Trig        GPIO_Pin_15
#define SOUND_Echo        GPIO_Pin_14
#define SOUND_PORT       GPIOB
#define SOUND_CLK        RCC_APB2Periph_GPIOB
#define SOUND_EXIT_PORT        GPIO_PortSourceGPIOB
#define SOUND_EXIT_Echo        GPIO_PinSource14
#define SOUND_EXIT_Line        EXTI_Line14
#define SOUND_EXIT_Line_IRQ        EXTI15_10_IRQn

/* ===== TEST_UART ===== */
#define TEST_UART_TX        GPIO_Pin_9
#define TEST_UART_RX        GPIO_Pin_10
#define TEST_UART_PORT       GPIOA
#define TEST_UART_USART       USART1
#define TEST_UART_USART_CLK        RCC_APB2Periph_USART1
#define TEST_UART_PORT_CLK        RCC_APB2Periph_GPIOA
#define TEST_UART_IRQ        USART1_IRQn

/* ===== IRTRACKING ===== */
#define IRTRACKING_Lo        GPIO_Pin_13
#define IRTRACKING_Ro        GPIO_Pin_12
#define IRTRACKING_PORT       GPIOB
#define IRTRACKING_CLK        RCC_APB2Periph_GPIOB

/* ===== MOTOR ===== */
#define MOTOR_In4        GPIO_Pin_9
#define MOTOR_In3        GPIO_Pin_8
#define MOTOR_In2        GPIO_Pin_7
#define MOTOR_In1        GPIO_Pin_6
#define MOTOR_PORT       GPIOB
#define MOTOR_CLK        RCC_APB2Periph_GPIOB

/* ===== KEY ===== */
#define KEY_PIN        GPIO_Pin_15
#define KEY_PORT       GPIOA
#define KEY_CLK        RCC_APB2Periph_GPIOA

/* ===== LED ===== */
#define LED_A        GPIO_Pin_0
#define LED_B        GPIO_Pin_1
#define LED_C        GPIO_Pin_2
#define LED_D        GPIO_Pin_3
#define LED_E        GPIO_Pin_4
#define LED_F        GPIO_Pin_5
#define LED_G        GPIO_Pin_6
#define LED_DP        GPIO_Pin_7
#define LED_PORT       GPIOA
#define LED_CLK        RCC_APB2Periph_GPIOA

#endif
