cmd_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/cpu/cpu-exec.o := unused

source_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/cpu/cpu-exec.o := src/cpu/cpu-exec.c

deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/cpu/cpu-exec.o := \
    $(wildcard include/config/itrace/cond.h) \
    $(wildcard include/config/itrace.h) \
    $(wildcard include/config/difftest.h) \
    $(wildcard include/config/watchpoint.h) \
    $(wildcard include/config/isa/x86.h) \
    $(wildcard include/config/device.h) \
    $(wildcard include/config/target/am.h) \
  /home/wzq/ics-pa/nemu/include/cpu/cpu.h \
  /home/wzq/ics-pa/nemu/include/common.h \
    $(wildcard include/config/mbase.h) \
    $(wildcard include/config/msize.h) \
    $(wildcard include/config/isa64.h) \
  /home/wzq/ics-pa/nemu/include/macro.h \
  /home/wzq/ics-pa/nemu/include/debug.h \
  /home/wzq/ics-pa/nemu/include/utils.h \
    $(wildcard include/config/target/native/elf.h) \
  /home/wzq/ics-pa/nemu/include/cpu/decode.h \
  /home/wzq/ics-pa/nemu/include/isa.h \
  /home/wzq/ics-pa/nemu/src/isa/riscv32/include/isa-def.h \
    $(wildcard include/config/rve.h) \
    $(wildcard include/config/rv64.h) \
  /home/wzq/ics-pa/nemu/include/cpu/difftest.h \
  /home/wzq/ics-pa/nemu/include/difftest-def.h \
    $(wildcard include/config/isa/mips32.h) \
    $(wildcard include/config/isa/riscv.h) \
    $(wildcard include/config/isa/loongarch32r.h) \

/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/cpu/cpu-exec.o: $(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/cpu/cpu-exec.o)

$(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/cpu/cpu-exec.o):
