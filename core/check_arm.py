"""Run one reproducible cross-midnight data vector on the compiled Cortex-M4 ELF."""
import struct
import sys
from pathlib import Path

from unicorn import Uc, UC_ARCH_ARM, UC_MODE_MCLASS, UC_MODE_THUMB
from unicorn.arm_const import UC_ARM_REG_LR, UC_ARM_REG_PC, UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_SP

elf = Path(sys.argv[1]).read_bytes()
assert elf[:6] == b'\x7fELF\x01\x01'
entry, phoff = struct.unpack_from('<II', elf, 24)
phsize, nph = struct.unpack_from('<HH', elf, 42)
cpu = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
pages = set()
for i in range(nph):
    kind, offset, addr, _, size, mem, _, _ = struct.unpack_from('<8I', elf, phoff + i * phsize)
    if kind != 1:
        continue
    for page in range(addr & ~4095, (addr + mem + 4095) & ~4095, 4096):
        if page not in pages:
            cpu.mem_map(page, 4096)
            pages.add(page)
    cpu.mem_write(addr, elf[offset:offset + size])
ram = 0x20000000
cpu.mem_map(ram, 0x10000)
inp, out, hr, ox, st, stop = [ram + x for x in (0x40,0x200,0x400,0x500,0x600,0x800)]
sample = struct.Struct('<IHBx')
cpu.mem_write(hr, b''.join(sample.pack(*s) for s in [(86370,60,1),(86410,80,1)]))
cpu.mem_write(ox, b''.join(sample.pack(*s) for s in [(86370,89,1),(86375,92,0),(86410,99,1)]))
cpu.mem_write(st, struct.pack('<6I',86370,86395,1,86405,86430,3))
cpu.mem_write(inp, struct.pack('<10IB3x',86370,86430,hr,2,ox,3,st,2,15,25,90))
for reg,val in [(UC_ARM_REG_SP,ram+0xff00),(UC_ARM_REG_LR,stop|1),(UC_ARM_REG_R0,inp),(UC_ARM_REG_R1,out)]:
    cpu.reg_write(reg,val)
cpu.emu_start(entry,stop,count=100000)
assert cpu.reg_read(UC_ARM_REG_PC)==stop and cpu.reg_read(UC_ARM_REG_R0)==0
buf=cpu.mem_read(out,104)
assert struct.unpack_from('<10I',buf,0)==(86370,86430,60,0,25,0,25,10,50,10)
stat=struct.Struct('<4I3H2x2I')
assert stat.unpack_from(buf,40)==(2,0,30,25,60,80,700,0,0)
assert stat.unpack_from(buf,72)==(2,1,25,35,89,99,940,1,5)
print('Cortex-M4 Thumb execution passed: 60 s across midnight; HR 70.0; SpO2 94.0; 10 s unknown stage.')
