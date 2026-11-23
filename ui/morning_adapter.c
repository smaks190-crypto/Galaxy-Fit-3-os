#include "morning_ui.h"

mf_result morning_prepare(const mf_input *input,const uint16_t clock_min[3],uint8_t is_demo, MorningBrief *out){
 if(!out)return MF_BAD_INPUT;
 *out=(MorningBrief){0};
 if(!input||!clock_min||input->low_spo2_pct!=90u)return MF_BAD_INPUT;
 if(input->end_s<=input->begin_s||input->end_s-input->begin_s>172800u)return MF_BAD_INPUT;
 mf_summary s;
 mf_result status=mf_summarize(input,&s);
 if(status!=MF_OK)return status;
 out->begin_s=s.begin_s;out->end_s=s.end_s;
 out->sleeping_s=s.asleep_s;out->stage_missing_s=s.phase_s[MF_UNKNOWN];
 for(unsigned i=0;i<MF_STAGE_COUNT;i++)out->phase_s[i]=s.phase_s[i];
 out->sleep_known=s.session_s>s.phase_s[MF_UNKNOWN];
 out->spans=input->stages;out->n_spans=input->n_stages;
 for(unsigned i=0;i<3;i++)out->clock_min[i]=clock_min[i];
 out->pulse_n=s.hr.n;out->pulse_mean_x10=s.hr.mean_x10;
 out->oxygen_n=s.spo2.n;out->oxygen_mean_x10=s.spo2.mean_x10;out->oxygen_min=(uint8_t)s.spo2.min;
 out->oxygen_low_count=s.spo2.low_count;out->is_demo=is_demo;
 return MF_OK;
}
