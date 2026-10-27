#include "motor.h"
#include "elevator_config.h"
#include <string.h>
void motor_init(Motor *m, uint32_t now) {
    memset(m, 0, sizeof *m);
    m->last_ms = m->zero_since = now;
}
void motor_stop(Motor *m, uint32_t now, MotorStop reason) {
    if (m->out.pwm != 0) m->zero_since = now;
    m->out = (MotorOutput){0, 0, reason};
    m->last_ms = now;
    m->fraction = 0;
}
void motor_step(Motor *m, uint32_t now, int direction, uint16_t command) {
    uint32_t dt = now - m->last_ms;
    m->last_ms = now;
    if (dt > EL_MAX_GAP_MS) dt = EL_MAX_GAP_MS;
    if (direction < -1 || direction > 1) {
        motor_stop(m, now, MOTOR_FAULT); return;
    }
    if (direction == 0 || command == 0) {
        motor_stop(m, now, MOTOR_NORMAL); return;
    }
    if (command > EL_PWM_MAX) command = EL_PWM_MAX;
    uint32_t budget = dt * EL_RAMP_PER_SECOND + m->fraction;
    uint32_t step = budget / 1000u;
    m->fraction = budget % 1000u;
    if (m->out.pwm && direction != m->out.direction) {
        m->out.stop = MOTOR_REVERSAL;
        if (step >= m->out.pwm) motor_stop(m, now, MOTOR_REVERSAL);
        else m->out.pwm = (uint16_t)(m->out.pwm - step);
        return;
    }
    if (!m->out.pwm && direction != m->last_direction &&
        (uint32_t)(now - m->zero_since) < EL_REVERSE_MS) {
        m->out.stop = MOTOR_REVERSAL;
        m->fraction = 0;
        return;
    }
    m->out.stop = MOTOR_NORMAL;
    if (m->out.pwm < command) {
        uint32_t next = m->out.pwm + step;
        m->out.pwm = (uint16_t)(next > command ? command : next);
    } else if (m->out.pwm > command) {
        uint32_t diff = m->out.pwm - command;
        m->out.pwm = (uint16_t)(m->out.pwm - (step > diff ? diff : step));
    }
    if (m->out.pwm) {
        m->out.direction = direction;
        m->last_direction = direction;
    }
}
