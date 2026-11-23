#ifndef MORNING_UI_H
#define MORNING_UI_H
#include "../core/morning_summary.h"


enum MorningAlign { MU_LEFT, MU_CENTER, MU_RIGHT };


/* Provide the local clock at window start, midpoint and end, computed by the caller.
 * Absolute timestamps and spans are sorted Unix seconds, with exclusive right endpoints.
 * Missing stages stay visibly empty. sleeping_s excludes wake and unknown intervals.
 * All UI strings use UTF-8. */
typedef struct {
 uint32_t begin_s, end_s, sleeping_s;
 uint16_t clock_min[3];
 const mf_stage_span *spans;
 size_t n_spans;
 uint32_t stage_missing_s, oxygen_low_count, phase_s[MF_STAGE_COUNT];
 uint32_t pulse_n, oxygen_n;
 uint16_t pulse_mean_x10, oxygen_mean_x10;
 uint8_t oxygen_min;
 uint8_t is_demo, sleep_known;
} MorningBrief;

/* rgb is 0xRRGGBB. Text has top-aligned ink, measured/clipped to width;
 * align specifies layout within that width. Backends implement the two primitives. */
typedef struct {
 void *context;
 void (*rect)(void *, int x,int y,int w,int h,uint32_t rgb);
 void (*text)(void *, int x,int y,int w,int size,int align,uint32_t rgb,const char *utf8);
} MorningCanvas;
/* Validates and summarizes exactly the same input spans passed to the plot.
 * Input records must outlive painting. This product view uses a strict 90% threshold. */
mf_result morning_prepare(const mf_input *, const uint16_t local_clock_min[3], uint8_t is_demo, MorningBrief *);
void morning_paint(const MorningBrief *, const MorningCanvas *);
void morning_paint_stages(const MorningBrief *, const MorningCanvas *);
#endif
