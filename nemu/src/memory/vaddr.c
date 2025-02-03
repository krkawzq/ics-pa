/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

// 内存跟踪
// 但是其实会有问题，因为sdb调试的时候，使用vaddr访问也会被记录
#ifdef CONFIG_MTRACE
#define MTRACE_SIZE 32
char mtrace_ringbuf[MTRACE_SIZE][128];
int mtrace_idx = 0;
size_t mtrace_count = 0;  // 添加计数器

void mtrace_format(char *buf, vaddr_t addr, int len, word_t data, bool is_read) {
  if (is_read) {
    switch (len) {
      case 1: sprintf(buf, "0x%08x: READ %d  %02x\n", addr, len, data); break;
      case 2: sprintf(buf, "0x%08x: READ %d  %04x\n", addr, len, data); break;
      case 4: sprintf(buf, "0x%08x: READ %d  %08x\n", addr, len, data); break;
    }
  } else {
    switch (len) {
      case 1: sprintf(buf, "0x%08x: WRITE %d  %02x\n", addr, len, data); break;
      case 2: sprintf(buf, "0x%08x: WRITE %d  %04x\n", addr, len, data); break;
      case 4: sprintf(buf, "0x%08x: WRITE %d  %08x\n", addr, len, data); break;
    }
  }
}

void mtrace_print() {
  if (mtrace_count == 0) {
    printf("No memory trace\n");
    return;
  }
  if (mtrace_count < MTRACE_SIZE) {
    for (int i = 0; i < mtrace_idx; i++) {
      printf("%s", mtrace_ringbuf[i]);
    }
  } else {
    for (int i = (mtrace_idx + 1) % MTRACE_SIZE; i < MTRACE_SIZE; i++) {
      printf("%s", mtrace_ringbuf[i]);
    }
    for (int i = 0; i < mtrace_idx; i++) {
      printf("%s", mtrace_ringbuf[i]);
    }
  }
}
#endif

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  word_t data = paddr_read(addr, len);
#ifdef CONFIG_MTRACE
  mtrace_format(mtrace_ringbuf[mtrace_idx], addr, len, data, true);
  mtrace_idx = (mtrace_idx + 1) % MTRACE_SIZE;
  mtrace_count++;
#endif
  return data;
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
#ifdef CONFIG_MTRACE
  mtrace_format(mtrace_ringbuf[mtrace_idx], addr, len, data, false);
  mtrace_idx = (mtrace_idx + 1) % MTRACE_SIZE;
  mtrace_count++;
#endif
  paddr_write(addr, len, data);
}
