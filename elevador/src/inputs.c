#include "inputs.h"
#include "elevator_config.h"
#include <string.h>
static bool debounce(Debounce *d, bool raw, uint32_t now) {
    if (raw != d->candidate) { d->candidate = raw; d->since = now; }
    if ((uint32_t)(now - d->since) >= EL_DEBOUNCE_MS) d->stable = raw;
    return d->stable;
}
void inputs_init(Inputs *i, uint32_t now) {
    memset(i, 0, sizeof *i);
    i->boot_ms = now;
    i->filtered = EL_ADC_CENTER;
    for (unsigned n = 0; n < 3; ++n) i->floors[n].since = now;
    i->button.since = now;
}
InputEvent inputs_step(Inputs *i, const RawInputs *r, uint32_t now, Sensors *s) {
    *s = (Sensors){0};
    s->valid = r->valid && r->joystick <= 4095;
    s->coherent = r->floors <= 7 && !(r->floors & (r->floors - 1u));
    /* Safety assertions bypass debounce; a fault latches any sampled pulse. */
    s->stop = r->stop; s->lower_limit = r->lower_limit; s->upper_limit = r->upper_limit;
    s->ready = (uint32_t)(now - i->boot_ms) >= EL_SETTLE_MS;
    for (unsigned n = 0; n < 3; ++n)
        if (debounce(&i->floors[n], (r->floors & (1u << n)) != 0, now))
            s->floors |= (uint8_t)(1u << n);
    bool old = i->button.stable;
    bool pressed = debounce(&i->button, r->button, now);
    if (!r->button && !pressed && s->ready) i->released = true;
    InputEvent event = EV_NONE;
    if (pressed && !old && i->released) {
        i->pressed_ms = now; i->pressing = true; i->released = false;
    }
    if (!pressed && old && i->pressing) {
        event = (uint32_t)(now - i->pressed_ms) >= EL_LONG_PRESS_MS ? EV_LONG : EV_CONFIRM;
        i->pressing = false;
    }
    i->filtered += ((int32_t)r->joystick - i->filtered) / 4;
    int delta = (int)i->filtered - EL_ADC_CENTER;
    if (delta >= -EL_ADC_DEADZONE && delta <= EL_ADC_DEADZONE &&
        r->joystick >= EL_ADC_CENTER - EL_ADC_DEADZONE &&
        r->joystick <= EL_ADC_CENTER + EL_ADC_DEADZONE) i->centered = true;
    if (i->centered && (delta > EL_ADC_TRIGGER || delta < -EL_ADC_TRIGGER)) {
        i->centered = false;
        if (event == EV_NONE) event = delta > 0 ? EV_UP : EV_DOWN;
    }
    return s->ready && s->valid ? event : EV_NONE;
}
