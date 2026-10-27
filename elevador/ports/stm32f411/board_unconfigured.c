#include "board_port.h"
/* Deliberately no GPIO/PWM mapping until board, bridge and mechanics are confirmed.
 * The invalid sample makes the application latch FAULT_INPUT. Replacing only a
 * macro cannot accidentally enable the motor. Replace this file with real drivers.
 */
void board_safe_init(void) { }
RawInputs board_read_inputs(void) {
    return (RawInputs){.joystick = 2048, .valid = false};
}
void board_motor_write(MotorOutput output) { (void)output; }
void board_indicators(ElState state) { (void)state; }
void board_lcd_write(const char *row0, const char *row1) { (void)row0; (void)row1; }
void board_fault_disable(void) { }
void board_watchdog_refresh(void) { }
