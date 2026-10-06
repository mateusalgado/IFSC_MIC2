/* FreeRTOS exercises adapted from the STM32F103 examples to Barry's STM32F411CE.
 * Only the selected LAB_EXAMPLE is compiled; clock, USB and GPIO stay in main.c.
 */
#include "lab_examples.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "PRNG_LFSR.h"
#include <stdint.h>
#include <stdio.h>

#define MS(n) pdMS_TO_TICKS(n)
#define CREATE(fn, name, arg, priority) \
  xTaskCreate((fn), (name), 256, (arg), (priority), NULL)

#if LAB_EXAMPLE == 1
static void task1(void *arg)
{
  (void)arg;
  for (;;) { printf("Task 1 is running.\r\n"); HAL_Delay(200); }
}
static void task2(void *arg)
{
  (void)arg;
  for (;;) { printf("Task 2 is running.\r\n"); HAL_Delay(200); }
}
#elif LAB_EXAMPLE == 2 || LAB_EXAMPLE == 3
static void messageTask(void *arg)
{
  for (;;) { printf("%s", (const char *)arg); HAL_Delay(200); }
}
#elif LAB_EXAMPLE == 4 || LAB_EXAMPLE == 5
static void timedMessageTask(void *arg)
{
#if LAB_EXAMPLE == 5
  TickType_t last = xTaskGetTickCount();
#endif
  for (;;) {
    printf("%s", (const char *)arg);
#if LAB_EXAMPLE == 4
    vTaskDelay(MS(250));
#else
    vTaskDelayUntil(&last, MS(250));
#endif
  }
}
#elif LAB_EXAMPLE == 6
static void continuousTask(void *arg)
{
  for (;;) { printf("%s", (const char *)arg); HAL_Delay(200); }
}
static void periodicTask(void *arg)
{
  TickType_t last = xTaskGetTickCount();
  (void)arg;
  for (;;) {
    printf("Periodic task is running.\r\n");
    vTaskDelayUntil(&last, MS(250));
  }
}
#elif LAB_EXAMPLE == 7
static volatile uint32_t idleCycles;
static void idleReportTask(void *arg)
{
  for (;;) {
    printf("%s idle cycles = %lu\r\n", (const char *)arg,
           (unsigned long)idleCycles);
    vTaskDelay(MS(250));
  }
}
#elif LAB_EXAMPLE == 8
static TaskHandle_t secondTask;
static void priorityTask1(void *arg)
{
  UBaseType_t own = uxTaskPriorityGet(NULL);
  (void)arg;
  for (;;) {
    printf("Task 1 raises Task 2 priority.\r\n");
    vTaskPrioritySet(secondTask, own + 1);
  }
}
static void priorityTask2(void *arg)
{
  UBaseType_t raised = 3;
  (void)arg;
  for (;;) {
    printf("Task 2 lowers its own priority.\r\n");
    vTaskPrioritySet(NULL, raised - 2);
  }
}
#elif LAB_EXAMPLE == 9
static void shortLivedTask(void *arg)
{
  (void)arg;
  printf("Task 2 runs and deletes itself.\r\n");
  vTaskDelete(NULL);
}
static void creatorTask(void *arg)
{
  (void)arg;
  for (;;) {
    printf("Task 1 creates Task 2.\r\n");
    CREATE(shortLivedTask, "Task 2", NULL, 2);
    vTaskDelay(MS(100));
  }
}
#elif LAB_EXAMPLE == 10 || LAB_EXAMPLE == 11
static QueueHandle_t dataQueue;
static SemaphoreHandle_t uartMutex;
#if LAB_EXAMPLE == 10
static void numberSender(void *arg)
{
  uint32_t value = (uint32_t)(uintptr_t)arg;
  for (;;) {
    if (xQueueSendToBack(dataQueue, &value, 0) != pdPASS) {
      xSemaphoreTake(uartMutex, portMAX_DELAY);
      printf("Could not send to the queue.\r\n");
      xSemaphoreGive(uartMutex);
    }
    taskYIELD();
  }
}
static void numberReceiver(void *arg)
{
  uint32_t value;
  (void)arg;
  for (;;) {
    if (xQueueReceive(dataQueue, &value, MS(250)) == pdPASS) {
      xSemaphoreTake(uartMutex, portMAX_DELAY);
      printf("Received = %lu\r\n", (unsigned long)value);
      xSemaphoreGive(uartMutex);
    }
  }
}
#else
typedef struct { uint8_t value; uint8_t source; } Data;
static Data senderData[2] = {{100, 1}, {200, 2}};
static void structSender(void *arg)
{
  for (;;) {
    if (xQueueSendToBack(dataQueue, arg, MS(250)) != pdPASS) {
      xSemaphoreTake(uartMutex, portMAX_DELAY);
      printf("Could not send to the queue.\r\n");
      xSemaphoreGive(uartMutex);
    }
    taskYIELD();
  }
}
static void structReceiver(void *arg)
{
  Data value;
  (void)arg;
  for (;;) {
    if (xQueueReceive(dataQueue, &value, portMAX_DELAY) == pdPASS) {
      xSemaphoreTake(uartMutex, portMAX_DELAY);
      printf("From Sender %u = %u\r\n", value.source, value.value);
      xSemaphoreGive(uartMutex);
    }
  }
}
#endif
#elif LAB_EXAMPLE == 12 || LAB_EXAMPLE == 13
static SemaphoreHandle_t eventSemaphore;
static SemaphoreHandle_t uartMutex;
static void interruptGenerator(void *arg)
{
  (void)arg;
  for (;;) {
    vTaskDelay(MS(500));
    xSemaphoreTake(uartMutex, portMAX_DELAY);
    printf("Generating software interrupt.\r\n");
    xSemaphoreGive(uartMutex);
    NVIC_SetPendingIRQ(EXTI0_IRQn);
  }
}
static void eventHandler(void *arg)
{
  (void)arg;
  for (;;) {
    xSemaphoreTake(eventSemaphore, portMAX_DELAY);
    xSemaphoreTake(uartMutex, portMAX_DELAY);
    printf("Handler task: processing event.\r\n");
    xSemaphoreGive(uartMutex);
  }
}
#elif LAB_EXAMPLE == 14
static QueueHandle_t numberQueue;
static QueueHandle_t stringQueue;
static SemaphoreHandle_t uartMutex;
static void numberGenerator(void *arg)
{
  TickType_t last = xTaskGetTickCount();
  uint32_t number = 0;
  (void)arg;
  for (;;) {
    vTaskDelayUntil(&last, MS(200));
    for (unsigned i = 0; i < 5; ++i) {
      xQueueSendToBack(numberQueue, &number, 0);
      ++number;
    }
    NVIC_SetPendingIRQ(EXTI0_IRQn);
  }
}
static void stringPrinter(void *arg)
{
  const char *message;
  (void)arg;
  for (;;) {
    if (xQueueReceive(stringQueue, &message, portMAX_DELAY) == pdPASS) {
      xSemaphoreTake(uartMutex, portMAX_DELAY);
      printf("%s", message);
      xSemaphoreGive(uartMutex);
    }
  }
}
#elif LAB_EXAMPLE == 15
static SemaphoreHandle_t uartMutex;
static void mutexPrintTask(void *arg)
{
  for (;;) {
    xSemaphoreTake(uartMutex, portMAX_DELAY);
    printf("%s %lu\r\n", (const char *)arg,
           (unsigned long)(prng_LFSR() % 1001));
    xSemaphoreGive(uartMutex);
    vTaskDelay(MS(prng_LFSR() % 1001));
  }
}
#elif LAB_EXAMPLE == 16
static QueueHandle_t printQueue;
static const char *messages[] = {
  "Task 1 *********************************************\r\n",
  "Task 2 ---------------------------------------------\r\n",
  "Message from the tick hook interrupt ##############\r\n"
};
static void gatekeeperTask(void *arg)
{
  const char *message;
  (void)arg;
  for (;;) {
    if (xQueueReceive(printQueue, &message, portMAX_DELAY) == pdPASS)
      printf("%s", message);
  }
}
static void queuedPrintTask(void *arg)
{
  uintptr_t index = (uintptr_t)arg;
  for (;;) {
    xQueueSendToBack(printQueue, &messages[index], 0);
    vTaskDelay(MS(prng_LFSR() % 512));
  }
}
#endif

