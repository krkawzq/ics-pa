cmd_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/memory/paddr.o := unused

source_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/memory/paddr.o := src/memory/paddr.c

deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/memory/paddr.o := \
    $(wildcard include/config/pmem/malloc.h) \
    $(wildcard include/config/pmem/garray.h) \
    $(wildcard include/config/msize.h) \
    $(wildcard include/config/mbase.h) \
    $(wildcard include/config/mem/random.h) \
    $(wildcard include/config/device.h) \
  /home/wzq/ics-pa/nemu/include/memory/host.h \
    $(wildcard include/config/isa64.h) \
    $(wildcard include/config/rt/check.h) \
  /home/wzq/ics-pa/nemu/include/common.h \
    $(wildcard include/config/target/am.h) \
  /home/wzq/ics-pa/nemu/include/macro.h \
  /home/wzq/ics-pa/nemu/include/debug.h \
  /home/wzq/ics-pa/nemu/include/utils.h \
    $(wildcard include/config/target/native/elf.h) \
  /home/wzq/ics-pa/nemu/include/memory/paddr.h \
    $(wildcard include/config/pc/reset/offset.h) \
  /home/wzq/ics-pa/nemu/include/device/mmio.h \
  /home/wzq/ics-pa/nemu/include/isa.h \
  /home/wzq/ics-pa/nemu/src/isa/riscv32/include/isa-def.h \
    $(wildcard include/config/rve.h) \
    $(wildcard include/config/rv64.h) \

/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/memory/paddr.o: $(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/memory/paddr.o)

$(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/memory/paddr.o):
