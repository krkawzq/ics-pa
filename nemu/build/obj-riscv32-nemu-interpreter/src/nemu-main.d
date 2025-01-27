cmd_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o := unused

source_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o := src/nemu-main.c

deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o := \
    $(wildcard include/config/target/am.h) \
  /home/wzq/ics-pa/nemu/include/common.h \
    $(wildcard include/config/mbase.h) \
    $(wildcard include/config/msize.h) \
    $(wildcard include/config/isa64.h) \
  /home/wzq/ics-pa/nemu/include/macro.h \
  /home/wzq/ics-pa/nemu/include/debug.h \
  /home/wzq/ics-pa/nemu/include/utils.h \
    $(wildcard include/config/target/native/elf.h) \

/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o: $(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o)

$(deps_/home/wzq/ics-pa/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o):
