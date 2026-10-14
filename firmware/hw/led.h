#ifndef U4RK_LED_H
#define U4RK_LED_H

#include <stdbool.h>

/* LP5562 RGB status LED on I2C0 (GPIO24/25), one colour per pipeline stage,
 * dimmed to U4RK_LED_LEVEL_PERCENT. All calls run on core 0 only. */
typedef enum {
    U4RK_LED_OFF = 0,
    U4RK_LED_BOOT,      /* white:   initialising */
    U4RK_LED_IDLE,      /* green:   ready, nothing in flight */
    U4RK_LED_CAPTURE,   /* yellow:  ADC capture (and pulse) running */
    U4RK_LED_DSP,       /* blue:    core 1 processing a frame */
    U4RK_LED_USB,       /* magenta: frame queued for / being sent over USB */
    U4RK_LED_FAULT,     /* red:     DMA fault (held briefly) */
    U4RK_LED_STAGE_COUNT,
} u4rk_led_stage_t;

/* Initialise I2C0 and the LP5562. Returns false (and the LED stays unused) if
 * the chip does not answer or the register read-back fails. */
bool u4rk_led_init(void);
bool u4rk_led_present(void);
/* Show a stage on the RGB LED. Cheap when the stage is unchanged; otherwise
 * one I2C write (~0.5 ms at 100 kHz). */
void u4rk_led_set_stage(u4rk_led_stage_t stage);
/* W channel (LED2): on while a host has the ttyACM open, off when it drops.
 * Cheap when unchanged. */
void u4rk_led_set_host_connected(bool connected);
const char *u4rk_led_stage_name(u4rk_led_stage_t stage);

#endif
