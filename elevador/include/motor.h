#ifndef EL_MOTOR_H
#define EL_MOTOR_H
#include <stdint.h>
typedef enum { MOTOR_NORMAL, MOTOR_REVERSAL, MOTOR_FAULT } MotorStop;
typedef struct {
    int direction; /* -1 down, 0 inactive, +1 up */
    uint16_t pwm;
    MotorStop stop;
} MotorOutput;
typedef struct {
    MotorOutput out;
    int last_direction;
    uint32_t last_ms, zero_since, fraction;
} Motor;
void motor_init(Motor *m, uint32_t now);
void motor_stop(Motor *m, uint32_t now, MotorStop reason);
void motor_step(Motor *m, uint32_t now, int direction, uint16_t command);
#endif
