#ifndef ELEVATOR_H
#define ELEVATOR_H
#include "inputs.h"
#include "motor.h"
typedef enum { EL_INIT, EL_HOME, EL_IDLE, EL_UP, EL_DOWN, EL_ARRIVAL, EL_FAULT, EL_STATE_COUNT } ElState;
typedef enum { FAULT_NONE, FAULT_STOP, FAULT_LIMIT, FAULT_SENSOR, FAULT_HOME_TIMEOUT,
    FAULT_TRAVEL_TIMEOUT, FAULT_POSITION, FAULT_INPUT, FAULT_DEADLINE } ElFault;
typedef enum { NOTICE_NONE, NOTICE_BUSY, NOTICE_SAME_FLOOR, NOTICE_NEED_HOME } ElNotice;
typedef struct {
    ElState state;
    ElFault fault;
    ElNotice notice;
    int last_floor, current_floor, destination, selection;
    bool referenced;
    uint32_t state_ms, last_ms, floor_ms, notice_ms, revision;
    Motor motor;
} Elevator;
void elevator_init(Elevator *e, uint32_t now);
void elevator_step(Elevator *e, const Sensors *s, InputEvent event, uint32_t now);
const char *elevator_state_name(ElState state);
const char *elevator_fault_name(ElFault fault);
#endif
