#include "elevator.h"
#include "display.h"
#include "controller.h"
#include "elevator_config.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Elevator e;
static Sensors s;
static uint32_t now;
static unsigned tests;
static void tick(InputEvent event) {
    now += EL_CONTROL_MS; elevator_step(&e, &s, event, now);
    assert(e.motor.out.pwm <= EL_PWM_MAX);
    if (e.state == EL_INIT || e.state == EL_IDLE || e.state == EL_ARRIVAL || e.state == EL_FAULT)
        assert(e.motor.out.pwm == 0 && e.motor.out.direction == 0);
}
static void wait_ms(uint32_t ms) { for (uint32_t n = 0; n < ms; n += EL_CONTROL_MS) tick(EV_NONE); }
static void fresh(void) {
    now = 0; elevator_init(&e, now);
    s = (Sensors){.valid = true, .coherent = true, .ready = true};
}
static void home(void) {
    fresh(); tick(EV_LONG); assert(e.state == EL_HOME);
    wait_ms(500); assert(e.motor.out.direction == -1);
    s.floors = 1; tick(EV_NONE); assert(e.state == EL_ARRIVAL);
    wait_ms(EL_ARRIVAL_MS); assert(e.state == EL_IDLE && e.referenced);
}
static void to_second(void) {
    home(); tick(EV_UP); tick(EV_UP); tick(EV_CONFIRM);
    assert(e.state == EL_UP); s.floors = 0; wait_ms(500);
    s.floors = 2; tick(EV_NONE); s.floors = 0; wait_ms(500);
    s.floors = 4; tick(EV_NONE); assert(e.state == EL_ARRIVAL);
    wait_ms(EL_ARRIVAL_MS); assert(e.last_floor == 2 && e.state == EL_IDLE);
}
static void test_boot(void) {
    fresh(); wait_ms(1000); assert(e.state == EL_INIT && !e.referenced);
    tick(EV_CONFIRM); assert(e.state == EL_INIT && e.notice == NOTICE_NEED_HOME);
    s.floors = 4; tick(EV_NONE); assert(!e.referenced);
}
static void test_home(void) { home(); assert(e.last_floor == 0); }
static void test_existing_ground(void) {
    fresh(); s.floors = 1; tick(EV_LONG); tick(EV_NONE);
    assert(e.state == EL_ARRIVAL && e.motor.out.pwm == 0);
}
static void test_both_directions(void) {
    to_second(); tick(EV_DOWN); tick(EV_DOWN); tick(EV_CONFIRM);
    assert(e.state == EL_DOWN); s.floors = 0; wait_ms(500);
    assert(e.motor.out.direction == -1);
    s.floors = 2; tick(EV_NONE); s.floors = 0; wait_ms(500);
    s.floors = 1; tick(EV_NONE); wait_ms(EL_ARRIVAL_MS);
    assert(e.state == EL_IDLE && e.last_floor == 0);
}
static void test_same_floor(void) { home(); tick(EV_CONFIRM); assert(e.notice == NOTICE_SAME_FLOOR && e.state == EL_IDLE); }
static void test_busy(void) {
    home(); tick(EV_UP); tick(EV_CONFIRM); s.floors = 0; wait_ms(500);
    tick(EV_DOWN); tick(EV_CONFIRM); tick(EV_LONG);
    assert(e.state == EL_UP && e.destination == 1 && e.notice == NOTICE_BUSY);
    s.floors = 2; tick(EV_CONFIRM); wait_ms(EL_ARRIVAL_MS + 100);
    assert(e.state == EL_IDLE && e.destination == -1);
}
static void test_home_timeout(void) {
    fresh(); tick(EV_LONG); wait_ms(EL_HOME_TIMEOUT_MS);
    assert(e.state == EL_FAULT && e.fault == FAULT_HOME_TIMEOUT);
}
static void test_travel_timeout(void) {
    home(); tick(EV_UP); tick(EV_CONFIRM); s.floors = 0; wait_ms(EL_TRAVEL_TIMEOUT_MS);
    assert(e.state == EL_FAULT && e.fault == FAULT_TRAVEL_TIMEOUT);
}
static void test_sensors(void) {
    home(); s.floors = 3; tick(EV_CONFIRM); assert(e.fault == FAULT_SENSOR);
    home(); tick(EV_UP); tick(EV_UP); tick(EV_CONFIRM); s.floors = 4;
    tick(EV_NONE); assert(e.fault == FAULT_SENSOR); /* missing intermediate floor */
    home(); tick(EV_UP); tick(EV_CONFIRM); wait_ms(EL_DEPART_TIMEOUT_MS);
    assert(e.fault == FAULT_SENSOR);
}
static void test_limits(void) {
    for (unsigned limit = 0; limit < 2; ++limit) {
        home(); tick(EV_UP); tick(EV_CONFIRM); s.floors = 0; wait_ms(500);
        s.lower_limit = limit == 0; s.upper_limit = limit == 1;
        tick(EV_CONFIRM); assert(e.fault == FAULT_LIMIT && e.motor.out.pwm == 0);
        tick(EV_LONG); assert(e.state == EL_FAULT);
    }
    fresh(); s.lower_limit = true; s.upper_limit = true; tick(EV_LONG); assert(e.state == EL_FAULT);
}
static void test_stop_recovery(void) {
    home(); tick(EV_UP); tick(EV_CONFIRM); s.floors = 0; wait_ms(500);
    s.stop = true; tick(EV_CONFIRM); assert(e.fault == FAULT_STOP && !e.referenced);
    tick(EV_LONG); assert(e.state == EL_FAULT);
    s.stop = false; tick(EV_CONFIRM); assert(e.state == EL_FAULT);
    tick(EV_LONG); assert(e.state == EL_INIT && e.motor.out.pwm == 0);
    wait_ms(500); tick(EV_CONFIRM); assert(e.state == EL_INIT);
    tick(EV_LONG); assert(e.state == EL_HOME);
}
static void test_reset(void) {
    home(); tick(EV_UP); tick(EV_CONFIRM); s.floors = 0; wait_ms(500);
    elevator_init(&e, now); wait_ms(500);
    assert(e.state == EL_INIT && !e.referenced && e.last_floor == -1);
}
static void test_position_loss(void) { home(); s.floors = 0; tick(EV_CONFIRM); assert(e.fault == FAULT_POSITION); }
static void test_deadline(void) { home(); now += 100; tick(EV_UP); assert(e.fault == FAULT_DEADLINE); }
static void test_invalid_sample(void) { home(); s.valid = false; tick(EV_UP); assert(e.fault == FAULT_INPUT); }
static void test_wrap(void) {
    fresh(); now = UINT32_MAX - 200; elevator_init(&e, now);
    tick(EV_LONG); wait_ms(500); assert(e.state == EL_HOME && e.motor.out.direction == -1);
    wait_ms(EL_HOME_TIMEOUT_MS); assert(e.fault == FAULT_HOME_TIMEOUT);
}
static void test_motor(void) {
    Motor m; motor_init(&m, 0);
    for (uint32_t t = 10; t <= 2000; t += 10) motor_step(&m, t, 1, 1000);
    assert(m.out.pwm == EL_PWM_MAX && m.out.direction == 1);
    uint32_t t = 2000, zero = 0;
    do {
        t += 10; motor_step(&m, t, -1, EL_PWM_TRAVEL);
        if (m.out.pwm) assert(m.out.direction == 1);
    } while (m.out.pwm);
    zero = t;
    while (t + 10 < zero + EL_REVERSE_MS) { t += 10; motor_step(&m, t, -1, EL_PWM_TRAVEL); assert(m.out.pwm == 0); }
    t += 10; motor_step(&m, t, -1, EL_PWM_TRAVEL); assert(m.out.direction == -1 && m.out.pwm <= 2);
    motor_stop(&m, t, MOTOR_FAULT); assert(m.out.pwm == 0 && m.out.stop == MOTOR_FAULT);
    motor_step(&m, t + 10, 1, 250); assert(m.out.pwm == 0);
}
static RawInputs neutral(void) { return (RawInputs){.valid = true, .joystick = EL_ADC_CENTER}; }
static void test_joystick(void) {
    Inputs i; Sensors out; RawInputs r = neutral(); inputs_init(&i, 0);
    unsigned count = 0; uint32_t t;
    for (t = 0; t < 1000; t += 10) assert(inputs_step(&i, &r, t, &out) == EV_NONE);
    r.joystick = 4095;
    for (; t < 2000; t += 10) if (inputs_step(&i, &r, t, &out) == EV_UP) ++count;
    assert(count == 1);
    r.joystick = 0;
    for (; t < 2500; t += 10) assert(inputs_step(&i, &r, t, &out) == EV_NONE);
    r.joystick = EL_ADC_CENTER;
    for (; t < 3000; t += 10) (void)inputs_step(&i, &r, t, &out);
    r.joystick = 0;
    for (; t < 4000; t += 10) if (inputs_step(&i, &r, t, &out) == EV_DOWN) ++count;
    assert(count == 2);
}
static void test_debounce(void) {
    Inputs i; Sensors out; RawInputs r = neutral(); inputs_init(&i, 0);
    (void)inputs_step(&i, &r, 100, &out);
    r.floors = 1; (void)inputs_step(&i, &r, 110, &out); assert(out.floors == 0);
    r.floors = 0; (void)inputs_step(&i, &r, 120, &out);
    r.floors = 1; (void)inputs_step(&i, &r, 130, &out);
    (void)inputs_step(&i, &r, 160, &out); assert(out.floors == 1);
    r.floors = 3; r.stop = true; (void)inputs_step(&i, &r, 170, &out);
    assert(!out.coherent && out.stop);
}
static void test_button(void) {
    Inputs i; Sensors out; RawInputs r = neutral(); inputs_init(&i, 0);
    r.button = true;
    for (uint32_t t = 0; t <= 1500; t += 10) assert(inputs_step(&i, &r, t, &out) == EV_NONE);
    r.button = false;
    for (uint32_t t = 1510; t <= 1600; t += 10) assert(inputs_step(&i, &r, t, &out) == EV_NONE);
    r.button = true;
    for (uint32_t t = 1610; t <= 1710; t += 10) assert(inputs_step(&i, &r, t, &out) == EV_NONE);
    r.button = false;
    (void)inputs_step(&i, &r, 1720, &out);
    assert(inputs_step(&i, &r, 1750, &out) == EV_CONFIRM);
    r.button = true; (void)inputs_step(&i, &r, 1800, &out); (void)inputs_step(&i, &r, 1830, &out);
    assert(inputs_step(&i, &r, 3000, &out) == EV_NONE);
    r.button = false; (void)inputs_step(&i, &r, 3010, &out);
    assert(inputs_step(&i, &r, 3040, &out) == EV_LONG);
}
static void test_display(void) {
    char lines[2][17]; home(); tick(EV_UP); tick(EV_CONFIRM); s.floors = 0; tick(EV_NONE);
    display_format(&e, 0, lines); assert(strstr(lines[0], "entre andares") && strstr(lines[1], "PWM"));
    display_format(&e, 1000, lines); assert(strstr(lines[0], "Ult:0 D:1"));
    assert(strlen(lines[0]) == 16 && strlen(lines[1]) == 16);
}
static void test_safety_sweep(void) {
    for (int st = EL_INIT; st < EL_STATE_COUNT; ++st) {
        for (int event = EV_NONE; event <= EV_LONG; ++event) {
            fresh(); e.state = (ElState)st; e.motor.out = (MotorOutput){1, 200, MOTOR_NORMAL};
            s.stop = true; tick((InputEvent)event);
            assert(e.state == EL_FAULT && e.motor.out.pwm == 0);
        }
    }
}
static void raw_wait(Controller *c, RawInputs *raw, uint32_t *time, uint32_t duration) {
    for (uint32_t j = 0; j < duration; j += 10) {
        *time += 10; controller_step(c, raw, *time);
    }
}
static void raw_press(Controller *c, RawInputs *raw, uint32_t *time, uint32_t duration) {
    raw->button = true; raw_wait(c, raw, time, duration);
    raw->button = false; raw_wait(c, raw, time, 100);
}
static void test_integrated_raw_trip(void) {
    Controller c; RawInputs raw = neutral(); uint32_t time = 0;
    controller_init(&c, time); raw_wait(&c, &raw, &time, 200);
    raw_press(&c, &raw, &time, 1100); assert(c.elevator.state == EL_HOME);
    raw_wait(&c, &raw, &time, 500); raw.floors = 1;
    raw_wait(&c, &raw, &time, 500); assert(c.elevator.state == EL_IDLE);
    raw.joystick = 4095; raw_wait(&c, &raw, &time, 200);
    raw.joystick = 2048; raw_wait(&c, &raw, &time, 200);
    raw_press(&c, &raw, &time, 100); assert(c.elevator.state == EL_UP);
    raw.floors = 0; raw_wait(&c, &raw, &time, 500);
    raw.button = true; raw_wait(&c, &raw, &time, 100);
    raw.floors = 2; raw_wait(&c, &raw, &time, 500);
    raw.button = false; raw_wait(&c, &raw, &time, 100);
    assert(c.elevator.state == EL_IDLE && c.elevator.notice == NOTICE_NONE);
}
static void test_fault_requires_new_press(void) {
    Controller c; RawInputs raw = neutral(); uint32_t time = 0;
    controller_init(&c, time); raw_wait(&c, &raw, &time, 200);
    raw.button = true; raw_wait(&c, &raw, &time, 1100);
    raw.stop = true; raw_wait(&c, &raw, &time, 10);
    raw.stop = false; raw.button = false; raw_wait(&c, &raw, &time, 100);
    assert(c.elevator.state == EL_FAULT);
    raw_press(&c, &raw, &time, 1100); assert(c.elevator.state == EL_INIT);
}
#define RUN(fn) do { fn(); ++tests; puts("PASS " #fn); } while (0)
int main(void) {
    RUN(test_boot); RUN(test_home); RUN(test_existing_ground); RUN(test_both_directions);
    RUN(test_same_floor); RUN(test_busy); RUN(test_home_timeout); RUN(test_travel_timeout);
    RUN(test_sensors); RUN(test_limits); RUN(test_stop_recovery); RUN(test_reset);
    RUN(test_position_loss); RUN(test_deadline); RUN(test_invalid_sample); RUN(test_wrap);
    RUN(test_motor); RUN(test_joystick); RUN(test_debounce); RUN(test_button);
    RUN(test_display); RUN(test_safety_sweep); RUN(test_integrated_raw_trip);
    RUN(test_fault_requires_new_press);
    printf("%u test groups passed\n", tests); return 0;
}
