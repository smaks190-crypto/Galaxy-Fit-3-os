"""Build the emulator ELF using an existing checkout, Morning UI sources and a local TTF."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('toolkit', type=Path)
p.add_argument('morning', type=Path)
p.add_argument('font', type=Path)
p.add_argument('out', type=Path)
p.add_argument('--zig',default='zig')
a=p.parse_args()
a.out.mkdir(parents=True,exist_ok=True)
app=Path(__file__).with_name('morning_app.c')
ui=a.morning/'ui/morning_ui.c'
sys.path.insert(0,str(a.toolkit/'src/assets'))
import mkfont
charset=set('0123456789:,.+-%— ')
for f in (app,ui):
    for literal in re.findall(r'"([^"\n]*)"',f.read_text()):
        charset.update(literal)
codes=sorted(map(ord,charset))
missing=set(codes)-mkfont.ttf_codepoints(a.font)
if missing: raise ValueError('Font misses '+str(missing))
mkfont.charset=lambda extra,polish:codes
headers=[mkfont.emit(mkfont.build(a.font,n,bpp=2,polish=False),'M'+str(n)) for n in (12,14,23,29)]
(a.out/'morning_font.h').write_text('\n'.join(headers))
subprocess.run([a.zig,'cc','-target','thumb-freestanding-eabi','-mcpu=cortex_m4','-Oz','-std=c11','-nostdlib','-ffreestanding','-fno-builtin','-fno-stack-protector','-fdata-sections','-ffunction-sections','-Wl,--gc-sections,-e,morning_emulator_create','-I'+str(a.toolkit/'src/watch'),'-I'+str(a.morning/'ui'),'-I'+str(a.out),str(ui),str(app),'-o',str(a.out/'Morning-Emulator.elf')],check=True)
