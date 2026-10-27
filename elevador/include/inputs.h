#ifndef EL_INPUTS_H
#define EL_INPUTS_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint8_t floors; /* active-high normalized bits 0..2 */
    bool lower_limit, upper_limit, stop, button, valid;
    uint16_t joystick;
} RawInputs;
typedef struct { bool stable, candidate; uint32_t since; } Debounce;
typedef struct {
    uint8_t floors;
    bool lower_limit, upper_limit, stop, valid, coherent, ready;
} Sensors;
typedef enum { EV_NONE, EV_UP, EV_DOWN, EV_CONFIRM, EV_LONG } InputEvent;
typedef struct {
    Debounce floors[3], button;
    uint32_t boot_ms, pressed_ms;
    int32_t filtered;
    bool centered, released, pressing;
} Inputs;
void inputs_init(Inputs *i, uint32_t now);
InputEvent inputs_step(Inputs *i, const RawInputs *r, uint32_t now, Sensors *s);
#endif
