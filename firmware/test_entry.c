#include "morning_entry.h"
#include <assert.h>
int main(void)
{
    mf_input night = {.begin_s=100, .end_s=200, .low_spo2_pct=90, .hr_hold_s=30, .spo2_hold_s=30};
    MorningBrief summary;
    const uint16_t clock[3] = {60,61,62};
    assert(morning_prepare_on_wake(&night,0,201,0,clock,&summary)==MORNING_WAIT);
    assert(morning_prepare_on_wake(&night,1,199,0,clock,&summary)==MORNING_WAIT);
    assert(morning_prepare_on_wake(&night,1,201,0,clock,&summary)==MORNING_READY);
    assert(morning_prepare_on_wake(&night,1,201,200,clock,&summary)==MORNING_WAIT);
    assert(summary.end_s==0);
    return 0;
}
