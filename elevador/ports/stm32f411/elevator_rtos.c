#include "elevator_rtos.h"
#include "board_port.h"
#include "display.h"
#include "controller.h"
#include "elevator_config.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include <stdio.h>
#include <string.h>
typedef struct { Elevator elevator; uint32_t now, dropped; } Snapshot;
static QueueHandle_t display_queue, log_queue;
static StaticQueue_t display_queue_cb, log_queue_cb;
static uint8_t display_storage[sizeof(Snapshot)];
static uint8_t log_storage[EL_LOG_QUEUE_LENGTH * sizeof(Snapshot)];
static StaticTask_t control_cb, display_cb, log_cb;
static StackType_t control_stack[512], display_stack[512], log_stack[768];
static void control_task(void *arg) {
    (void)arg;
    Controller controller;
    uint32_t now = HAL_GetTick(), dropped = 0, previous_revision = UINT32_MAX;
    uint32_t last_log = now;
    TickType_t wake = xTaskGetTickCount();
    controller_init(&controller, now);
    board_motor_write(controller.elevator.motor.out);
    for (;;) {
        now = HAL_GetTick();
        RawInputs raw = board_read_inputs();
        controller_step(&controller, &raw, now);
        const Elevator e = controller.elevator;
        board_motor_write(e.motor.out);
        board_indicators(e.state);
        Snapshot snapshot = {e, now, dropped};
        (void)xQueueOverwrite(display_queue, &snapshot);
        if (previous_revision != e.revision || (uint32_t)(now - last_log) >= EL_LOG_PERIOD_MS) {
            if (xQueueSend(log_queue, &snapshot, 0) != pdPASS) ++dropped;
            previous_revision = e.revision;
            last_log = now;
        }
        board_watchdog_refresh();
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(EL_CONTROL_MS));
    }
}
static void display_task(void *arg) {
    (void)arg;
    Snapshot s; char lines[2][17];
    for (;;) {
        if (xQueueReceive(display_queue, &s, portMAX_DELAY) == pdPASS) {
            display_format(&s.elevator, s.now, lines);
            board_lcd_write(lines[0], lines[1]);
        }
        vTaskDelay(pdMS_TO_TICKS(EL_LCD_MS));
    }
}
static void log_task(void *arg) {
    (void)arg;
    extern USBD_HandleTypeDef hUsbDeviceFS;
    static uint8_t tx[192]; /* persists until USB transfer completes */
    Snapshot s;
    uint32_t usb_dropped = 0;
    MX_USB_DEVICE_Init();
    for (;;) {
        if (xQueueReceive(log_queue, &s, portMAX_DELAY) != pdPASS) continue;
        /* USB IRQ is priority 5 in the reference project: masked by this short
         * critical section. No waits or formatting inside it. Single CDC owner.
         */
        bool available;
        taskENTER_CRITICAL();
        USBD_CDC_HandleTypeDef *cdc = hUsbDeviceFS.pClassData;
        available = hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED && cdc && !cdc->TxState;
        taskEXIT_CRITICAL();
        if (!available) { ++usb_dropped; continue; }
        int len = snprintf((char *)tx, sizeof tx,
            "ms=%lu state=%s fault=%s last=%d dest=%d pwm=%u notice=%u lost=%lu usb_lost=%lu\r\n",
            (unsigned long)s.now, elevator_state_name(s.elevator.state),
            elevator_fault_name(s.elevator.fault), s.elevator.last_floor,
            s.elevator.destination, s.elevator.motor.out.pwm / 10u,
            (unsigned)s.elevator.notice, (unsigned long)s.dropped, (unsigned long)usb_dropped);
        uint16_t n = (uint16_t)(len < 0 ? 0 : len >= (int)sizeof tx ? (int)sizeof tx - 1 : len);
        taskENTER_CRITICAL();
        cdc = hUsbDeviceFS.pClassData;
        if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED || !cdc || cdc->TxState ||
            CDC_Transmit_FS(tx, n) != USBD_OK) ++usb_dropped;
        taskEXIT_CRITICAL();
    }
}
void ElevatorRTOS_Start(void) {
    configASSERT(pdMS_TO_TICKS(EL_CONTROL_MS) > 0);
    display_queue = xQueueCreateStatic(1, sizeof(Snapshot), display_storage, &display_queue_cb);
    log_queue = xQueueCreateStatic(EL_LOG_QUEUE_LENGTH, sizeof(Snapshot), log_storage, &log_queue_cb);
    configASSERT(display_queue && log_queue);
    configASSERT(xTaskCreateStatic(control_task, "Control", 512, NULL, 4, control_stack, &control_cb));
    configASSERT(xTaskCreateStatic(display_task, "Display", 512, NULL, 2, display_stack, &display_cb));
    configASSERT(xTaskCreateStatic(log_task, "USBLog", 768, NULL, 1, log_stack, &log_cb));
}
void vApplicationIdleHook(void) { }
void vApplicationTickHook(void) { }
void elevator_panic(void) {
    board_fault_disable();
    taskDISABLE_INTERRUPTS();
    for (;;) { }
}
void vApplicationMallocFailedHook(void) { elevator_panic(); }
void vApplicationStackOverflowHook(TaskHandle_t task, char *name) {
    (void)task; (void)name; elevator_panic();
}
void vApplicationGetIdleTaskMemory(StaticTask_t **cb, StackType_t **stack, uint32_t *size) {
    static StaticTask_t idle_cb;
    static StackType_t idle_stack[configMINIMAL_STACK_SIZE];
    *cb = &idle_cb; *stack = idle_stack; *size = configMINIMAL_STACK_SIZE;
}
