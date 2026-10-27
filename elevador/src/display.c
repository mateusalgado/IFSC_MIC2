#include "display.h"
#include <stdio.h>
#include <string.h>
static void line(char dst[17], const char *src) {
    size_t n = strlen(src); if (n > 16) n = 16;
    memset(dst, ' ', 16); memcpy(dst, src, n); dst[16] = '\0';
}
void display_format(const Elevator *e, uint32_t now, char lines[2][17]) {
    char a[64], b[64];
    char last = e->last_floor < 0 ? '?' : (char)('0' + e->last_floor);
    char dest = e->destination < 0 ? '-' : (char)('0' + e->destination);
    snprintf(a, sizeof a, "Ult:%c D:%c Sel:%d", last, dest, e->selection);
    snprintf(b, sizeof b, "%s", elevator_state_name(e->state));
    if (e->state == EL_INIT) snprintf(b, sizeof b, "Segure p/ refer.");
    if (e->state == EL_UP || e->state == EL_DOWN || e->state == EL_HOME) {
        if (e->current_floor < 0 && (now / 1000u) % 2u == 0) snprintf(a, sizeof a, "entre andares");
        snprintf(b, sizeof b, "%s D:%c PWM %u%%", e->state == EL_UP ? "SUB" : e->state == EL_DOWN ? "DES" : "REF", dest, e->motor.out.pwm / 10u);
    }
    if (e->notice == NOTICE_BUSY) snprintf(b, sizeof b, "Pedido recusado");
    if (e->notice == NOTICE_SAME_FLOOR) snprintf(b, sizeof b, "Ja neste andar");
    if (e->notice == NOTICE_NEED_HOME) snprintf(b, sizeof b, "Faca referencia");
    if (e->state == EL_FAULT) {
        snprintf(a, sizeof a, "FALHA %s", elevator_fault_name(e->fault));
        snprintf(b, sizeof b, "Segure p/ reconh.");
    }
    line(lines[0], a); line(lines[1], b);
}
