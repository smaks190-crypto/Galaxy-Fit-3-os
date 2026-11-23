#include "morning_ui.h"

#define PAPER 0xf1eee7u
#define DIM 0xb0a9b9u
#define LINE 0x29252fu
#define LILAC 0xb9a5f2u
static const uint32_t colors[]={0xf4ab80u,0xda9bd8u,0xaca3edu,0x778bc5u};
static const char *labels[]={"Бодр.","БДГ","Лёгк.","Глуб."};
static char *number(char *out,uint32_t n) {
 char rev[10]; unsigned i=0;
 do{rev[i++]=(char)('0'+n%10u);n/=10u;}while(n);
 while(i){*out++=rev[--i];}
 *out=0;return out;
}
static char *append(char *p,const char *s){while(*s)*p++=*s++;*p=0;return p;}
static void clock_text(char *s,unsigned minutes){
 unsigned h=minutes/60u,m=minutes%60u;
 s[0]=(char)('0'+h/10);s[1]=(char)('0'+h%10);s[2]=':';s[3]=(char)('0'+m/10);s[4]=(char)('0'+m%10);s[5]=0;
}
static void label(const MorningCanvas *c,int x,int y,int w,int size,int align,uint32_t col,const char *s){
 c->text(c->context,x,y,w,size,align,col,s);
}
static void rect(const MorningCanvas *c,int x,int y,int w,int h,uint32_t col){
 if(w>0&&h>0)c->rect(c->context,x,y,w,h,col);
}
static int plot_x(uint32_t elapsed,uint32_t duration){
 return 60+(int)((elapsed*181u+duration/2u)/duration);
}
static void graph(const MorningBrief *m,const MorningCanvas *c,uint32_t duration){
 for(int j=0;j<4;j++){
  int y=149+j*18;
  label(c,14,y-4,43,11,MU_LEFT,DIM,labels[j]);rect(c,60,y+1,182,1,LINE);
 }
 if(!duration)return;
 int last_y=-1,last_x=0,known=0;
 uint32_t last_t=m->begin_s;
 for(size_t i=0;m->spans&&i<m->n_spans;i++){
  mf_stage_span s=m->spans[i];
  if(s.end_s<=m->begin_s||s.begin_s>=m->end_s||s.begin_s>=s.end_s)continue;
  uint32_t a=s.begin_s<m->begin_s?m->begin_s:s.begin_s;
  uint32_t b=s.end_s>m->end_s?m->end_s:s.end_s;
  if(a<last_t)continue;
  int x=plot_x(a-m->begin_s,duration),end=plot_x(b-m->begin_s,duration);
  if(s.stage==MF_UNKNOWN){last_y=-1;last_t=b;continue;}
  const unsigned row_of_stage[MF_STAGE_COUNT]={[MF_AWAKE]=0,[MF_LIGHT]=2,[MF_DEEP]=3,[MF_REM]=1,[MF_UNKNOWN]=0};
  const unsigned row=row_of_stage[s.stage];
  int y=149+(int)row*18;
  if(last_y>=0&&last_t==a&&last_x==x&&last_y!=y){
   int top=y<last_y?y:last_y;int bottom=y>last_y?y:last_y;
   rect(c,x,top,2,bottom-top+3,colors[row]);
  }
  if(end>x){rect(c,x,y,end-x,3,colors[row]);known=1;}
  last_t=b;last_x=end;last_y=y;
 }
 for(int i=0;i<3;i++){
  if(m->clock_min[i]>=1440u)continue;
  char buf[6];clock_text(buf,m->clock_min[i]);
  const int xx[]={60,127,192};const int alignment[]={MU_LEFT,MU_CENTER,MU_RIGHT};
  label(c,xx[i],216,50,12,alignment[i],DIM,buf);
 }
 if(known&&m->stage_missing_s){
  char message[64];char *p=append(message,"Без стадий: ");p=number(p,m->stage_missing_s/60u);append(p," мин");
  label(c,14,240,228,12,MU_LEFT,DIM,message);
 }else label(c,14,240,228,12,MU_LEFT,DIM,known?"Все стадии записаны":"Нет данных о стадиях");
}
static void metric(const MorningCanvas *c,int x,const char *title,uint32_t n,unsigned mean,const char *unit){
 label(c,x,282,108,14,MU_LEFT,DIM,title);
 char buf[48];
 if(!n)label(c,x,306,106,27,MU_LEFT,PAPER,"—");
 else {
  char *p=number(buf,mean/10u);*p++=',';*p++=(char)('0'+mean%10u);p=append(p,unit);
  label(c,x,306,108,25,MU_LEFT,PAPER,buf);
 }
 char *p=append(buf,"Замеров: ");number(p,n);label(c,x,344,109,12,MU_LEFT,DIM,buf);
}
void morning_paint(const MorningBrief *m,const MorningCanvas *c){
 if(!m||!c||!c->rect||!c->text)return;
 rect(c,0,0,256,402,0);
 label(c,14,18,228,23,MU_LEFT,PAPER,"Утренняя сводка");
 uint32_t duration=m->end_s>m->begin_s?m->end_s-m->begin_s:0;
 if(duration>172800u)duration=0;
 label(c,14,56,135,13,MU_LEFT,LILAC,"Время сна");
 if(m->is_demo)label(c,156,56,86,12,MU_RIGHT,DIM,"Пример");
 char buf[72];char *p;
 if(duration&&m->sleeping_s<=duration && m->sleep_known){
  uint32_t min=m->sleeping_s/60u;p=number(buf,min/60u);p=append(p," ч ");p=number(p,min%60u);append(p," мин");
  label(c,14,79,228,33,MU_LEFT,PAPER,buf);
 }else label(c,14,79,228,29,MU_LEFT,PAPER,"—");
 graph(m,c,duration);
 rect(c,14,263,228,1,LINE);rect(c,127,282,1,76,LINE);
 metric(c,14,"Пульс, уд/мин",m->pulse_n,m->pulse_mean_x10,"");
 metric(c,140,"SpO₂",m->oxygen_n,m->oxygen_mean_x10,"%");
 if(m->oxygen_n&&m->oxygen_min<=100){
  p=append(buf,"Минимум SpO₂ за ночь: ");p=number(p,m->oxygen_min);append(p,"%");
  label(c,14,362,228,12,MU_LEFT,DIM,buf);
  if(m->oxygen_low_count){
   p=append(buf,"Замеров ниже 90%: ");number(p,m->oxygen_low_count);label(c,14,382,228,13,MU_LEFT,0xf4ab80u,buf);
  }else label(c,14,382,228,12,MU_LEFT,DIM,"Замеров ниже 90%: 0");
 }else label(c,14,366,228,13,MU_LEFT,DIM,"Минимум SpO₂: нет данных");
}

