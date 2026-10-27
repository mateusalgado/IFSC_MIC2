#include "elevator.h"
#include "elevator_config.h"
#include <string.h>
static int floor_number(uint8_t bits) {
    return bits == 1 ? 0 : bits == 2 ? 1 : bits == 4 ? 2 : -1;
}
static void state(Elevator *e, ElState next, uint32_t now) {
    e->state = next; e->state_ms = now; ++e->revision;
}
static void notice(Elevator *e, ElNotice n, uint32_t now) {
    e->notice = n; e->notice_ms = now; ++e->revision;
}
static void fail(Elevator *e, ElFault f, uint32_t now) {
    motor_stop(&e->motor, now, MOTOR_FAULT);
    if (e->state != EL_FAULT) {
        e->fault = f; e->referenced = false; e->destination = -1;
        e->notice = NOTICE_NONE; state(e, EL_FAULT, now);
    }
}
void elevator_init(Elevator *e, uint32_t now) {
    memset(e, 0, sizeof *e);
    e->last_floor = e->current_floor = e->destination = -1;
    e->last_ms = e->state_ms = now;
    motor_init(&e->motor, now);
}
static void arrive(Elevator *e, uint32_t now) {
    /* Stop at the confirmed point sensor; no invented pre-arrival deceleration. */
    motor_stop(&e->motor, now, MOTOR_NORMAL);
    e->last_floor = e->current_floor;
    e->referenced = true;
    state(e, EL_ARRIVAL, now);
}
typedef void (*Handler)(Elevator *, InputEvent, uint32_t);
static void initializing(Elevator *e, InputEvent event, uint32_t now) {
    if (event == EV_LONG) {
        e->destination = 0; e->floor_ms = now;
        state(e, EL_HOME, now);
    } else if (event != EV_NONE) notice(e, NOTICE_NEED_HOME, now);
}
static void homing(Elevator *e, InputEvent event, uint32_t now) {
    if (event != EV_NONE) notice(e, NOTICE_BUSY, now);
    if ((uint32_t)(now - e->state_ms) >= EL_HOME_TIMEOUT_MS) {
        fail(e, FAULT_HOME_TIMEOUT, now); return;
    }
    if (e->current_floor == 0) { arrive(e, now); return; }
    motor_step(&e->motor, now, -1, EL_PWM_HOME);
}
static void idle(Elevator *e, InputEvent event, uint32_t now) {
    if (e->current_floor != e->last_floor || !e->referenced) {
        fail(e, FAULT_POSITION, now); return;
    }
    if (event == EV_UP && e->selection < 2) { ++e->selection; ++e->revision; }
    if (event == EV_DOWN && e->selection > 0) { --e->selection; ++e->revision; }
    if (event == EV_CONFIRM) {
        if (e->selection == e->current_floor) { notice(e, NOTICE_SAME_FLOOR, now); return; }
        e->destination = e->selection; e->floor_ms = now;
        e->notice = NOTICE_NONE;
        state(e, e->destination > e->current_floor ? EL_UP : EL_DOWN, now);
    }
}
static void moving(Elevator *e, InputEvent event, uint32_t now) {
    if (event != EV_NONE) notice(e, NOTICE_BUSY, now);
    if ((uint32_t)(now - e->state_ms) >= EL_TRAVEL_TIMEOUT_MS) {
        fail(e, FAULT_TRAVEL_TIMEOUT, now); return;
    }
    int direction = e->state == EL_UP ? 1 : -1;
    if (e->current_floor >= 0 && e->current_floor != e->last_floor) {
        if (e->current_floor != e->last_floor + direction) {
            fail(e, FAULT_SENSOR, now); return;
        }
        e->last_floor = e->current_floor; e->floor_ms = now; ++e->revision;
    }
    if (e->current_floor == e->destination) { arrive(e, now); return; }
    if (e->current_floor >= 0 && (uint32_t)(now - e->floor_ms) >= EL_DEPART_TIMEOUT_MS) {
        fail(e, FAULT_SENSOR, now); return;
    }
    motor_step(&e->motor, now, direction, EL_PWM_TRAVEL);
}
static void arrival(Elevator *e, InputEvent event, uint32_t now) {
    if (event != EV_NONE) notice(e, NOTICE_BUSY, now);
    if (e->current_floor != e->last_floor) { fail(e, FAULT_POSITION, now); return; }
    if ((uint32_t)(now - e->state_ms) >= EL_ARRIVAL_MS) {
        e->selection = e->last_floor; e->destination = -1;
        state(e, EL_IDLE, now);
    }
}
static void faulted(Elevator *e, InputEvent event, uint32_t now) {
    if (event == EV_LONG) {
        e->fault = FAULT_NONE;
        /* Recognition never starts motion. A second, separate long press homes. */
        state(e, EL_INIT, now);
    }
}
void elevator_step(Elevator *e, const Sensors *s, InputEvent event, uint32_t now) {
    static const Handler handlers[EL_STATE_COUNT] = {
        initializing, homing, idle, moving, moving, arrival, faulted
    };
    uint32_t dt = now - e->last_ms; e->last_ms = now;
    e->current_floor = floor_number(s->floors);
    if (e->notice != NOTICE_NONE && (uint32_t)(now - e->notice_ms) >= EL_NOTICE_MS) {
        e->notice = NOTICE_NONE; ++e->revision;
    }
    /* Global guards run before every state and every ordinary input event. */
    if (s->stop) { fail(e, FAULT_STOP, now); return; }
    if (s->lower_limit || s->upper_limit) { fail(e, FAULT_LIMIT, now); return; }
    if (!s->valid) { fail(e, FAULT_INPUT, now); return; }
    if (!s->coherent || s->floors > 7 || (s->floors & (s->floors - 1u))) {
        fail(e, FAULT_SENSOR, now); return;
    }
    if (dt > EL_MAX_GAP_MS) { fail(e, FAULT_DEADLINE, now); return; }
    if (!s->ready) { motor_stop(&e->motor, now, MOTOR_NORMAL); return; }
    if ((unsigned)e->state >= EL_STATE_COUNT) { fail(e, FAULT_SENSOR, now); return; }
    handlers[e->state](e, event, now);
}
const char *elevator_state_name(ElState s) {
    static const char *const names[] = {"INICIO", "REFERENCIA", "PARADO", "SUBIDA", "DESCIDA", "CHEGADA", "FALHA"};
    return (unsigned)s < EL_STATE_COUNT ? names[s] : "INVALIDO";
}
const char *elevator_fault_name(ElFault f) {
    static const char *const names[] = {"nenhuma", "parada", "fim_curso", "sensor", "tempo_ref", "tempo_viagem", "posicao", "entrada", "prazo_controle"};
    return (unsigned)f < sizeof names / sizeof names[0] ? names[f] : "invalida";
}
