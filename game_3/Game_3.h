#ifndef GAME_3_H
#define GAME_3_H

#include "Menu.h"
#include "stm32l4xx_hal.h"  // Required for GPIO types and HAL functions

/* Pin Defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn

#define A5_Pin GPIO_PIN_0
#define A5_GPIO_Port GPIOC

#define A4_Pin GPIO_PIN_1
#define A4_GPIO_Port GPIOC

#define BTN2_Pin GPIO_PIN_2
#define BTN2_GPIO_Port GPIOC
#define BTN2_EXTI_IRQn EXTI2_IRQn

#define BTN3_Pin GPIO_PIN_3
#define BTN3_GPIO_Port GPIOC
#define BTN3_EXTI_IRQn EXTI3_IRQn

#define A0_Pin GPIO_PIN_0
#define A0_GPIO_Port GPIOA

#define A1_Pin GPIO_PIN_1
#define A1_GPIO_Port GPIOA

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA

#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

#define A2_Pin GPIO_PIN_4
#define A2_GPIO_Port GPIOA

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

#define A3_Pin GPIO_PIN_0
#define A3_GPIO_Port GPIOB

#define BZZ_Pin GPIO_PIN_10
#define BZZ_GPIO_Port GPIOB

#define BTN4_Pin GPIO_PIN_8
#define BTN4_GPIO_Port GPIOA
#define BTN4_EXTI_IRQn EXTI9_5_IRQn

#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA

#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA

#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

#define PWM_Pin GPIO_PIN_6
#define PWM_GPIO_Port GPIOB

#define PWM_Pin_2 GPIO_PIN_8
#define PWM_GPIO_Port GPIOB

/* Function Prototypes ---------------------------------------------------*/
MenuState Game3_Run(void);

#endif // GAME_3_H