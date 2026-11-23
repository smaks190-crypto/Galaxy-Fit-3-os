"""Render the separately mounted ARM application using the stock AZA3 LVGL in the toolkit emulator."""
from pathlib import Path
import argparse,hashlib,struct,sys,json
p=argparse.ArgumentParser()
p.add_argument('toolkit',type=Path);p.add_argument('stock_main',type=Path);p.add_argument('elf',type=Path);p.add_argument('output',type=Path)
a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
sys.path.insert(0,str(a.toolkit/'src/emu'))
from screenshot import Screen,children
from probe_lvgl import A
s=Screen(str(a.stock_main),'');s.seed_txt();e=s.e
image=a.elf.read_bytes();entry,phoff=struct.unpack_from('<II',image,24);size,count=struct.unpack_from('<HH',image,42)
pages=set()
for i in range(count):
    k,off,addr,_,length,mem,_,_=struct.unpack_from('<8I',image,phoff+i*size)
    if k!=1:continue
    for page in range(addr & ~4095,(addr+mem+4095)& ~4095,4096):
        if page not in pages:
            e.uc.mem_map(page,4096);pages.add(page)
    e.write(addr,image[off:off+length])
root=e.call(A['obj_create'],0)
assert e.call(entry,root,max_insns=25000000)==root
nodes=children(e,root)
labels=[e.cstr(e.u32(c+0x2c)).encode('latin1').decode('utf8') for c in nodes if e.u32(c)==A['label_class']]
assert labels.count('—')==3 and labels.count('Замеров: 0')==2
assert 'Нет синхронизированной ночи' in labels and 'Пример' not in labels
s.show(root)
assert s.flushes>0
im=s.png(str(a.output/'Morning-LVGL.png'),1)
assert len(im.getcolors(256*402))>5 and im.getbbox()
summary={'entry':'Morning SDK app factory','node_count':len(nodes),'labels':labels,'display_flushes':s.flushes,'sha256_elf':hashlib.sha256(image).hexdigest(),'stock_main_sha256':hashlib.sha256(a.stock_main.read_bytes()).hexdigest(),'bluetooth_accesses':False,'actual_sensor_data':False}
(a.output/'factory-result.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2))
print(json.dumps(summary,ensure_ascii=False))
