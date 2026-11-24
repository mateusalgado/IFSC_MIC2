#ifndef JOY_INPUT_H
#define JOY_INPUT_H
#include <stdbool.h>
#include <stdint.h>
/* Memoria da calibracao, filtro e debounce de um joystick. */
typedef struct
{
    uint32_t sum_x;
    uint32_t sum_y;
    uint16_t count;
    uint16_t min_x;
    uint16_t min_y;
    uint16_t max_x;
    uint16_t max_y;
    uint16_t center_x;
    uint16_t center_y;
    int32_t filtered_x;
    int32_t filtered_y;
    uint8_t button_count;
    bool calibrated;
    bool stable_button;
    bool candidate_button;
    bool button_armed;
} Joy;
typedef struct
{
    int16_t x; /* -1000..1000; zero dentro da zona morta. */
    int16_t y;
    bool calibrated;
    bool click; /* Um evento por pressionamento, depois do debounce. */
} JoyResult;
void Joy_Init(Joy *joy);
JoyResult Joy_Update(Joy *joy, uint16_t x, uint16_t y, bool button, bool valid);
#endif
