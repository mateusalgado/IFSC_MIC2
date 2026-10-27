/* Exercise the real driver against a bounded fake SPI panel. */
#ifndef DRIVER_TEST_WRAPPER_H
#define DRIVER_TEST_WRAPPER_H
#define __MAIN_H
#include <stddef.h>
#include <stdint.h>
typedef struct { int unused; } SPI_HandleTypeDef;
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1
#define CS_Pin 12
#define RST_Pin 13
#define DC_Pin 14
#define CS_GPIO_Port 0
#define RST_GPIO_Port 0
#define DC_GPIO_Port 0
void HAL_GPIO_WritePin(int port,int pin,int value);
void HAL_Delay(uint32_t ms);
HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *spi,uint8_t *data,uint16_t size,uint32_t timeout);
#include "../../Game_Start2_STM32F411CE/Core/Inc/st7735.h"
#endif
