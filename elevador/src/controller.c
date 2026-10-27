#include "controller.h"
void controller_init(Controller *c, uint32_t now) {
    elevator_init(&c->elevator, now); inputs_init(&c->inputs, now);
}
void controller_step(Controller *c, const RawInputs *raw, uint32_t now) {
    Sensors sensors;
    ElState previous = c->elevator.state;
    InputEvent event = inputs_step(&c->inputs, raw, now, &sensors);
    elevator_step(&c->elevator, &sensors, event, now);
    if (previous != c->elevator.state) {
        /* A press begun during travel or before a fault must never become an
         * idle request or a fault acknowledgement when released in a new state. */
        c->inputs.pressing = false;
        c->inputs.released = false;
    }
}
