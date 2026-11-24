#ifndef QUEUE_TEST_H
#define QUEUE_TEST_H
#include "FreeRTOS.h"
QueueHandle_t xQueueCreate(unsigned length, unsigned item_size);
BaseType_t xQueuePeek(QueueHandle_t queue, void *item, TickType_t timeout);
BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t timeout);
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t timeout);
BaseType_t xQueueOverwrite(QueueHandle_t queue, const void *item);
BaseType_t xQueueOverwriteFromISR(QueueHandle_t queue, const void *item, BaseType_t *woken);
#endif
