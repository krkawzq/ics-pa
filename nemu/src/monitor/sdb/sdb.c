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

#include <isa.h>// 包含了reg,mem等
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

// 内存操作
#include <memory/vaddr.h>



static int is_batch_mode = false;




/* We use the `readline' library to provide more flexibility to read from stdin. */
// static char* rl_gets() {
//   static char *line_read = NULL;

//   if (line_read) {
//     free(line_read);
//     line_read = NULL;
//   }

//   line_read = readline("(nemu) ");

//   if (line_read && *line_read) {
//     add_history(line_read);
//   }

//   return line_read;
// }

/*
更好的版本
增加多行指令支持
- 增加\的强制换行支持
- 更多待续
*/
#define LINE_BUFFER_SIZE 256
static char line_buffer[LINE_BUFFER_SIZE];

static struct {
  char *line_read;
  uint16_t len;
} rl = {line_buffer, 0};

/*
DONE
- \续行符
TODO
- ""续行
- 特殊语法下的续行
*/
static void rl_gets() {
  char *current_line;
  uint8_t len;
  rl.len = 0;

  current_line = readline("(nemu) ");
  
  // loop to read input until \n without \ to append new line
  while (1) {
    if (current_line == NULL) {
      continue;
    }else if (*current_line == '\0') {
      free(current_line);
      rl.len = 0;
      return;
    } // 如果输入为空，则释放内存并返回, 防止越界

    add_history(current_line);

    len = strlen(current_line);
    
    // 防止缓冲区溢出
    if (rl.len + len >= LINE_BUFFER_SIZE - 1) {
      printf("Line buffer overflow\n");
      free(current_line);
      break;
    }

    if (current_line[len - 1] == '\\') {
      // readline 得到的字符串不包含\n，检测最后一个是否是续航符
      memcpy(rl.line_read + rl.len, current_line, len - 1);
      rl.len += len - 1;
    } else {
      memcpy(rl.line_read + rl.len, current_line, len);
      rl.len += len;
      free(current_line);
      break;
    }
    free(current_line);

    current_line = readline("> ");
  }
  rl.line_read[rl.len] = '\0';
}


static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_info(char *args) {
  if (args == NULL) {
    printf("Need args w/r, [help info] to know more\n");
  } else if (strcmp("r", args) == 0) {
    isa_reg_display();
  } else if (strcmp("w", args) == 0) {
    if (wp_used_count == 0) {
      printf("No watchpoints\n");
    } else {
      print_watchpoint();
    }
  } else {
    printf("Unknown usage, [help info] to know more\n");
  }
  return 0;
}

static int cmd_si(char *args) {
  int n = 1;
  if (args) { // == NULL
    n = atoi(args);
  }
  if (n <=0 ) {
    printf("Invalid number of instructions to execute\nMore infomation to [help si]\n");
    return 0; // good hit
  }
  cpu_exec(n);
  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Need args [x n expr], [help x] to know more\n");
    return 0;
  }
  
  args = strtok(args, " "); // 获取第一个参数
  if (args == NULL) {
    printf("Need args [x n expr], [help x] to know more\n");
    return 0;
  }
  int n = atoi(args);
  if (n <= 0) {
    printf("Invalid number of bytes to read\nMore infomation to [help x]\n");
    return 0;
  }

  args = strtok(NULL, " "); // 获取第二个参数
  if (args == NULL) {
    printf("Need args [x n expr], [help x] to know more\n");
    return 0;
  }
  
  /*
  DONE:
  - 解析值，使用atoi,支持hex,oct,dec
  TODO:
  - 解析表达式
  */
  vaddr_t addr = strtol(args, NULL, 0);
  
  printf("addr:0x%08x\n", addr);
  for (int i = 0; i < n; i++) {
    word_t value = vaddr_read(addr + i * 4, 4);
    printf(
      "%02x %02x %02x %02x\n", 
      (value >> 24) & 0xff,
      (value >> 16) & 0xff,
      (value >> 8) & 0xff,
      value & 0xff
    );
  }
  
  return 0;
}

static int cmd_p(char *args) {
  uint32_t answer;
  bool success;
  if (args == NULL) {
    printf("empty expr\n");
    return 0;                 
  }
  answer = expr(args, &success);
  if (!success) {
    printf("invalid expr\n");
  } else {
    printf("%02x ", answer >> 24);
    printf("%02x ", (answer >> 16) & 0xff);
    printf("%02x ", (answer >> 8) & 0xff);
    printf("%02x\n", answer & 0xff);
  }
  return 0;
}


static int cmd_w(char *args) {
  if (args == NULL) {
    printf("empty expr\n");
    return 0;
  }
  bool success;
  word_t value = expr(args, &success);
  if (!success) {
    printf("invalid expr\n");
    return 0;
  }
  WP *wp = new_wp(); // 0 -> 1
  wp->NO = wp_no; // 唯一编号
  wp_no++;
  wp->value = value;
  strcpy(wp->expr, args);
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Need args [d N], [help d] to know more\n");
    return 0;
  }
  int n = atoi(args);
  if (n <= 0) {
    printf("Invalid number of watchpoints to delete\nMore infomation to [help d]\n");
    return 0;
  }

  delete_watchpoint(n);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands",             cmd_help  },
  { "c",    "Continue the execution of the program",                        cmd_c     },
  { "q",    "Exit NEMU",                                                    cmd_q     },
  { "info", "info[r/w]: print informations of registers or watch point",    cmd_info  },
  { "si",   "si[N]: single step",                                           cmd_si    },
  { "x",    "x [addr]: scan memory, use number or expression",              cmd_x     },
  { "p",    "p [expr]: evaluate expression",                                cmd_p     },
  { "w",    "w [expr]: set watch point",                                    cmd_w     },
  { "d",    "d [N]: delete watch point",                                    cmd_d     }
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

// void sdb_mainloop() {
//   if (is_batch_mode) {
//     cmd_c(NULL);
//     return;
//   }

//   for (char *str; (str = rl_gets()) != NULL; ) {
//     char *str_end = str + strlen(str);

//     /* extract the first token as the command */
//     char *cmd = strtok(str, " ");
//     if (cmd == NULL) { continue; }

//     /* treat the remaining string as the arguments,
//      * which may need further parsing
//      */
//     char *args = cmd + strlen(cmd) + 1;
//     if (args >= str_end) {
//       args = NULL;
//     }

// #ifdef CONFIG_DEVICE
//     extern void sdl_clear_event_queue();
//     sdl_clear_event_queue();
// #endif

//     int i;
//     for (i = 0; i < NR_CMD; i ++) {
//       if (strcmp(cmd, cmd_table[i].name) == 0) {
//         if (cmd_table[i].handler(args) < 0) { return; }//如果返回值小于0， 则退出
//         break;
//       }
//     }

//     if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
//   }
// }

// 通过readline解析命令，使用更好的readline版本（增加了跨行支持）
// 解析参数后，直接将args传递给命令处理函数
// 考虑到兼容，将第一个参数识别为命令，剩下的参数统一传递一个地址（即只strtok一次）
/*
更快更好的版本
修改：

*/
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  char *cmd, *args;

  // 获取命令行
  while (1) {
    rl_gets(); // 通过读取静态rl
    if (rl.len == 0) {
      continue;
    }
    
    cmd = strtok(rl.line_read, " ");
    int len = strlen(cmd);
    if (cmd == NULL) { continue; } // empty line
    if (len >= rl.len - 1) {
      args = NULL;
    } else {
      args = cmd + len + 1; 
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    // 解析命令
    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }
    if (i == NR_CMD) { printf("Unknown command '%s'\n", rl.line_read); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
