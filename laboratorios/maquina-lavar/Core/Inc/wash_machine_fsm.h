#ifndef __WASH_MACHINE_FSM_H
#define __WASH_MACHINE_FSM_H

#include "main.h"

typedef enum {
    STATE_IDLE,
    STATE_DOOR_CLOSED,
    STATE_FILLING,
    STATE_WASHING,
    STATE_DRAINING,
    STATE_SPINNING,
    STATE_READY,
    STATE_ERROR
} WashMachineState;

typedef struct {
    WashMachineState current_state;
    WashMachineState next_state;
    uint32_t timer_ms;
    uint32_t timer_counter;
    void (*state_handler)(void);
    uint8_t door_locked;
    uint8_t error_flag;
} WashMachine;

extern WashMachine wash_machine;

void wash_machine_init(void);
void wash_machine_update(void);
void wash_machine_tick(void);
void transition_to(WashMachineState next_state);
void wash_machine_emergency_stop(void);
void wash_machine_reset_error(void);

#endif
