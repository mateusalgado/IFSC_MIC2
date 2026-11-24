#ifndef MAIN_TEST_H
#define MAIN_TEST_H
#include <stdint.h>
#define GPIOC 2
#define GPIO_PIN_15 15
#define GPIO_PIN_RESET 0
#define LED_GPIO_Port 2
#define LED_Pin 13
uint32_t HAL_GetTick(void);
int HAL_GPIO_ReadPin(int port, int pin);
void HAL_GPIO_TogglePin(int port, int pin);
void Error_Handler(void);
#endif
