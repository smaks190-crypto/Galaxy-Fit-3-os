#include "morning_entry.h"

morning_status morning_prepare_on_wake(const mf_input *night, int finalized_main_sleep,
                              uint32_t now, uint32_t last_viewed_end,
                              const uint16_t local_clock_minutes[3], MorningBrief *screen)
{
    if (!screen) return MORNING_INVALID;
    *screen = (MorningBrief){0};
    if (!night || !finalized_main_sleep) return MORNING_WAIT;
    if (night->begin_s >= night->end_s) return MORNING_INVALID;
    if (night->end_s > now || night->end_s <= last_viewed_end) return MORNING_WAIT;
    return morning_prepare(night, local_clock_minutes, 0, screen) == MF_OK ? MORNING_READY : MORNING_INVALID;
}
