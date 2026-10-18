// SPDX-License-Identifier: GPL-3.0-or-later
#include "led.h"

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "u4rk.h"

/* LP5562, ADDR_SEL0/1 tied to GND. */
#define LP5562_ADDR        0x30u
#define LP5562_ENABLE      0x00u
#define LP5562_OP_MODE     0x01u
#define LP5562_B_PWM       0x02u   /* B, G, R PWM are consecutive (auto-increment) */
#define LP5562_CONFIG      0x08u
#define LP5562_RESET       0x0Du
#define LP5562_W_PWM       0x0Eu
#define LP5562_LED_MAP     0x70u

#define LP5562_ENABLE_CHIP_EN   0x40u   /* LOG_EN clear: linear PWM */
#define LP5562_CONFIG_INT_CLK   0x01u
#define LP5562_LED_MAP_I2C      0x00u   /* all channels driven by the PWM registers */

/* The v2 board has no external I2C pull-ups: run slowly on the internal ones. */
#define U4RK_LED_I2C_HZ        100000u
#define U4RK_LED_I2C_TIMEOUT_US 2000u
/* Full-scale PWM, scaled to the configured light level (5% of 255 = 13). */
#define U4RK_LED_MAX_PWM \
    ((255u * U4RK_LED_LEVEL_PERCENT + 50u) / 100u)

typedef struct {
    uint8_t r, g, b;
} rgb_t;

static const rgb_t stage_colours[U4RK_LED_STAGE_COUNT] = {
    [U4RK_LED_OFF]     = {0, 0, 0},
    [U4RK_LED_BOOT]    = {255, 255, 255},
    [U4RK_LED_IDLE]    = {0, 255, 0},
    [U4RK_LED_CAPTURE] = {255, 160, 0},
    [U4RK_LED_DSP]     = {0, 0, 255},
    [U4RK_LED_USB]     = {255, 0, 255},
    [U4RK_LED_FAULT]   = {255, 0, 0},
};

static const char *const stage_names[U4RK_LED_STAGE_COUNT] = {
    "off", "boot", "idle", "capture", "dsp", "usb", "fault",
};

static bool present;
static u4rk_led_stage_t current = U4RK_LED_STAGE_COUNT;
static bool host_shown;

static bool write_regs(uint8_t reg, const uint8_t *values, size_t count) {
    uint8_t buf[4];
    if (count + 1u > sizeof(buf)) {
        return false;
    }
    buf[0] = reg;
    for (size_t i = 0; i < count; ++i) {
        buf[i + 1u] = values[i];
    }
    int written = i2c_write_timeout_us(i2c0, LP5562_ADDR, buf, count + 1u,
                                       false, U4RK_LED_I2C_TIMEOUT_US);
    return written == (int)(count + 1u);
}

static bool write_reg(uint8_t reg, uint8_t value) {
    return write_regs(reg, &value, 1u);
}

static bool read_regs(uint8_t reg, uint8_t *values, size_t count) {
    if (i2c_write_timeout_us(i2c0, LP5562_ADDR, &reg, 1u, true,
                             U4RK_LED_I2C_TIMEOUT_US) != 1) {
        return false;
    }
    return i2c_read_timeout_us(i2c0, LP5562_ADDR, values, count, false,
                               U4RK_LED_I2C_TIMEOUT_US) == (int)count;
}

static uint8_t scale(uint8_t level) {
    return (uint8_t)((level * U4RK_LED_MAX_PWM + 127u) / 255u);
}

static bool show(u4rk_led_stage_t stage) {
    rgb_t c = stage_colours[stage];
    uint8_t bgr[3] = {scale(c.b), scale(c.g), scale(c.r)};
    return write_regs(LP5562_B_PWM, bgr, 3u);
}

bool u4rk_led_init(void) {
    i2c_init(i2c0, U4RK_LED_I2C_HZ);
    gpio_set_function(U4RK_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(U4RK_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(U4RK_I2C_SDA_PIN);
    gpio_pull_up(U4RK_I2C_SCL_PIN);

    /* The chip keeps its state across an RP2350 reboot: reset it first.
     * (The reset write is not acknowledged reliably, so ignore its result.) */
    write_reg(LP5562_RESET, 0xFFu);
    sleep_us(1000);
    present = write_reg(LP5562_ENABLE, LP5562_ENABLE_CHIP_EN);
    if (!present) {
        return false;
    }
    sleep_us(1000);   /* >= 500 us start-up after CHIP_EN */
    present = write_reg(LP5562_CONFIG, LP5562_CONFIG_INT_CLK) &&
              write_reg(LP5562_LED_MAP, LP5562_LED_MAP_I2C) &&
              write_reg(LP5562_W_PWM, 0u) &&
              show(U4RK_LED_BOOT);
    if (present) {
        /* Check the auto-incremented B, G, R write landed as intended. */
        uint8_t back[3];
        uint8_t white = scale(255u);
        present = read_regs(LP5562_B_PWM, back, 3u) &&
                  back[0] == white && back[1] == white && back[2] == white;
    }
    current = present ? U4RK_LED_BOOT : U4RK_LED_STAGE_COUNT;
    host_shown = false;
    return present;
}

bool u4rk_led_present(void) {
    return present;
}

void u4rk_led_set_stage(u4rk_led_stage_t stage) {
    if (!present || stage >= U4RK_LED_STAGE_COUNT || stage == current) {
        return;
    }
    if (show(stage)) {
        current = stage;
    }
}

void u4rk_led_set_host_connected(bool connected) {
    if (!present || connected == host_shown) {
        return;
    }
    if (write_reg(LP5562_W_PWM, connected ? scale(255u) : 0u)) {
        host_shown = connected;
    }
}

const char *u4rk_led_stage_name(u4rk_led_stage_t stage) {
    return stage < U4RK_LED_STAGE_COUNT ? stage_names[stage] : "?";
}
