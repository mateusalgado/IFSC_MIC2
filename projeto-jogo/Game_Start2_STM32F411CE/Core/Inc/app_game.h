#ifndef APP_GAME_H
#define APP_GAME_H
#include <stdint.h>
void App_Start(void);
void App_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp);
#endif
