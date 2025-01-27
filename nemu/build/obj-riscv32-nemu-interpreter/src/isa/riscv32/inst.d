cmd_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/isa/riscv32/inst.o := unused

source_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/isa/riscv32/inst.o := src/isa/riscv32/inst.c

deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/isa/riscv32/inst.o := \
  src/isa/riscv32/local-include/reg.h \
    $(wildcard include/config/rt/check.h) \
    $(wildcard include/config/rve.h) \
  /home/wzq/ics-pa/nemu/include/common.h \
    $(wildcard include/config/target/am.h) \
    $(wildcard include/config/mbase.h) \
    $(wildcard include/config/msize.h) \
    $(wildcard include/config/isa64.h) \
  /home/wzq/ics-pa/nemu/include/macro.h \
  /home/wzq/ics-pa/nemu/include/debug.h \
  /home/wzq/ics-pa/nemu/include/utils.h \
    $(wildcard include/config/target/native/elf.h) \
  /home/wzq/ics-pa/nemu/include/cpu/cpu.h \
  /home/wzq/ics-pa/nemu/include/cpu/ifetch.h \
  /home/wzq/ics-pa/nemu/include/memory/vaddr.h \
  /home/wzq/ics-pa/nemu/include/cpu/decode.h \
    $(wildcard include/config/itrace.h) \
  /home/wzq/ics-pa/nemu/include/isa.h \
  /home/wzq/ics-pa/nemu/src/isa/riscv32/include/isa-def.h \
    $(wildcard include/config/rv64.h) \

/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/isa/riscv32/inst.o: $(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/isa/riscv32/inst.o)

$(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/isa/riscv32/inst.o):
