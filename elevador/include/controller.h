#ifndef EL_CONTROLLER_H
#define EL_CONTROLLER_H
#include "elevator.h"
typedef struct { Elevator elevator; Inputs inputs; } Controller;
void controller_init(Controller *c, uint32_t now);
void controller_step(Controller *c, const RawInputs *raw, uint32_t now);
#endif
