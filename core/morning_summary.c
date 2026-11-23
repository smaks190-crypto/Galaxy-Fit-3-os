#include "morning_summary.h"

static uint32_t lesser(uint32_t a, uint32_t b) { return a < b ? a : b; }
static uint32_t greater(uint32_t a, uint32_t b) { return a > b ? a : b; }

static void clear_output(mf_summary *out)
{
    volatile unsigned char *p = (volatile unsigned char *)out;
    for (size_t i = 0; i < sizeof(*out); ++i) p[i] = 0;
}

static int ordered(mf_stream input)
{
    for (size_t i = 1; i < input.n; ++i)
        if (input.records[i].utc_s <= input.records[i-1].utc_s) return 0;
    return 1;
}

static uint16_t rounded_mean(uint64_t sum, uint32_t count, uint16_t maximum)
{
    /* Bounded quotient, also builds for Cortex-M4 without a 64-bit divide runtime. */
    const uint64_t numerator = sum * 10 + count / 2;
    unsigned low = 0, high = 10u * maximum;
    while (low < high) {
        unsigned middle = (low + high + 1) / 2;
        if ((uint64_t)middle * count <= numerator) low = middle;
        else high = middle - 1;
    }
    return (uint16_t)low;
}

static void summarize_sensor(const mf_input *in, mf_stream stream, uint32_t hold,
                             uint16_t limit, uint8_t low_threshold, mf_stat *out)
{
    uint64_t sum = 0;
    uint32_t covered_until = in->begin_s;
    for (size_t i = 0; i < stream.n; ++i) {
        const mf_sample *s = &stream.records[i];
        if (s->utc_s < in->begin_s) continue;
        if (s->utc_s >= in->end_s) break;
        if (s->valid != 1 || s->value == 0 || s->value > limit) {
            ++out->discarded;
            continue;
        }
        uint32_t next = in->end_s;
        if (i + 1 < stream.n) next = lesser(next, stream.records[i+1].utc_s);
        const uint32_t length = lesser(hold, next - s->utc_s);
        out->max_gap_s = greater(out->max_gap_s, s->utc_s - covered_until);
        covered_until = s->utc_s + length;
        out->coverage_s += length;
        if (!out->n || s->value < out->min) out->min = s->value;
        if (s->value > out->max) out->max = s->value;
        ++out->n;
        sum += s->value;
        if (s->value < low_threshold) {
            ++out->low_count;
            out->low_coverage_s += length;
        }
    }
    out->max_gap_s = greater(out->max_gap_s, in->end_s - covered_until);
    if (out->n) out->mean_x10 = rounded_mean(sum, out->n, out->max);
}

static void summarize_stages(const mf_input *in, mf_summary *out)
{
    uint32_t last_known_end = in->begin_s;
    for (size_t i = 0; i < in->n_stages; ++i) {
        const mf_stage_span *s = &in->stages[i];
        if (s->end_s <= in->begin_s || s->stage == MF_UNKNOWN) continue;
        if (s->begin_s >= in->end_s) break;
        uint32_t a = greater(in->begin_s, s->begin_s);
        uint32_t b = lesser(in->end_s, s->end_s);
        uint32_t gap = a - last_known_end;
        out->phase_s[MF_UNKNOWN] += gap;
        out->longest_unknown_s = greater(out->longest_unknown_s, gap);
        out->phase_s[s->stage] += b - a;
        last_known_end = b;
    }
    uint32_t tail = in->end_s - last_known_end;
    out->phase_s[MF_UNKNOWN] += tail;
    out->longest_unknown_s = greater(out->longest_unknown_s, tail);
    out->asleep_s = out->phase_s[MF_LIGHT] + out->phase_s[MF_DEEP] + out->phase_s[MF_REM];
}

mf_result mf_summarize(const mf_input *in, mf_summary *out)
{
    if (!out) return MF_BAD_INPUT;
    clear_output(out);
    if (!in || in->begin_s >= in->end_s || !in->hr_hold_s || !in->spo2_hold_s ||
        !in->low_spo2_pct || in->low_spo2_pct > 100 ||
        (in->hr.n && !in->hr.records) || (in->spo2.n && !in->spo2.records) ||
        (in->n_stages && !in->stages)) return MF_BAD_INPUT;
    if (!ordered(in->hr) || !ordered(in->spo2)) return MF_BAD_ORDER;
    for (size_t i = 0; i < in->n_stages; ++i) {
        const mf_stage_span *s = &in->stages[i];
        if (s->begin_s >= s->end_s || (unsigned)s->stage >= MF_STAGE_COUNT ||
            (i && s->begin_s < in->stages[i - 1].end_s)) return MF_BAD_STAGES;
    }
    out->begin_s = in->begin_s;
    out->end_s = in->end_s;
    out->session_s = in->end_s - in->begin_s;
    summarize_stages(in, out);
    summarize_sensor(in, in->hr, in->hr_hold_s, 254, 0, &out->hr);
    summarize_sensor(in, in->spo2, in->spo2_hold_s, 100, in->low_spo2_pct, &out->spo2);
    return MF_OK;
}