static void stage_duration(char *s,uint32_t seconds){
 if(seconds&&seconds<60){append(s,"< 1 мин");return;}
 uint32_t minutes=seconds/60u;
 char *p=s;
 if(minutes>=60){p=number(p,minutes/60u);p=append(p," ч ");}
 p=number(p,minutes%60u);append(p," мин");
}
void morning_paint_stages(const MorningBrief *m,const MorningCanvas *c){
 if(!m||!c||!c->text||!c->rect)return;
 rect(c,0,0,256,402,0);
 label(c,14,20,228,23,MU_LEFT,PAPER,"Структура сна");
 label(c,14,61,228,14,MU_LEFT,DIM,"Длительность за ночь");
 uint32_t duration=m->end_s-m->begin_s;
 const mf_stage order[]={MF_DEEP,MF_LIGHT,MF_REM,MF_AWAKE,MF_UNKNOWN};
 const char *titles[]={"Глубокий сон","Лёгкий сон","Быстрый сон (БДГ)","Бодрствование","Без данных"};
 const uint32_t inks[]={colors[3],colors[2],colors[1],colors[0],0x67616eu};
 unsigned x=14;uint32_t total=0;char value[40];
 if(duration&&duration<=172800u){
  for(unsigned i=0;i<5;i++){
   total+=m->phase_s[order[i]];
   unsigned until=14u+(total*228u+duration/2u)/duration;
   rect(c,(int)x,95,(int)(until-x),10,inks[i]);x=until;
  }
 }
 for(unsigned i=0;i<5;i++){
  int y=131+(int)i*44;
  rect(c,14,y+4,4,11,inks[i]);label(c,25,y,136,15,MU_LEFT,PAPER,titles[i]);
  if(!m->sleep_known&&order[i]!=MF_UNKNOWN)append(value,"—");
  else stage_duration(value,m->phase_s[order[i]]);
  label(c,158,y-1,84,17,MU_RIGHT,PAPER,value);rect(c,14,y+28,228,1,LINE);
 }
 label(c,14,360,228,12,MU_LEFT,DIM,"Доли относятся ко всему периоду");
}