#if LAB_EXAMPLE >= 12 && LAB_EXAMPLE <= 14
void EXTI0_IRQHandler(void)
{
  BaseType_t woke = pdFALSE;
#if LAB_EXAMPLE == 12
  xSemaphoreGiveFromISR(eventSemaphore, &woke);
#elif LAB_EXAMPLE == 13
  for (unsigned i = 0; i < 3; ++i)
    xSemaphoreGiveFromISR(eventSemaphore, &woke);
#else
  static const char *strings[] = {
    "String 0\r\n", "String 1\r\n", "String 2\r\n", "String 3\r\n"
  };
  uint32_t number;
  while (xQueueReceiveFromISR(numberQueue, &number, &woke) == pdTRUE) {
    const char *message = strings[number & 3U];
    xQueueSendToBackFromISR(stringQueue, &message, &woke);
  }
#endif
  portYIELD_FROM_ISR(woke);
}
#endif

void vApplicationIdleHook(void)
{
#if LAB_EXAMPLE == 7
  ++idleCycles;
#endif
}

void vApplicationTickHook(void)
{
#if LAB_EXAMPLE == 16
  static uint32_t ticks;
  if (++ticks >= MS(200)) {
    BaseType_t woke = pdFALSE;
    ticks = 0;
    if (printQueue != NULL)
      xQueueSendToFrontFromISR(printQueue, &messages[2], &woke);
  }
#endif
}

