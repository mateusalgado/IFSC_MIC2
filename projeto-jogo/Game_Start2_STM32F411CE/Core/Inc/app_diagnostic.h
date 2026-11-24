#ifndef APP_DIAGNOSTIC_H
#define APP_DIAGNOSTIC_H
#include <stdint.h>

/* Usado apenas no build APP_DIAGNOSTIC=1. O jogo normal usa App_Start(). */
void Diagnostic_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp);
void Diagnostic_Init(void);
/* Executa na tarefa inicial, depois de iniciar o ADC/DMA; nao retorna. */
void Diagnostic_Run(void);
#endif
