#ifndef ELEVATOR_CONFIG_H
#define ELEVATOR_CONFIG_H
/* Provisional simulation values. Recalibrate before energizing a real motor. */
#define EL_CONTROL_MS 10u
#define EL_MAX_GAP_MS 50u
#define EL_DEBOUNCE_MS 30u
#define EL_SETTLE_MS 100u
#define EL_LONG_PRESS_MS 1000u
#define EL_HOME_TIMEOUT_MS 15000u
#define EL_TRAVEL_TIMEOUT_MS 20000u
#define EL_DEPART_TIMEOUT_MS 2000u
#define EL_ARRIVAL_MS 300u
#define EL_NOTICE_MS 1500u
#define EL_PWM_MAX 300u /* permille, command only; NOT measured speed */
#define EL_PWM_HOME 150u
#define EL_PWM_TRAVEL 250u
#define EL_RAMP_PER_SECOND 200u
#define EL_REVERSE_MS 300u
#define EL_ADC_CENTER 2048
#define EL_ADC_DEADZONE 250
#define EL_ADC_TRIGGER 700
#define EL_LCD_MS 100u
#define EL_LOG_QUEUE_LENGTH 16u
#define EL_LOG_PERIOD_MS 1000u
#if EL_PWM_HOME > EL_PWM_MAX || EL_PWM_TRAVEL > EL_PWM_MAX || EL_PWM_MAX > 1000
#error Invalid provisional motor limits
#endif
#endif
