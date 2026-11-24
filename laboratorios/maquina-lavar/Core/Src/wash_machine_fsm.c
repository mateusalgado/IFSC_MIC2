#include "wash_machine_fsm.h"

WashMachine wash_machine = {
    .current_state = STATE_IDLE,
    .next_state = STATE_IDLE,
    .timer_ms = 2000,
    .timer_counter = 0,
    .door_locked = 0,
    .error_flag = 0
};

#define FILL_TIME       2000
#define WASH_TIME       5000
#define DRAIN_TIME      2000
#define SPIN_TIME       3000

#define BTN_START_PIN   GPIO_PIN_0
#define SENSOR_DOOR_PIN GPIO_PIN_1
#define SENSOR_LEVEL_PIN GPIO_PIN_2
#define SENSOR_EMPTY_PIN GPIO_PIN_3
#define MOTOR_PIN       GPIO_PIN_6
#define VALVE_IN_PIN    GPIO_PIN_5
#define VALVE_OUT_PIN   GPIO_PIN_4
#define LED_STATUS_PIN  GPIO_PIN_7

static uint8_t button_pressed(void) {
    return HAL_GPIO_ReadPin(GPIOA, BTN_START_PIN) == GPIO_PIN_SET;
}

static uint8_t door_closed(void) {
    return HAL_GPIO_ReadPin(GPIOA, SENSOR_DOOR_PIN) == GPIO_PIN_SET;
}

static uint8_t tank_full(void) {
    return HAL_GPIO_ReadPin(GPIOA, SENSOR_LEVEL_PIN) == GPIO_PIN_SET;
}

static uint8_t tank_empty(void) {
    return HAL_GPIO_ReadPin(GPIOA, SENSOR_EMPTY_PIN) == GPIO_PIN_SET;
}

static void motor_on(void) {
    HAL_GPIO_WritePin(GPIOA, MOTOR_PIN, GPIO_PIN_SET);
}

static void motor_off(void) {
    HAL_GPIO_WritePin(GPIOA, MOTOR_PIN, GPIO_PIN_RESET);
}

static void valve_in_open(void) {
    HAL_GPIO_WritePin(GPIOA, VALVE_IN_PIN, GPIO_PIN_SET);
}

static void valve_in_close(void) {
    HAL_GPIO_WritePin(GPIOA, VALVE_IN_PIN, GPIO_PIN_RESET);
}

static void valve_out_open(void) {
    HAL_GPIO_WritePin(GPIOA, VALVE_OUT_PIN, GPIO_PIN_SET);
}

static void valve_out_close(void) {
    HAL_GPIO_WritePin(GPIOA, VALVE_OUT_PIN, GPIO_PIN_RESET);
}

static void lock_door(void) {
    wash_machine.door_locked = 1;
}

static void unlock_door(void) {
    wash_machine.door_locked = 0;
}

static void led_toggle(void) {
    HAL_GPIO_TogglePin(GPIOA, LED_STATUS_PIN);
}

static void stop_all_outputs(void) {
    motor_off();
    valve_in_close();
    valve_out_close();
}

void handle_idle(void) {
    unlock_door();
    stop_all_outputs();

    if (button_pressed() && door_closed()) {
        transition_to(STATE_DOOR_CLOSED);
    }
}

void handle_door_closed(void) {
    lock_door();
    if (!door_closed()) {
        wash_machine.error_flag = 1;
        transition_to(STATE_ERROR);
        return;
    }
    transition_to(STATE_FILLING);
}

void handle_filling(void) {
    led_toggle();
    valve_in_open();
    motor_off();

    if (!door_closed()) {
        wash_machine.error_flag = 1;
        transition_to(STATE_ERROR);
        return;
    }

    if (tank_full() || wash_machine.timer_counter >= FILL_TIME) {
        valve_in_close();
        wash_machine.timer_counter = 0;
        transition_to(STATE_WASHING);
    }
}