void LabExamples_Start(void)
{
#if LAB_EXAMPLE == 1
  CREATE(task1, "Task 1", NULL, 1);
  CREATE(task2, "Task 2", NULL, 1);
#elif LAB_EXAMPLE == 2 || LAB_EXAMPLE == 3
  CREATE(messageTask, "Task 1", "Task 1 is running.\r\n", 1);
  CREATE(messageTask, "Task 2", "Task 2 is running.\r\n", LAB_EXAMPLE == 2 ? 1 : 2);
#elif LAB_EXAMPLE == 4 || LAB_EXAMPLE == 5
  CREATE(timedMessageTask, "Task 1", "Task 1 is running.\r\n", 1);
  CREATE(timedMessageTask, "Task 2", "Task 2 is running.\r\n", 2);
#elif LAB_EXAMPLE == 6
  CREATE(continuousTask, "Task 1", "Continuous task 1.\r\n", 1);
  CREATE(continuousTask, "Task 2", "Continuous task 2.\r\n", 1);
  CREATE(periodicTask, "Task 3", NULL, 2);
#elif LAB_EXAMPLE == 7
  CREATE(idleReportTask, "Task 1", "Task 1", 1);
  CREATE(idleReportTask, "Task 2", "Task 2", 1);
#elif LAB_EXAMPLE == 8
  xTaskCreate(priorityTask2, "Task 2", 256, NULL, 1, &secondTask);
  CREATE(priorityTask1, "Task 1", NULL, 2);
#elif LAB_EXAMPLE == 9
  CREATE(creatorTask, "Task 1", NULL, 1);
#elif LAB_EXAMPLE == 10
  dataQueue = xQueueCreate(5, sizeof(uint32_t));
  uartMutex = xSemaphoreCreateMutex();
  if (dataQueue != NULL && uartMutex != NULL) {
    CREATE(numberSender, "Sender 1", (void *)(uintptr_t)100, 1);
    CREATE(numberSender, "Sender 2", (void *)(uintptr_t)200, 1);
    CREATE(numberReceiver, "Receiver", NULL, 2);
  }
#elif LAB_EXAMPLE == 11
  dataQueue = xQueueCreate(3, sizeof(Data));
  uartMutex = xSemaphoreCreateMutex();
  if (dataQueue != NULL && uartMutex != NULL) {
    CREATE(structSender, "Sender 1", &senderData[0], 2);
    CREATE(structSender, "Sender 2", &senderData[1], 2);
    CREATE(structReceiver, "Receiver", NULL, 1);
  }
#elif LAB_EXAMPLE == 12 || LAB_EXAMPLE == 13
  uartMutex = xSemaphoreCreateMutex();
#if LAB_EXAMPLE == 12
  eventSemaphore = xSemaphoreCreateBinary();
#else
  eventSemaphore = xSemaphoreCreateCounting(10, 0);
#endif
  if (uartMutex != NULL && eventSemaphore != NULL) {
    NVIC_SetPriority(EXTI0_IRQn, 6);
    NVIC_EnableIRQ(EXTI0_IRQn);
    CREATE(eventHandler, "Handler", NULL, 3);
    CREATE(interruptGenerator, "Generator", NULL, 1);
  }
#elif LAB_EXAMPLE == 14
  uartMutex = xSemaphoreCreateMutex();
  numberQueue = xQueueCreate(10, sizeof(uint32_t));
  stringQueue = xQueueCreate(10, sizeof(const char *));
  if (uartMutex != NULL && numberQueue != NULL && stringQueue != NULL) {
    NVIC_SetPriority(EXTI0_IRQn, 6);
    NVIC_EnableIRQ(EXTI0_IRQn);
    CREATE(numberGenerator, "Generator", NULL, 1);
    CREATE(stringPrinter, "Printer", NULL, 2);
  }
#elif LAB_EXAMPLE == 15
  init_LFSR(13);
  uartMutex = xSemaphoreCreateMutex();
  if (uartMutex != NULL) {
    CREATE(mutexPrintTask, "Print 1", "Task 1 =", 1);
    CREATE(mutexPrintTask, "Print 2", "Task 2 =", 2);
  }
#elif LAB_EXAMPLE == 16
  init_LFSR(13);
  printQueue = xQueueCreate(5, sizeof(const char *));
  if (printQueue != NULL) {
    CREATE(queuedPrintTask, "Print 1", (void *)(uintptr_t)0, 1);
    CREATE(queuedPrintTask, "Print 2", (void *)(uintptr_t)1, 2);
    CREATE(gatekeeperTask, "Gatekeeper", NULL, 1);
  }
#endif
}
