#ifndef GALAXYFIT3_MORNING_ENTRY_H
#define GALAXYFIT3_MORNING_ENTRY_H
#include "../ui/morning_ui.h"

/* Pass the newest finalized main sleep after history sync. The window manager
 * persists last_viewed_end only after it successfully presents this screen. */
typedef enum { MORNING_WAIT, MORNING_READY, MORNING_INVALID } morning_status;
morning_status morning_prepare_on_wake(const mf_input *night, int finalized_main_sleep,
                                       uint32_t now, uint32_t last_viewed_end,
                                       const uint16_t local_clock_minutes[3],
                                       MorningBrief *screen);
#endif
