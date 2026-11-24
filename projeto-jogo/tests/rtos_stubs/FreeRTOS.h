#ifndef RTOS_TEST_H
#define RTOS_TEST_H
#include <stddef.h>
#include <stdint.h>
typedef int BaseType_t;
typedef unsigned UBaseType_t;
typedef uint32_t TickType_t;
typedef void (*TaskFunction_t)(void *);
typedef struct TestQueue *QueueHandle_t;
#define pdPASS 1
#define pdFALSE 0
#define pdMS_TO_TICKS(ms) (ms)
void TestEnterCritical(void);
void TestExitCritical(void);
void TestYield(BaseType_t woken);
#define taskENTER_CRITICAL() TestEnterCritical()
#define taskEXIT_CRITICAL() TestExitCritical()
#define portYIELD_FROM_ISR(woken) TestYield(woken)
size_t xPortGetMinimumEverFreeHeapSize(void);
#endif
