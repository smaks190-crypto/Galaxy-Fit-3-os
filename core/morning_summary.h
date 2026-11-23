#ifndef MORNING_SUMMARY_H
#define MORNING_SUMMARY_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    MF_AWAKE, MF_LIGHT, MF_DEEP, MF_REM, MF_UNKNOWN, MF_STAGE_COUNT
} mf_stage;

typedef struct {
    uint32_t utc_s;
    uint16_t value;
    uint8_t valid; /* Set to one only for valid, on-wrist sensor records. */
} mf_sample;

typedef struct { uint32_t begin_s, end_s; mf_stage stage; } mf_stage_span;
typedef struct { const mf_sample *records; size_t n; } mf_stream;

typedef struct {
    uint32_t begin_s, end_s;
    mf_stream hr, spo2;
    const mf_stage_span *stages;
    size_t n_stages;
    uint32_t hr_hold_s, spo2_hold_s; /* Expected maximum interval per sample. */
    uint8_t low_spo2_pct; /* Count samples strictly below this configured value. */
} mf_input;

typedef struct {
    uint32_t n, discarded, coverage_s, max_gap_s;
    uint16_t min, max, mean_x10;
    uint32_t low_count, low_coverage_s;
} mf_stat;

typedef struct {
    uint32_t begin_s, end_s, session_s;
    uint32_t phase_s[MF_STAGE_COUNT], asleep_s, longest_unknown_s;
    mf_stat hr, spo2;
} mf_summary;

typedef enum { MF_OK, MF_BAD_INPUT, MF_BAD_ORDER, MF_BAD_STAGES } mf_result;

/* UTC half-open intervals. Each stream must be strictly increasing in time.
 * Stage spans must be nonempty, chronological, and nonoverlapping. Arrays are
 * caller-owned. The output may not alias input. Statistics with n==0 have
 * no value (render a dash). On errors, a supplied output is cleared. */
mf_result mf_summarize(const mf_input *input, mf_summary *output);

#endif
