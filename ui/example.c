#include "morning_ui.h"
#include <stdio.h>
#include <string.h>

static const struct {unsigned min;uint8_t stage;} sequence[]={
 {0,MF_AWAKE},{9,MF_LIGHT},{31,MF_DEEP},{57,MF_LIGHT},{92,MF_REM},{111,MF_LIGHT},
 {142,MF_DEEP},{164,MF_LIGHT},{183,MF_REM},{219,MF_UNKNOWN},{246,MF_LIGHT},{264,MF_DEEP},
 {282,MF_LIGHT},{317,MF_REM},{355,MF_LIGHT},{391,MF_DEEP},{410,MF_LIGHT},
 {428,MF_REM},{466,MF_LIGHT},{480,MF_AWAKE},{486,MF_UNKNOWN}
};
static void r(void*ctx,int x,int y,int w,int h,uint32_t rgb){
 (void)ctx;printf("R\t%d\t%d\t%d\t%d\t%06x\n",x,y,w,h,(unsigned)rgb);
}
static void t(void*ctx,int x,int y,int w,int sz,int align,uint32_t rgb,const char *s){
 (void)ctx;printf("T\t%d\t%d\t%d\t%d\t%d\t%06x\t%s\n",x,y,w,sz,align,(unsigned)rgb,s);
}
int main(int argc,char **argv){
 const uint32_t base=1800000000;
 MorningCanvas canvas={0,r,t};
 mf_stage_span spans[sizeof sequence/sizeof *sequence-1];
 mf_sample hr[264],oxygen[38];
 const size_t n_spans=sizeof spans/sizeof *spans;
 for(size_t i=0;i<n_spans;i++){
  spans[i]=(mf_stage_span){base+sequence[i].min*60u,base+sequence[i+1].min*60u,(mf_stage)sequence[i].stage};
 }
 for(unsigned i=0;i<264;i++)hr[i]=(mf_sample){base+i*108u,(uint16_t)(56u+(i%5u!=0u)),1};
 for(unsigned i=0;i<38;i++)oxygen[i]=(mf_sample){base+i*762u,(uint16_t)(i==4?89:i==5?90:i==6?88:98),1};
 mf_input in={.begin_s=base,.end_s=base+486u*60u,.hr={hr,264},.spo2={oxygen,38},.stages=spans,
  .n_stages=n_spans,.hr_hold_s=108,.spo2_hold_s=762,.low_spo2_pct=90};
 if(argc>1&&!strcmp(argv[1],"empty")){
  in.stages=0;in.n_stages=0;in.hr=(mf_stream){0};in.spo2=(mf_stream){0};
 }
 MorningBrief m;
 const uint16_t clock_min[]={1421,224,467};
 if(morning_prepare(&in,clock_min,1,&m)!=MF_OK)return 1;
 if(in.n_stages&&(m.oxygen_low_count!=2||m.stage_missing_s!=27u*60u||m.sleeping_s!=444u*60u||
    m.spans!=in.stages||m.oxygen_min!=88||m.n_spans!=n_spans))return 2;
 if(argc>1&&!strcmp(argv[1],"stages"))morning_paint_stages(&m,&canvas);
 else morning_paint(&m,&canvas);
 in.low_spo2_pct=91;
 if(morning_prepare(&in,clock_min,1,&m)!=MF_BAD_INPUT||m.spans)return 3;
 return 0;
}
