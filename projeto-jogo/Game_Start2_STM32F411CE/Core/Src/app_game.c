/* Integracao RTOS: cada tarefa tem uma responsabilidade e troca copias por filas. */
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

#define INPUT_PRIORITY 4U
#define GAME_PRIORITY 3U
#define DISPLAY_PRIORITY 1U
#define INPUT_STACK_WORDS 256U
#define GAME_STACK_WORDS 512U
#define DISPLAY_STACK_WORDS 512U
#define CLICK_QUEUE_LENGTH 8U
#define INPUT_MAX_AGE_MS 100U
#define ADC_START_TIMEOUT_MS 1000U
#define DISPLAY_TIMEOUT_MS 2000U

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint32_t timestamp;
} AdcSample;

typedef struct
{
    int16_t x;
    int16_t y;
    bool valid;
    bool calibrated;
    uint32_t timestamp;
} JoystickSample;

/* Filas de tamanho 1 guardam o valor mais recente, sem acumular leituras. */
static QueueHandle_t adc_queue;            /* Interrupcao -> entrada: ADC bruto. */
static QueueHandle_t input_queue;          /* Entrada -> jogo: eixos tratados. */
static QueueHandle_t frame_queue;          /* Jogo -> display: copia do estado. */
static QueueHandle_t rendered_scene_queue; /* Display -> jogo: cena desenhada. */
/* Cliques sao eventos: guardar ate oito, em ordem de chegada. */
static QueueHandle_t click_queue;
static volatile GameReason first_fault;

/* Consultar pelo debugger: pilhas em palavras de 32 bits; heap em bytes. */
volatile uint32_t game_input_stack_free;
volatile uint32_t game_logic_stack_free;
volatile uint32_t game_render_stack_free;
volatile uint32_t game_late_ticks;
volatile uint32_t game_min_free_heap;

static void RegisterFault(GameReason reason)
{
    /* Preservar a primeira falha, mesmo se duas tarefas falharem em seguida. */
    taskENTER_CRITICAL();
    if (first_fault == GAME_NO_ERROR)
    {
        first_fault = reason;
    }
    taskEXIT_CRITICAL();
}

