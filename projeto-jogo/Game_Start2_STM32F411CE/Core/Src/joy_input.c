#include "joy_input.h"
#include "game_config.h"
#include <string.h>
#include <stdlib.h>

static void window(Joy *j)
{
    j->count = 0; j->sum_x = j->sum_y = 0;
    j->min_x = j->min_y = 4095; j->max_x = j->max_y = 0;
}
void Joy_Init(Joy *j) { memset(j, 0, sizeof(*j)); window(j); }

static int16_t axis(uint16_t raw, uint16_t center, int32_t *filtered)
{
    int delta = (int)raw - center;
    int range = delta >= 0 ? 4095 - center : center;
    int dead = range / 10;
    if (abs(delta) <= dead) { *filtered = (int32_t)center * 4; return 0; }
    *filtered += ((int32_t)raw * 4 - *filtered) / 4;
    delta = *filtered / 4 - center;
    range = delta >= 0 ? 4095 - center : center;
    dead = range / 10;
    int magnitude = abs(delta) - dead;
    if (magnitude <= 0) return 0;
    int value = magnitude * 1000 / (range - dead);
    if (value > 1000) value = 1000;
    return (int16_t)(delta < 0 ? -value : value);
}

JoyResult Joy_Update(Joy *j, uint16_t x, uint16_t y, bool button, bool valid)
{
    JoyResult result = {0};
    if (!valid) { if (!j->calibrated) window(j); return result; }
    if (button != j->candidate_button) { j->candidate_button = button; j->button_count = 0; }
    else if (j->button_count < 3) ++j->button_count;
    if (j->button_count >= 3) {
        if (!button) j->button_armed = true;
        if (button && !j->stable_button && j->button_armed) {
            result.click = true; j->button_armed = false;
        }
        j->stable_button = button;
    }
    if (!j->calibrated) {
        j->sum_x += x; j->sum_y += y; ++j->count;
        if (x < j->min_x) j->min_x = x;
        if (y < j->min_y) j->min_y = y;
        if (x > j->max_x) j->max_x = x;
        if (y > j->max_y) j->max_y = y;
        if (j->count == 100) {
            if (j->max_x - j->min_x <= 160 && j->max_y - j->min_y <= 160 &&
                j->min_x >= 1024 && j->max_x <= 3072 && j->min_y >= 1024 && j->max_y <= 3072) {
                j->center_x = (uint16_t)(j->sum_x / 100); j->center_y = (uint16_t)(j->sum_y / 100);
                j->filtered_x = j->center_x * 4; j->filtered_y = j->center_y * 4;
                j->calibrated = true;
            } else window(j);
        }
        result.click = false;
    }
    result.calibrated = j->calibrated;
    if (j->calibrated) {
        result.x = axis(x, j->center_x, &j->filtered_x);
        result.y = axis(y, j->center_y, &j->filtered_y);
        if (GAME_INVERT_X) result.x = -result.x;
        if (GAME_INVERT_Y) result.y = -result.y;
    }
    return result;
}
