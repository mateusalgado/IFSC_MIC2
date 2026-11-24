/* Tela opcional para conferir display, ADC e botao antes de testar o jogo. */
#include "app_diagnostic.h"
#include "game_config.h"
#if APP_DIAGNOSTIC
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "st7735.h"
#include <stdio.h>

/* A ISR publica; a tarefa copia os tres campos em uma secao critica curta. */
static volatile uint32_t adc_sample;
static volatile uint32_t adc_updated_ms;
static volatile uint8_t adc_ready;
static uint32_t led_ms;

void Diagnostic_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp)
{
    adc_sample = ((uint32_t)x << 16) | y;
    adc_updated_ms = timestamp;
    adc_ready = 1U;
}

void Diagnostic_Init(void)
{
    led_ms = HAL_GetTick();
    HAL_Delay(100);
    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_WriteString(2, 2, "MIC2 TESTE SPI1", Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_FillRectangle(2, 18, 50, 10, ST7735_RED);
    ST7735_FillRectangle(55, 18, 50, 10, ST7735_GREEN);
    ST7735_FillRectangle(108, 18, 50, 10, ST7735_BLUE);
    ST7735_FillRectangle(1, 89, 158, 38, ST7735_WHITE);
    ST7735_FillRectangle(2, 90, 156, 36, ST7735_BLACK);
}

void Diagnostic_Run(void)
{
    char line[24];
    uint16_t previous_x = 2U;
    uint16_t previous_y = 91U;
    TickType_t last_wake = xTaskGetTickCount();
    for (;;)
    {
        /* A single aligned load keeps X/Y from different callbacks out of the UI. */
        /* Copy sample and age together; an ISR cannot publish a future timestamp. */
        taskENTER_CRITICAL();
        uint32_t sample = adc_sample;
        uint32_t updated_ms = adc_updated_ms;
        uint8_t ready = adc_ready;
        uint32_t now = HAL_GetTick();
        taskEXIT_CRITICAL();
        uint16_t x = (uint16_t)(sample >> 16);
        uint16_t y = (uint16_t)sample;
        snprintf(line, sizeof(line), "X A1: %4u", (unsigned)x);
        ST7735_WriteString(2, 35, line, Font_7x10, ST7735_WHITE, ST7735_BLACK);
        snprintf(line, sizeof(line), "Y A0: %4u", (unsigned)y);
        ST7735_WriteString(2, 48, line, Font_7x10, ST7735_WHITE, ST7735_BLACK);
        ST7735_WriteString(2, 61,
                           HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET
                               ? "SW: PRESSIONADO"
                               : "SW: SOLTO     ",
                           Font_7x10, ST7735_WHITE, ST7735_BLACK);
        ST7735_WriteString(2, 74,
                           ready && (uint32_t)(now - updated_ms) < 250U ? "ADC: OK       "
                                                                        : "ADC: SEM DADOS",
                           Font_7x10, ST7735_WHITE, ST7735_BLACK);
        ST7735_FillRectangle(previous_x, previous_y, 4, 4, ST7735_BLACK);
        previous_x = 2U + ((uint32_t)x * 152U) / 4095U;
        previous_y = 91U + ((uint32_t)y * 30U) / 4095U;
        ST7735_FillRectangle(previous_x, previous_y, 4, 4, ST7735_GREEN);
        if ((uint32_t)(now - led_ms) >= 500U)
        {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            led_ms = now;
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(50));
    }
}
#endif
