#ifndef JOY_INPUT_H
#define JOY_INPUT_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
    uint32_t sum_x, sum_y;
    uint16_t count, min_x, min_y, max_x, max_y, center_x, center_y;
    int32_t filtered_x, filtered_y;
    uint8_t button_count;
    bool calibrated, stable_button, candidate_button, button_armed;
} Joy;
typedef struct { int16_t x, y; bool calibrated, click; } JoyResult;
void Joy_Init(Joy *joy);
JoyResult Joy_Update(Joy *joy, uint16_t x, uint16_t y, bool button, bool valid);
#endif
