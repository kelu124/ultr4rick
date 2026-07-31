#ifndef U4RK_H
#define U4RK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define U4RK_SAMPLE_COUNT             4096u
#define U4RK_SAMPLE_RATE_HZ           60000000u
#define U4RK_PROTOCOL_VERSION         1u
#define U4RK_HEADER_SIZE              64u
#define U4RK_MAX_PAYLOAD_SIZE         (U4RK_SAMPLE_COUNT * sizeof(float))
#define U4RK_MAX_FRAME_SIZE           (U4RK_HEADER_SIZE + U4RK_MAX_PAYLOAD_SIZE)
#define U4RK_RAW_BUFFER_COUNT         2u
#define U4RK_OUTPUT_SLOT_COUNT        2u
#define U4RK_ALAW_DEFAULT_REFERENCE   512.0f

#define U4RK_ADC_CLOCK_PIN            0u
#define U4RK_ADC_DATA_FIRST_PIN       1u
#define U4RK_ADC_DATA_PIN_COUNT       10u
#define U4RK_DAC_CS_PIN               11u
#define U4RK_DAC_SCK_PIN              22u
#define U4RK_DAC_TX_PIN               23u
#define U4RK_PULSER_PIN_BASE          27u

typedef enum {
    U4RK_PAYLOAD_NONE = 0,
    U4RK_PAYLOAD_RAW = 1,
    U4RK_PAYLOAD_ENVELOPE = 2,
    U4RK_PAYLOAD_ALAW = 3,
} u4rk_payload_type_t;

enum {
    U4RK_FLAG_ALAW_SATURATED = 1u << 0,
    U4RK_FLAG_PROCESSING_DROP = 1u << 1,
    U4RK_FLAG_USB_DROP = 1u << 2,
    U4RK_FLAG_SELFTEST = 1u << 3,
    U4RK_FLAG_PULSER_ARMED = 1u << 4,
    U4RK_FLAG_SELFTEST_CASE_SHIFT = 8,
};

typedef enum {
    U4RK_PULSE_NEGATIVE_FIRST = 0,
    U4RK_PULSE_POSITIVE_FIRST = 1,
} u4rk_pulse_order_t;

typedef struct {
    uint32_t negative_ns;
    uint32_t damp_ns;
    uint32_t positive_ns;
    u4rk_pulse_order_t order;
} u4rk_pulse_config_t;

typedef struct {
    uint8_t raw_index;
    uint8_t payload_type;
    uint16_t flags;
    uint32_t sequence;
    uint32_t sample_rate_hz;
    uint64_t capture_timestamp_us;
    float alaw_reference;
    u4rk_pulse_config_t pulse;
} u4rk_capture_job_t;

typedef struct {
    uint32_t preprocess_us;
    uint32_t forward_fft_us;
    uint32_t mask_us;
    uint32_t inverse_fft_us;
    uint32_t magnitude_us;
    uint32_t alaw_us;
    uint32_t total_us;
    uint32_t worst_total_us;
    float dc_mean;
    float envelope_peak;
} u4rk_dsp_metrics_t;

#endif
