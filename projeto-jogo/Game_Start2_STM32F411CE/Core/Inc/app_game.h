#ifndef APP_GAME_H
#define APP_GAME_H
#include <stdint.h>
/* Chamar uma vez, da tarefa inicial, antes de iniciar o ADC/DMA. */
void App_Start(void);
/* Callback de interrupcao: publica X/Y sem bloquear nem acessar o display. */
void App_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp);
#endif
