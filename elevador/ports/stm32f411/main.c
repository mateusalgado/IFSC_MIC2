/* Clock parameters derived from labRTOS/ex_Barry_USB_STM32F411CE.
 * Target premise: STM32F411CEU6, HSE 25 MHz; physical board still unconfirmed.
 */
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "elevator_rtos.h"
#include "board_port.h"
static void clock_init(void) {
    RCC_OscInitTypeDef osc = {0}; RCC_ClkInitTypeDef clk = {0};
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON; osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = 25; osc.PLL.PLLN = 192; osc.PLL.PLLP = RCC_PLLP_DIV2; osc.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1; clk.APB1CLKDivider = RCC_HCLK_DIV2; clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_3) != HAL_OK) Error_Handler();
}
int main(void) {
    HAL_Init();
    board_safe_init();
    clock_init();
    ElevatorRTOS_Start();
    vTaskStartScheduler();
    Error_Handler();
    return 0;
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM4) HAL_IncTick();
}
void Error_Handler(void) {
    board_fault_disable();
    __disable_irq();
    for (;;) { }
}
