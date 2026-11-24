/* Trata o joystick sem depender de HAL ou FreeRTOS: facilita testar no computador. */
#include "joy_input.h"
#include "game_config.h"
#include <string.h>
#include <stdlib.h>

#define ADC_MAX 4095
#define CALIBRATION_SAMPLES 100U
#define CALIBRATION_MAX_VARIATION 160
#define CALIBRATION_MIN_CENTER 1024U
#define CALIBRATION_MAX_CENTER 3072U
#define BUTTON_STABLE_SAMPLES 3U
#define AXIS_SCALE 1000
#define FILTER_SCALE 4

static void ResetCalibration(Joy *joy)
{
    joy->count = 0;
    joy->sum_x = 0;
    joy->sum_y = 0;
    joy->min_x = ADC_MAX;
    joy->min_y = ADC_MAX;
    joy->max_x = 0;
    joy->max_y = 0;
}

void Joy_Init(Joy *joy)
{
    memset(joy, 0, sizeof(*joy));
    ResetCalibration(joy);
}

static bool UpdateButton(Joy *joy, bool pressed)
{
    bool click = false;
    if (pressed != joy->candidate_button)
    {
        joy->candidate_button = pressed;
        joy->button_count = 0;
    }
    else if (joy->button_count < BUTTON_STABLE_SAMPLES)
    {
        ++joy->button_count;
    }
    /* Tres periodos depois da mudanca: 30 ms com entrada a cada 10 ms. */
    if (joy->button_count >= BUTTON_STABLE_SAMPLES)
    {
        if (!pressed)
        {
            joy->button_armed = true;
        }
        if (pressed && !joy->stable_button && joy->button_armed)
        {
            click = true;
            joy->button_armed = false;
        }
        joy->stable_button = pressed;
    }
    return click;
}

static void UpdateCalibration(Joy *joy, uint16_t x, uint16_t y)
{
    joy->sum_x += x;
    joy->sum_y += y;
    ++joy->count;
    if (x < joy->min_x)
    {
        joy->min_x = x;
    }
    if (y < joy->min_y)
    {
        joy->min_y = y;
    }
    if (x > joy->max_x)
    {
        joy->max_x = x;
    }
    if (y > joy->max_y)
    {
        joy->max_y = y;
    }
    if (joy->count != CALIBRATION_SAMPLES)
    {
        return;
    }

    bool stable = joy->max_x - joy->min_x <= CALIBRATION_MAX_VARIATION &&
                  joy->max_y - joy->min_y <= CALIBRATION_MAX_VARIATION;
    bool centered = joy->min_x >= CALIBRATION_MIN_CENTER && joy->max_x <= CALIBRATION_MAX_CENTER &&
                    joy->min_y >= CALIBRATION_MIN_CENTER && joy->max_y <= CALIBRATION_MAX_CENTER;
    if (stable && centered)
    {
        joy->center_x = (uint16_t)(joy->sum_x / CALIBRATION_SAMPLES);
        joy->center_y = (uint16_t)(joy->sum_y / CALIBRATION_SAMPLES);
        joy->filtered_x = joy->center_x * FILTER_SCALE;
        joy->filtered_y = joy->center_y * FILTER_SCALE;
        joy->calibrated = true;
    }
    else
    {
        /* Joystick se moveu ou estava inclinado: repetir a janela de um segundo. */
        ResetCalibration(joy);
    }
}

static int16_t NormalizeAxis(uint16_t raw, uint16_t center, int32_t *filtered)
{
    int delta = (int)raw - center;
    int range = delta >= 0 ? ADC_MAX - center : center;
    int dead_zone = range / 10;
    if (abs(delta) <= dead_zone)
    {
        /* Parar imediatamente no centro; evita arrastar o ponto depois de soltar. */
        *filtered = (int32_t)center * FILTER_SCALE;
        return 0;
    }
    /* Media exponencial em inteiros, mantendo duas casas binarias de fracao. */
    *filtered += ((int32_t)raw * FILTER_SCALE - *filtered) / FILTER_SCALE;
    delta = *filtered / FILTER_SCALE - center;
    range = delta >= 0 ? ADC_MAX - center : center;
    dead_zone = range / 10;
    int magnitude = abs(delta) - dead_zone;
    if (magnitude <= 0)
    {
        return 0;
    }
    int value = magnitude * AXIS_SCALE / (range - dead_zone);
    if (value > AXIS_SCALE)
    {
        value = AXIS_SCALE;
    }
    if (delta < 0)
    {
        value = -value;
    }
    return (int16_t)value;
}

JoyResult Joy_Update(Joy *joy, uint16_t x, uint16_t y, bool button, bool valid)
{
    JoyResult result = {0};
    if (!valid)
    {
        if (!joy->calibrated)
        {
            ResetCalibration(joy);
        }
        return result;
    }
    result.click = UpdateButton(joy, button);
    if (!joy->calibrated)
    {
        UpdateCalibration(joy, x, y);
        result.click = false; /* Nao confirmar menus durante a calibracao. */
    }
    result.calibrated = joy->calibrated;
    if (joy->calibrated)
    {
        result.x = NormalizeAxis(x, joy->center_x, &joy->filtered_x);
        result.y = NormalizeAxis(y, joy->center_y, &joy->filtered_y);
        if (GAME_INVERT_X)
        {
            result.x = -result.x;
        }
        if (GAME_INVERT_Y)
        {
            result.y = -result.y;
        }
    }
    return result;
}
