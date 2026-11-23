#include "morning_summary.h"
#include <assert.h>
#include <stdio.h>

#define LEN(x) (sizeof(x) / sizeof((x)[0]))
#define STREAM(x) ((mf_stream){(x),LEN(x)})
static mf_input setup(uint32_t a, uint32_t b)
{
    mf_input in = {0};
    in.begin_s = a; in.end_s = b;
    in.hr_hold_s = 20; in.spo2_hold_s = 30; in.low_spo2_pct = 90;
    return in;
}

static void test_clipped_session(void)
{
    mf_input in = setup(86350,86450);
    mf_sample hr[] = {{86000,180,1},{86350,40,1},{86360,60,1},{86365,255,1},
                      {86400,80,1},{86440,0,1},{86450,240,1}};
    mf_sample oxygen[] = {{86340,88,1},{86355,88,1},{86365,90,1},
                          {86375,99,0},{86405,99,1},{86412,120,1}};
    mf_stage_span stages[]={{86300,86360,MF_LIGHT},{86360,86370,MF_AWAKE},
        {86380,86390,MF_UNKNOWN},{86395,86400,MF_DEEP},{86400,86455,MF_REM}};
    in.hr=STREAM(hr); in.spo2=STREAM(oxygen); in.stages=stages; in.n_stages=LEN(stages);
    mf_summary out;
    assert(mf_summarize(&in,&out)==MF_OK);
    assert(out.hr.n==3 && out.hr.discarded==2 && out.hr.mean_x10==600);
    assert(out.hr.min==40 && out.hr.max==80 && out.hr.coverage_s==35 && out.hr.max_gap_s==35);
    assert(out.spo2.n==3 && out.spo2.discarded==2 && out.spo2.mean_x10==923);
    assert(out.spo2.min==88 && out.spo2.max==99 && out.spo2.low_count==1);
    assert(out.spo2.low_coverage_s==10 && out.spo2.coverage_s==27 && out.spo2.max_gap_s==38);
    assert(out.session_s==100 && out.phase_s[MF_LIGHT]==10 && out.phase_s[MF_DEEP]==5);
    assert(out.phase_s[MF_REM]==50 && out.phase_s[MF_AWAKE]==10 && out.asleep_s==65);
    assert(out.phase_s[MF_UNKNOWN]==25 && out.longest_unknown_s==25);
}

static void test_empty_and_invalid(void)
{
    mf_input in=setup(0,1); mf_summary out;
    assert(mf_summarize(&in,&out)==MF_OK && !out.hr.n && !out.spo2.n);
    assert(out.phase_s[MF_UNKNOWN]==1 && out.spo2.max_gap_s==1 && !out.spo2.coverage_s);
    mf_sample invalid[]={{0,255,1}}; in.hr=STREAM(invalid); in.spo2=STREAM(invalid);
    assert(mf_summarize(&in,&out)==MF_OK);
    assert(out.hr.discarded==1 && out.spo2.discarded==1 && !out.spo2.n && !out.hr.n);
    assert(mf_summarize(0,&out)==MF_BAD_INPUT && !out.session_s);
    assert(mf_summarize(&in,0)==MF_BAD_INPUT);
    in.hr.records=0; assert(mf_summarize(&in,&out)==MF_BAD_INPUT);
    in=setup(1,1); assert(mf_summarize(&in,&out)==MF_BAD_INPUT);
    in=setup(0,2); in.low_spo2_pct=101; assert(mf_summarize(&in,&out)==MF_BAD_INPUT);
}

static void test_ambiguous_records(void)
{
    mf_summary out; mf_input in=setup(0,30);
    mf_sample s[]={{1,60,1},{1,80,1}}; in.hr=STREAM(s);
    assert(mf_summarize(&in,&out)==MF_BAD_ORDER && !out.hr.n);
    s[1].utc_s=0; assert(mf_summarize(&in,&out)==MF_BAD_ORDER);
    s[1].utc_s=2;
    mf_stage_span st[]={{0,20,MF_LIGHT},{18,30,MF_REM}}; in.stages=st; in.n_stages=2;
    assert(mf_summarize(&in,&out)==MF_BAD_STAGES && !out.session_s);
    st[1].begin_s=20; st[1].stage=(mf_stage)7; assert(mf_summarize(&in,&out)==MF_BAD_STAGES);
    st[1].stage=MF_REM; st[1].begin_s=30; assert(mf_summarize(&in,&out)==MF_BAD_STAGES);
}

static void test_extreme_clock_and_rounding(void)
{
    mf_input in=setup(0,UINT32_MAX); in.hr_hold_s=UINT32_MAX;
    mf_sample hr[]={{0,60,1},{UINT32_MAX-1,61,1}}; in.hr=STREAM(hr);
    mf_summary out; assert(mf_summarize(&in,&out)==MF_OK);
    assert(out.hr.n==2 && out.hr.mean_x10==605 && out.hr.coverage_s==UINT32_MAX);
    assert(out.hr.max_gap_s==0 && out.phase_s[MF_UNKNOWN]==UINT32_MAX);
    assert(out.longest_unknown_s==UINT32_MAX && out.spo2.max_gap_s==UINT32_MAX);
}

int main(void)
{
    test_clipped_session(); test_empty_and_invalid(); test_ambiguous_records();
    test_extreme_clock_and_rounding(); puts("Morning summary: all assertions passed.");
    return 0;
}
