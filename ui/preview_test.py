"""Run the freestanding C on the host, verify geometry/gaps and rasterize its commands."""
import argparse
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
from PIL import Image,ImageDraw,ImageFont
parser=argparse.ArgumentParser()
parser.add_argument('--font',type=Path,default='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf')
parser.add_argument('--output',type=Path,default=Path(__file__).resolve().parent)
args=parser.parse_args();src=Path(__file__).resolve().parent
def render(rows):
 im=Image.new('RGB',(256,402))
 dr=ImageDraw.Draw(im)
 for row in rows:
  op,*parts=row.split('\t')
  if op=='R':
   x,y,w,h=map(int,parts[:4]);assert 0<=x<x+w<=256 and 0<=y<y+h<=402
   dr.rectangle((x,y,x+w-1,y+h-1),fill='#'+parts[-1]);continue
  x,y,w,size,align=map(int,parts[:5]);color,txt=parts[5:]
  for sz in range(size,6,-1):
   font=ImageFont.truetype(str(args.font),sz);l,t,r,b=font.getbbox(txt)
   if r-l<=w:break
  assert 0<=x and x+w<=256 and y+b-t<=402 and r-l<=w,(txt,x,y,r,b)
  anchor_x=x+(0 if align==0 else (w-r+l)//2 if align==1 else w-r+l)
  dr.text((anchor_x-l,y-t),txt,font=font,fill='#'+color)
 return im
with TemporaryDirectory() as d:
 exe=Path(d)/'ui'
 subprocess.run(['cc','-std=c99','-Wall','-Wextra','-Werror','-pedantic',str(src/'morning_ui.c'),str(src/'example.c'),str(src/'morning_adapter.c'),str(src/'../core/morning_summary.c'),'-o',str(exe)],check=True)
 for arg,out in [('', 'Morning-example.png'),('empty','Morning-no-data.png'),('stages','Morning-stages.png')]:
  rows=subprocess.check_output([str(exe),arg],text=True).splitlines()
  im=render(rows);args.output.mkdir(exist_ok=True,parents=True);im.save(args.output/out)
  stagecolors={(244,171,128),(218,155,216),(172,163,237),(119,139,197)}
  if arg=='empty':
   assert not any(px in stagecolors for px in im.crop((60,145,243,211)).get_flattened_data())
   assert 'Нет данных о стадиях' in '\n'.join(rows)
  elif arg=='stages':
   assert all(name in '\n'.join(rows) for name in ['Глубокий сон','Быстрый сон (БДГ)','Бодрствование','15 мин','27 мин'])
   assert set(im.crop((14,95,242,105)).get_flattened_data())==stagecolors|{(103,97,110)}
  else:
   assert all(label in '\n'.join(rows) for label in ['23:41','03:44','07:47','7 ч 24 мин','56,8','97,3%','Замеров ниже 90%: 2','Без стадий: 27 мин'])
   assert all(im.getpixel((x,y)) not in stagecolors for x in range(143,151) for y in range(146,209))
   assert len(set(im.crop((60,148,242,209)).get_flattened_data()) & stagecolors)==4
print('PASS: host C, geometry, four stages, explicit gap, midnight clock, totals, absent samples')
