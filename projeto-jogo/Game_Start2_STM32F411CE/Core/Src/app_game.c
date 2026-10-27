#include "app_game.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "game_config.h"
#include "maze_game.h"
#include "joy_input.h"
#include "game_renderer.h"
#include "st7735.h"

typedef struct { uint16_t x, y; uint32_t timestamp; } AdcSample;
typedef struct { int16_t x, y; bool valid, calibrated; uint32_t timestamp; } Input;
static QueueHandle_t adc_queue, input_queue, clicks, frames, rendered;
static volatile GameReason fault;

/* Inspect in debugger: stack margins are in StackType_t words, not bytes. */
volatile uint32_t game_input_stack_free, game_logic_stack_free, game_render_stack_free;
volatile uint32_t game_late_ticks, game_min_free_heap;

static void fail(GameReason reason)
{
    taskENTER_CRITICAL();
    if (fault == GAME_NO_ERROR) fault = reason;
    taskEXIT_CRITICAL();
}

void App_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp)
{
    AdcSample sample = {x, y, timestamp};
    BaseType_t woke = pdFALSE;
    if (adc_queue != NULL) {
        xQueueOverwriteFromISR(adc_queue, &sample, &woke);
        portYIELD_FROM_ISR(woke);
    }
}

static void InputTask(void *argument)
{
    (void)argument;
    Joy joy; Joy_Init(&joy);
    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        AdcSample sample = {0};
        bool available = xQueuePeek(adc_queue, &sample, 0) == pdPASS;
        uint32_t now = HAL_GetTick();
        bool valid = available && (uint32_t)(now - sample.timestamp) <= 100;
        JoyResult result = Joy_Update(&joy, sample.x, sample.y,
            HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET, valid);
        Input input = {result.x, result.y, valid, result.calibrated, now};
        xQueueOverwrite(input_queue, &input);
        if (result.click && xQueueSend(clicks, &now, 0) != pdPASS) fail(GAME_QUEUE_ERROR);
        game_input_stack_free = uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(GAME_STEP_MS));
    }
}

static void GameTask(void *argument)
{
    (void)argument;
    Game game; Game_Init(&game);
    TickType_t wake = xTaskGetTickCount();
    uint32_t last = HAL_GetTick(), led = last;
    for (;;) {
        Input input = {0}; uint32_t ack = 0, click_time = 0;
        xQueuePeek(input_queue, &input, 0);
        xQueuePeek(rendered, &ack, 0);
        bool click = false;
        while (xQueueReceive(clicks, &click_time, 0) == pdPASS) click = true;
        uint32_t now = HAL_GetTick(), dt = now - last; last = now;
        GameInput commands = {input.x, input.y, input.calibrated,
            input.valid && (uint32_t)(now-input.timestamp) <= 100, click, ack == game.scene};
        if (dt > GAME_STEP_MS+2U) ++game_late_ticks;
        if (fault != GAME_NO_ERROR) Game_Fail(&game, fault);
        if (game.state != GAME_CALIBRATING && !commands.render_ready && game.state_ms > 2000)
            Game_Fail(&game, GAME_DISPLAY_ERROR);
        if (game.state == GAME_CALIBRATING && !commands.valid && game.state_ms > 1000)
            Game_Fail(&game, GAME_ADC_ERROR);
        Game_Tick(&game, &commands, dt);
        xQueueOverwrite(frames, &game);
        if (now-led >= (game.state == GAME_FAULT ? 100U : 500U)) {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin); led = now;
        }
        game_logic_stack_free = uxTaskGetStackHighWaterMark(NULL);
        game_min_free_heap = xPortGetMinimumEverFreeHeapSize();
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(GAME_STEP_MS));
    }
}

static void RenderTask(void *argument)
{
    (void)argument;
    vTaskDelay(pdMS_TO_TICKS(100));
    ST7735_Init(); Renderer_Init();
    TickType_t wake = xTaskGetTickCount();
    for (;;) {
        Game frame;
        if (xQueuePeek(frames, &frame, 0) == pdPASS) {
            Renderer_Draw(&frame);
            if (ST7735_HasError()) fail(GAME_DISPLAY_ERROR);
            else xQueueOverwrite(rendered, &frame.scene);
        }
        game_render_stack_free = uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(GAME_RENDER_MS));
    }
}

void App_Start(void)
{
    adc_queue = xQueueCreate(1, sizeof(AdcSample));
    input_queue = xQueueCreate(1, sizeof(Input));
    clicks = xQueueCreate(8, sizeof(uint32_t));
    frames = xQueueCreate(1, sizeof(Game));
    rendered = xQueueCreate(1, sizeof(uint32_t));
    if (!adc_queue || !input_queue || !clicks || !frames || !rendered) Error_Handler();
    /* Native FreeRTOS priorities; bootstrap CMSIS task deletes itself afterwards. */
    if (xTaskCreate(InputTask, "entrada", 256, NULL, 4, NULL) != pdPASS ||
        xTaskCreate(GameTask, "jogo", 512, NULL, 3, NULL) != pdPASS ||
        xTaskCreate(RenderTask, "display", 512, NULL, 1, NULL) != pdPASS) Error_Handler();
}

void vApplicationMallocFailedHook(void) { Error_Handler(); }
void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task; (void)name;
    Error_Handler();
}
