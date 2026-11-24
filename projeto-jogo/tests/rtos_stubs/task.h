#ifndef TASK_TEST_H
#define TASK_TEST_H
#include "FreeRTOS.h"
TickType_t xTaskGetTickCount(void);
BaseType_t xTaskCreate(TaskFunction_t task, const char *name, unsigned stack, void *argument,
                       unsigned priority, void *handle);
void vTaskDelay(TickType_t ticks);
void vTaskDelayUntil(TickType_t *wake, TickType_t period);
UBaseType_t uxTaskGetStackHighWaterMark(void *task);
#endif