void App_AdcFromISR(uint16_t x, uint16_t y, uint32_t timestamp)
{
    AdcSample sample = {.x = x, .y = y, .timestamp = timestamp};
    BaseType_t higher_priority_task_woken = pdFALSE;
    if (adc_queue != NULL)
    {
        /* Em interrupcoes, usar FromISR: sem bloquear nem desenhar. */
        xQueueOverwriteFromISR(adc_queue, &sample, &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}

static void InputTask(void *argument)
{
    (void)argument;
    Joy joystick;
    Joy_Init(&joystick);
    TickType_t last_wake = xTaskGetTickCount();
    for (;;)
    {
        AdcSample sample = {0};
        bool available = xQueuePeek(adc_queue, &sample, 0) == pdPASS;
        uint32_t now = HAL_GetTick();
        bool valid = available && (uint32_t)(now - sample.timestamp) <= INPUT_MAX_AGE_MS;
        bool button_pressed = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET;
        JoyResult result = Joy_Update(&joystick, sample.x, sample.y, button_pressed, valid);
        JoystickSample input = {.x = result.x,
                                .y = result.y,
                                .valid = valid,
                                .calibrated = result.calibrated,
                                .timestamp = now};
        xQueueOverwrite(input_queue, &input);
        if (result.click)
        {
            if (xQueueSend(click_queue, &now, 0) != pdPASS)
            {
                RegisterFault(GAME_QUEUE_ERROR);
            }
        }
        game_input_stack_free = uxTaskGetStackHighWaterMark(NULL);
        /* Espera pelo proximo periodo sem ocupar a CPU. */
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(GAME_STEP_MS));
    }
}

static GameInput ReadGameInput(const Game *game, uint32_t *now)
{
    JoystickSample input = {0};
    uint32_t rendered_scene = 0;
    uint32_t click_time = 0;
    bool clicked = false;
    xQueuePeek(input_queue, &input, 0);
    xQueuePeek(rendered_scene_queue, &rendered_scene, 0);
    /* Varios eventos no mesmo ciclo viram uma unica acao de confirmar. */
    while (xQueueReceive(click_queue, &click_time, 0) == pdPASS)
    {
        clicked = true;
    }
    *now = HAL_GetTick();
    GameInput commands = {.x = input.x,
                          .y = input.y,
                          .calibrated = input.calibrated,
                          .valid =
                              input.valid && (uint32_t)(*now - input.timestamp) <= INPUT_MAX_AGE_MS,
                          .click = clicked,
                          .render_ready = rendered_scene == game->scene};
    return commands;
}

static void CheckGameFaults(Game *game, const GameInput *input)
{
    if (first_fault != GAME_NO_ERROR)
    {
        Game_Fail(game, first_fault);
    }
    if (game->state != GAME_CALIBRATING && !input->render_ready &&
        game->state_ms > DISPLAY_TIMEOUT_MS)
    {
        Game_Fail(game, GAME_DISPLAY_ERROR);
    }
    if (game->state == GAME_CALIBRATING && !input->valid && game->state_ms > ADC_START_TIMEOUT_MS)
    {
        Game_Fail(game, GAME_ADC_ERROR);
    }
}

static void GameTask(void *argument)
{
    (void)argument;
    Game game;
    Game_Init(&game);
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t previous_time = HAL_GetTick();
    uint32_t led_time = previous_time;
    for (;;)
    {
        uint32_t now;
        GameInput input = ReadGameInput(&game, &now);
        uint32_t elapsed_ms = now - previous_time;
        previous_time = now;
        if (elapsed_ms > GAME_STEP_MS + 2U)
        {
            ++game_late_ticks;
        }
        CheckGameFaults(&game, &input);
        Game_Tick(&game, &input, elapsed_ms);
        /* A fila copia Game: o display nao acessa esta variavel diretamente. */
        xQueueOverwrite(frame_queue, &game);
        uint32_t led_period_ms = game.state == GAME_FAULT ? 100U : 500U;
        if ((uint32_t)(now - led_time) >= led_period_ms)
        {
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            led_time = now;
        }
        game_logic_stack_free = uxTaskGetStackHighWaterMark(NULL);
        game_min_free_heap = xPortGetMinimumEverFreeHeapSize();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(GAME_STEP_MS));
    }
}

static void RenderTask(void *argument)
{
    (void)argument;
    vTaskDelay(pdMS_TO_TICKS(100));
    ST7735_Init();
    Renderer_Init();
    TickType_t last_wake = xTaskGetTickCount();
    for (;;)
    {
        Game frame;
        if (xQueuePeek(frame_queue, &frame, 0) == pdPASS)
        {
            Renderer_Draw(&frame);
            if (ST7735_HasError())
            {
                RegisterFault(GAME_DISPLAY_ERROR);
            }
            else
            {
                xQueueOverwrite(rendered_scene_queue, &frame.scene);
            }
        }
        game_render_stack_free = uxTaskGetStackHighWaterMark(NULL);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(GAME_RENDER_MS));
    }
}

void App_Start(void)
{
    /* Executado uma vez pela tarefa inicial gerada pelo CubeMX. */
    adc_queue = xQueueCreate(1, sizeof(AdcSample));
    input_queue = xQueueCreate(1, sizeof(JoystickSample));
    click_queue = xQueueCreate(CLICK_QUEUE_LENGTH, sizeof(uint32_t));
    frame_queue = xQueueCreate(1, sizeof(Game));
    rendered_scene_queue = xQueueCreate(1, sizeof(uint32_t));
    if (adc_queue == NULL || input_queue == NULL || click_queue == NULL || frame_queue == NULL ||
        rendered_scene_queue == NULL)
    {
        Error_Handler();
    }
    /* No FreeRTOS nativo, numero maior = maior prioridade; pilha em palavras. */
    if (xTaskCreate(InputTask, "entrada", INPUT_STACK_WORDS, NULL, INPUT_PRIORITY, NULL) != pdPASS)
    {
        Error_Handler();
    }
    if (xTaskCreate(GameTask, "jogo", GAME_STACK_WORDS, NULL, GAME_PRIORITY, NULL) != pdPASS)
    {
        Error_Handler();
    }
    if (xTaskCreate(RenderTask, "display", DISPLAY_STACK_WORDS, NULL, DISPLAY_PRIORITY, NULL) !=
        pdPASS)
    {
        Error_Handler();
    }
}
