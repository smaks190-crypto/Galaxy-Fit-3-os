/* Native watch UI backed by fit3-toolkit's public app interface. */
#include "ui.h"
#include "morning_ui.h"
#include "morning_font.h"

static u16 color565(u32 value)
{
    return (u16)(((value >> 8) & 0xf800) | ((value >> 5) & 0x07e0) | ((value >> 3) & 31));
}
static void paint_box(void *root, int x,int y,int w,int h,uint32_t rgb)
{
    if(x==0 && y==0 && w==256 && h==402) {
        set_bg_color((lv_obj_t *)root,color565(rgb));
        set_bg_opa((lv_obj_t *)root,255);
        return;
    }
    lv_obj_t *o=mk_box(root,x,y,w,h,0,color565(rgb));
    lv_obj_clear_flag(o,FL_CLICKABLE|FL_SCROLLABLE);
}
static void paint_text(void *root,int x,int y,int w,int size,int align,uint32_t rgb,const char *s)
{
    const void *font=size<=12?&FONT_M12:size<=17?&FONT_M14:size<=25?&FONT_M23:&FONT_M29;
    unsigned alignment=align==MU_RIGHT?3u:(align==MU_CENTER?ALIGN_CENTER:ALIGN_LEFT);
    mk_text(root,x,y,w,font,color565(rgb),alignment,s);
}
static const MorningBrief unavailable={0};

static void *morning_create(lv_obj_t *root)
{
    MorningCanvas canvas={root,paint_box,paint_text};
    morning_paint(&unavailable,&canvas);
    paint_text(root,14,113,228,12,MU_LEFT,0xb0a9b9u,"Нет синхронизированной ночи");
    return root;
}
static int morning_back(void *state) {(void)state;return 0;}
static void morning_destroy(void *state) {(void)state;}
APP_EXPORT(morning,"Morning",0,morning_create,morning_back,morning_destroy)

/* A fixture entry point: the emulator supplies a fresh LVGL root. */
void *morning_emulator_create(lv_obj_t *root) { return app_morning.create(root); }
