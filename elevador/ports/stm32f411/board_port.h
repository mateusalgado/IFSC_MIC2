#ifndef EL_BOARD_PORT_H
#define EL_BOARD_PORT_H
#include "inputs.h"
#include "motor.h"
#include "elevator.h"
/* These are contracts, not a completed physical board driver. See docs/INTEGRACAO.md. */
void board_safe_init(void);
RawInputs board_read_inputs(void); /* bounded, nonblocking, coherent ADC/GPIO sample */
void board_motor_write(MotorOutput output); /* ControlTask only */
void board_indicators(ElState state); /* ControlTask only */
void board_lcd_write(const char *row0, const char *row1); /* DisplayTask only */
void board_fault_disable(void); /* startup/panic, never a normal second motor owner */
void board_watchdog_refresh(void); /* only after successful control iteration */
#endif