void handle_washing(void) {
    led_toggle();
    motor_on();
    valve_in_close();
    valve_out_close();

    if (!door_closed()) {
        wash_machine.error_flag = 1;
        transition_to(STATE_ERROR);
        return;
    }

    if (wash_machine.timer_counter >= WASH_TIME) {
        wash_machine.timer_counter = 0;
        transition_to(STATE_DRAINING);
    }
}

void handle_draining(void) {
    led_toggle();
    motor_off();
    valve_out_open();

    if (!door_closed()) {
        wash_machine.error_flag = 1;
        transition_to(STATE_ERROR);
        return;
    }

    if (tank_empty() || wash_machine.timer_counter >= DRAIN_TIME) {
        valve_out_close();
        wash_machine.timer_counter = 0;
        transition_to(STATE_SPINNING);
    }
}

void handle_spinning(void) {
    led_toggle();
    motor_on();

    if (!door_closed()) {
        wash_machine.error_flag = 1;
        transition_to(STATE_ERROR);
        return;
    }

    if (wash_machine.timer_counter >= SPIN_TIME) {
        motor_off();
        wash_machine.timer_counter = 0;
        transition_to(STATE_READY);
    }
}

void handle_ready(void) {
    motor_off();
    valve_in_close();
    valve_out_close();
    unlock_door();

    HAL_GPIO_WritePin(GPIOA, LED_STATUS_PIN, GPIO_PIN_SET);

    if (!door_closed()) {
        transition_to(STATE_IDLE);
    }
}

void handle_error(void) {
    stop_all_outputs();
    unlock_door();
    HAL_GPIO_WritePin(GPIOA, LED_STATUS_PIN, GPIO_PIN_RESET);
}

void wash_machine_init(void) {
    wash_machine.current_state = STATE_IDLE;
    wash_machine.next_state = STATE_IDLE;
    wash_machine.timer_counter = 0;
    wash_machine.door_locked = 0;
    wash_machine.error_flag = 0;
    stop_all_outputs();
    unlock_door();
}

void transition_to(WashMachineState next_state) {
    wash_machine.next_state = next_state;
}

void wash_machine_update(void) {
    if (wash_machine.current_state != wash_machine.next_state) {
        wash_machine.current_state = wash_machine.next_state;
        wash_machine.timer_counter = 0;
    }

    switch (wash_machine.current_state) {
        case STATE_IDLE:
            wash_machine.state_handler = handle_idle;
            handle_idle();
            break;
        case STATE_DOOR_CLOSED:
            wash_machine.state_handler = handle_door_closed;
            handle_door_closed();
            break;
        case STATE_FILLING:
            wash_machine.state_handler = handle_filling;
            handle_filling();
            break;
        case STATE_WASHING:
            wash_machine.state_handler = handle_washing;
            handle_washing();
            break;
        case STATE_DRAINING:
            wash_machine.state_handler = handle_draining;
            handle_draining();
            break;
        case STATE_SPINNING:
            wash_machine.state_handler = handle_spinning;
            handle_spinning();
            break;
        case STATE_READY:
            wash_machine.state_handler = handle_ready;
            handle_ready();
            break;
        case STATE_ERROR:
            wash_machine.state_handler = handle_error;
            handle_error();
            break;
        default:
            wash_machine.state_handler = handle_idle;
            handle_idle();
            break;
    }
}

void wash_machine_tick(void) {
    wash_machine.timer_counter += 10;
}

void wash_machine_emergency_stop(void) {
    stop_all_outputs();
    unlock_door();
    wash_machine.current_state = STATE_IDLE;
    wash_machine.next_state = STATE_IDLE;
}

void wash_machine_reset_error(void) {
    if (wash_machine.current_state == STATE_ERROR) {
        wash_machine.error_flag = 0;
        wash_machine.current_state = STATE_IDLE;
        wash_machine.next_state = STATE_IDLE;
        wash_machine.timer_counter = 0;
        stop_all_outputs();
        unlock_door();
    }
}
