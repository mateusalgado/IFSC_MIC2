/* Execute the real task bodies with copied queues and controlled time.
 * This checks wiring/fault paths, not FreeRTOS preemption or hardware timing. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"
#include "app_game.h"
#include "maze_game.h"
#include "game_renderer.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

struct TestQueue
{
    unsigned length, item_size, count;
    unsigned char data[8][256];
};
static struct TestQueue queues[5];
static TaskFunction_t tasks[3];
static unsigned queue_count, task_count, cycles, limit, critical_depth, isr_calls;
static unsigned display_calls;
static uint32_t now;
static int running_task = -1;
static int failed_queue = -1, failed_task = -1;
static bool display_error, overflow_clicks, fatal_error;
static jmp_buf stop;
static Game displayed;

QueueHandle_t xQueueCreate(unsigned length, unsigned item_size)
{
    unsigned index = queue_count++;
    assert(index < 5 && length <= 8 && item_size <= 256);
    queues[index].length = length;
    queues[index].item_size = item_size;
    return (int)index == failed_queue ? NULL : &queues[index];
}
BaseType_t xQueueOverwrite(QueueHandle_t q, const void *item)
{
    assert(q && q->length == 1);
    memcpy(q->data[0], item, q->item_size);
    q->count = 1;
    return pdPASS;
}
BaseType_t xQueueOverwriteFromISR(QueueHandle_t q, const void *item, BaseType_t *woken)
{
    ++isr_calls;
    *woken = pdFALSE;
    return xQueueOverwrite(q, item);
}
BaseType_t xQueueSend(QueueHandle_t q, const void *item, TickType_t timeout)
{
    assert(timeout == 0);
    if (q->count == q->length)
    {
        return pdFALSE;
    }
    memcpy(q->data[q->count++], item, q->item_size);
    return pdPASS;
}
BaseType_t xQueuePeek(QueueHandle_t q, void *item, TickType_t timeout)
{
    assert(timeout == 0);
    if (q->count == 0)
    {
        return pdFALSE;
    }
    memcpy(item, q->data[0], q->item_size);
    return pdPASS;
}
BaseType_t xQueueReceive(QueueHandle_t q, void *item, TickType_t timeout)
{
    if (xQueuePeek(q, item, timeout) != pdPASS)
    {
        return pdFALSE;
    }
    --q->count;
    for (unsigned i = 0; i < q->count; ++i)
    {
        memcpy(q->data[i], q->data[i + 1], q->item_size);
    }
    return pdPASS;
}
BaseType_t xTaskCreate(TaskFunction_t task, const char *name, unsigned stack, void *argument,
                       unsigned priority, void *handle)
{
    static const char *names[] = {"entrada", "jogo", "display"};
    static const unsigned stacks[] = {256, 512, 512}, priorities[] = {4, 3, 1};
    unsigned index = task_count++;
    assert(index < 3 && strcmp(name, names[index]) == 0);
    assert(stack == stacks[index] && priority == priorities[index]);
    assert(argument == NULL && handle == NULL);
    tasks[index] = task;
    return (int)index == failed_task ? pdFALSE : pdPASS;
}
TickType_t xTaskGetTickCount(void)
{
    return now;
}
uint32_t HAL_GetTick(void)
{
    return now;
}
void vTaskDelay(TickType_t ticks)
{
    now += ticks;
}
void vTaskDelayUntil(TickType_t *wake, TickType_t period)
{
    assert(period == (running_task == 2 ? 33U : 10U));
    *wake += period;
    now += period;
    ++cycles;
    if (cycles >= limit)
    {
        longjmp(stop, 1);
    }
    if (running_task == 0)
    {
        App_AdcFromISR(2048, 2048, now);
    }
    if (running_task == 1 && queues[3].count != 0)
    {
        Game frame;
        xQueuePeek(&queues[3], &frame, 0);
        xQueueOverwrite(&queues[4], &frame.scene);
    }
}
UBaseType_t uxTaskGetStackHighWaterMark(void *task)
{
    assert(task == NULL);
    return 32;
}
size_t xPortGetMinimumEverFreeHeapSize(void)
{
    return 1000;
}
void TestEnterCritical(void)
{
    ++critical_depth;
}
void TestExitCritical(void)
{
    assert(critical_depth > 0);
    --critical_depth;
}
void TestYield(BaseType_t woken)
{
    assert(woken == pdFALSE);
}
int HAL_GPIO_ReadPin(int port, int pin)
{
    assert(port == GPIOC && pin == GPIO_PIN_15);
    bool pressed = cycles >= 110;
    if (overflow_clicks)
    {
        pressed = cycles >= 110 && (cycles / 8) % 2 == 0;
    }
    return pressed ? GPIO_PIN_RESET : 1;
}
void HAL_GPIO_TogglePin(int port, int pin)
{
    assert(port == LED_GPIO_Port && pin == LED_Pin);
}
void Error_Handler(void)
{
    fatal_error = true;
    longjmp(stop, 1);
}
void ST7735_Init(void)
{
    assert(running_task == 2);
}
bool ST7735_HasError(void)
{
    return display_error;
}
void Renderer_Init(void)
{
    assert(running_task == 2);
}
void Renderer_Draw(const Game *game)
{
    assert(running_task == 2);
    displayed = *game;
    ++display_calls;
}

static void RunTask(unsigned index, unsigned iterations)
{
    running_task = (int)index;
    cycles = 0;
    limit = iterations;
    if (setjmp(stop) == 0)
    {
        tasks[index](NULL);
    }
    assert(!fatal_error && critical_depth == 0);
}
static Game LastFrame(void)
{
    Game frame;
    assert(queues[3].item_size == sizeof(frame));
    assert(xQueuePeek(&queues[3], &frame, 0) == pdPASS);
    return frame;
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    App_AdcFromISR(2048, 2048, 0); /* No queue exists yet. */
    assert(isr_calls == 0);
    if (strcmp(argv[1], "queue-allocation") == 0)
    {
        failed_queue = 2;
    }
    if (strcmp(argv[1], "task-allocation") == 0)
    {
        failed_task = 1;
    }
    if (setjmp(stop) == 0)
    {
        App_Start();
    }
    if (failed_queue >= 0 || failed_task >= 0)
    {
        assert(fatal_error);
        assert(task_count == (failed_queue >= 0 ? 0U : 2U));
        puts("RTOS allocation failure handled.");
        return 0;
    }
    assert(queue_count == 5 && task_count == 3);
    assert(queues[0].length == 1 && queues[1].length == 1 && queues[2].length == 8);
    assert(queues[3].length == 1 && queues[4].length == 1);
    if (strcmp(argv[1], "no-adc") == 0)
    {
        RunTask(1, 110);
        Game frame = LastFrame();
        assert(frame.state == GAME_FAULT && frame.reason == GAME_ADC_ERROR);
    }
    else
    {
        overflow_clicks = strcmp(argv[1], "click-overflow") == 0;
        App_AdcFromISR(0, 0, now);
        App_AdcFromISR(2048, 2048, now); /* Only the newest sample reaches calibration. */
        RunTask(0, overflow_clicks ? 300 : 140);
        assert(isr_calls > 100);
        if (!overflow_clicks)
        {
            assert(queues[2].count == 1); /* Held button -> one click. */
        }
        if (strcmp(argv[1], "stale-input") == 0)
        {
            RunTask(1, 15);
            Game frame = LastFrame();
            assert(frame.state == GAME_FAULT && frame.reason == GAME_ADC_ERROR);
        }
        else
        {
            RunTask(1, 1);
            Game frame = LastFrame();
            if (overflow_clicks)
            {
                assert(frame.state == GAME_FAULT && frame.reason == GAME_QUEUE_ERROR);
            }
            else
            {
                assert(frame.state == GAME_TITLE && queues[2].count == 0);
                display_error = strcmp(argv[1], "display-error") == 0;
                RunTask(2, 1);
                assert(display_calls == 1 && displayed.scene == frame.scene);
                if (display_error)
                {
                    assert(queues[4].count == 0); /* Failed SPI must not acknowledge a frame. */
                    RunTask(1, 1);
                    frame = LastFrame();
                    assert(frame.state == GAME_FAULT && frame.reason == GAME_DISPLAY_ERROR);
                }
                else
                {
                    uint32_t ack = 0;
                    assert(xQueuePeek(&queues[4], &ack, 0) == pdPASS && ack == frame.scene);
                }
            }
        }
    }
    printf("RTOS task/queue scenario passed: %s\n", argv[1]);
    return 0;
}
