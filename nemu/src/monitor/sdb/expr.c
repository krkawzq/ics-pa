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
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include <memory/vaddr.h>
#include <memory/paddr.h>

/*
支持的运算符

一元：
~ ! *(unref)

二元：
+ - * /
| &
|| && == >= <= != > <

标点：
( )

数字：
dec 支持 123 +123 -123
hex 0x5a

运算符优先级：
() 括号
~ ! *(unref) 一元运算符
* / 乘除
+ - 加减
> >= < <= 比较
== != 相等性
& 按位与
| 按位或
&& 逻辑与
|| 逻辑或

*/

enum {
  TK_SPACE = 0,
  TK_REG,
  TK_LOGIC_EQ,
  TK_LOGIC_NEQ,
  TK_LOGIC_AND,
  TK_LOGIC_OR,
  TK_GE,
  TK_LE,
  TK_GT,
  TK_LT,
  TK_BIT_AND,
  TK_BIT_OR,
  TK_LOGIC_NOT,
  TK_BIT_NOT,
  TK_DEREF,
  TK_PLUS,
  TK_MINUS,
  TK_MUT,
  TK_DIV,
  TK_NUM_DEC,
  TK_NUM_HEX,
  TK_LBRACKET,
  TK_RBRACKET,
  TK_IDENT,
};
static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
    {"\s+",                 TK_SPACE        },
    {"\\$(0|ra|sp|gp|tp|t[0-6]|s([0-9]|10|11)|a[0-7])", TK_REG},
    {"==",                  TK_LOGIC_EQ     },
    {"!=",                  TK_LOGIC_NEQ    },
    {"&&",                  TK_LOGIC_AND    },
    {"\\|\\|",              TK_LOGIC_OR     },
    {">=",                  TK_GE           },
    {"<=",                  TK_LE           },
    {">",                   TK_GT           },
    {"<",                   TK_LT           },
    {"&",                   TK_BIT_AND      },
    {"\\|",                 TK_BIT_OR       },
    {"!",                   TK_LOGIC_NOT    },
    {"~",                   TK_BIT_NOT      },
    {"\\+",                 TK_PLUS         },
    {"-",                   TK_MINUS        },
    {"\\*",                 TK_MUT          },
    {"/",                   TK_DIV          },
    {"0[xX][0-9a-fA-F]+",   TK_NUM_HEX      },
    {"[0-9]+",              TK_NUM_DEC      },
    {"\\(",                 TK_LBRACKET     },
    {"\\)",                 TK_RBRACKET     },
    {"[a-zA-Z_][a-zA-Z0-9_]*", TK_IDENT} // identifier
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32]; // 固定buffer
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) { // 尝试匹配所有正则
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) { // 从position开始匹配，匹配一个，匹配成功返回一个regmatch_t,包含offsets
        char *substr_start = e + position; // 子串起始位置
        int substr_len = pmatch.rm_eo; // 子串长度

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        // rules[i].token_type
        position += substr_len; // 调整position

        if (rules[i].token_type == TK_SPACE) {  
          continue;
        } // 跳过空格
        if (substr_len >= 31) { 
          Assert(false, "too long token len");
        }

        bool isDeref = false;
        bool signal_positive = true;
        switch (rules[i].token_type) {
        case TK_MUT:
          if (nr_token == 0) {
            isDeref = true;
          } else {
            isDeref = !( // mut 的情况，前面必须是表达式或数字
              tokens[nr_token - 1].type == TK_RBRACKET ||
              tokens[nr_token - 1].type == TK_NUM_DEC ||
              tokens[nr_token - 1].type == TK_NUM_HEX ||
              tokens[nr_token - 1].type == TK_REG ||
              tokens[nr_token - 1].type == TK_IDENT
            );
          }
          tokens[nr_token].type = isDeref ? TK_DEREF : TK_MUT;
          nr_token++;
          break;
        
        case TK_IDENT:
          Assert(false, "not implemented");
          break;
        
        case TK_REG:
        case TK_NUM_HEX:
        case TK_IDENT:
          tokens[nr_token].type = rules[i].token_type;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          nr_token++;
          break;
        
        case TK_NUM_DEC:
          signal_positive = true;
          if (nr_token == 0) {
            signal_positive = true;
          } else if (nr_token == 1) {
            if (tokens[0].type == TK_MINUS) {
              signal_positive = false;
              nr_token--;
            } else if (tokens[0].type == TK_PLUS) {
              signal_positive = true;
              nr_token--;
            }
          } else {
            if (tokens[nr_token - 1].type == TK_MINUS || tokens[nr_token - 1].type == TK_PLUS) {
              if (!(tokens[nr_token - 2].type == TK_NUM_DEC || tokens[nr_token - 2].type == TK_NUM_HEX || tokens[nr_token - 2].type == TK_REG || tokens[nr_token - 2].type == TK_IDENT)) {
                signal_positive = tokens[nr_token - 1].type == TK_MINUS? false : true;
                nr_token--;
              }
            }
          }
          tokens[nr_token].type = TK_NUM_DEC;
          if (signal_positive) {
            strncpy(tokens[nr_token].str, substr_start, substr_len);
          } else {
            tokens[nr_token].str[0] = '-';
            strncpy(tokens[nr_token].str + 1, substr_start, substr_len);
          }
          nr_token++;
          break;
        
        default:
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;
        }
        break; // 找到一次即结束
      }
    }
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  if (nr_token == 0) {
    printf("empty expr\n");
    return false;
  }
  return true;
}

/*
根据tokens计算结果，使用分治法
*/
static inline int get_priority(int type) {
  switch (type) {
    case TK_LOGIC_OR: return 1;
    case TK_LOGIC_AND: return 2;
    case TK_LOGIC_EQ:
    case TK_LOGIC_NEQ: return 3;
    case TK_GE:
    case TK_LE:
    case TK_GT:
    case TK_LT: return 4;
    case TK_BIT_OR: return 5;
    case TK_BIT_AND: return 6;
    case TK_PLUS:
    case TK_MINUS: return 7;
    case TK_MUT:
    case TK_DIV: return 8;
    case TK_DEREF:
    case TK_BIT_NOT:
    case TK_LOGIC_NOT: return 9; // 一元运算符
    case TK_LBRACKET:
    case TK_RBRACKET: return 10;
    default: return 0;
  }
}

static inline bool isNum(int type) {
  return type == TK_NUM_DEC || type == TK_NUM_HEX || type == TK_REG || type == TK_IDENT;
}

/*
分治法，使用左闭右闭
1 拆分表达式
  找到优先级最低的最右侧的运算符
2 递归求解
  递归求解左右两侧的表达式
3 合并
  根据运算符合并左右两侧的表达式
*/
static word_t eval(int left, int right, bool *success) {
  *success = false;
  if (left > right) {
    Assert(false, "left > right");
  }
  if (left == right) { // 只有一个数字
    if (get_priority(tokens[left].type) != 0) {
      printf("invalid expression\n");
      *success = false;
      return 0;
    }
    switch (tokens[left].type) {
      case TK_NUM_DEC: return strtol(tokens[left].str, NULL, 10);
      case TK_NUM_HEX: return strtol(tokens[left].str, NULL, 16);
      case TK_REG: return isa_reg_str2val(tokens[left].str);
      case TK_IDENT: Assert(false, "not implemented");
      default: Assert(false, "invalid token type");
    }
  }

  int min_priority = 10;
  int op_index = -1;
  int bracket_count = 0;
  for (int i = left; i <= right; i++) {
    if (tokens[i].type == TK_LBRACKET) {
      bracket_count++;
    } else if (tokens[i].type == TK_RBRACKET) {
      bracket_count--;
    } else if (bracket_count == 0) {
      int priority = get_priority(tokens[i].type);
      if (priority <= min_priority && priority != 0) { // 得到最右侧值优先级最小的表达式
        min_priority = priority;
        op_index = i;
      }
    }
  }
  if (bracket_count != 0) {
    printf("bracket_not_closed\n");
    *success = false;
    return 0;
  }
  if (op_index == -1) { // 没有运算符， 且left < right
    // num () num ... 的组合，需要识别出正确的组合即单个num 或者整个是一个(expr)
    if (tokens[left].type == TK_LBRACKET && tokens[right].type == TK_RBRACKET) { // 整个是一个(expr)
      if (left + 1 == right) { // 空括号
        printf("invalid expression\n");
        *success = false;
        return 0;
      }
      return eval(left + 1, right - 1, success);
    } else { // 因为单个num前面已经处理过了，因此num, num num, num(), ()num等情况都是有问题的
      printf("invalid expression\n");
      *success = false;
      return 0;
    }
  }
  int op = tokens[op_index].type;
  if (op == TK_DEREF || op == TK_BIT_NOT || op == TK_LOGIC_NOT) {
    word_t val = eval(op_index + 1, right, success);
    if (!*success) {
      return 0;
    }
    if (op_index != left) {
      printf("invalid expression\n");
      *success = false;
      return 0;
    }
    switch (op) { // 一元运算符优先级最高，因此一元运算符一定被递归分隔后，位于left
      case TK_DEREF: 
      if (!in_pmem(val)) {
        printf("invalid memory access at 0x%x\n", val);
        *success = false;
        return 0;
        }
        return *(word_t*)vaddr_read(val, 4);
      case TK_BIT_NOT: return ~val;
      case TK_LOGIC_NOT: return !val;
    }
  }

  if (op_index == left || op_index == right) {
    printf("invalid expression\n");
    *success = false;
    return 0;
  }
  word_t left_val = eval(left, op_index - 1, success);
  if (!*success) {
    return 0;
  }
  word_t right_val = eval(op_index + 1, right, success);
  if (!*success) {
    return 0;
  }
  switch (op) {
    case TK_PLUS: return left_val + right_val;
    case TK_MINUS: return left_val - right_val;
    case TK_MUT: return left_val * right_val;
    case TK_DIV: 
      if (right_val == 0) {
        printf("divide by zero\n");
        *success = false;
        return 0;
      }
      return left_val / right_val;
    case TK_LOGIC_EQ: return left_val == right_val;
    case TK_LOGIC_NEQ: return left_val != right_val;
    case TK_LOGIC_AND: return left_val && right_val;
    case TK_LOGIC_OR: return left_val || right_val;
    case TK_GE: return left_val >= right_val;
    case TK_LE: return left_val <= right_val;
    case TK_GT: return left_val > right_val;
    case TK_LT: return left_val < right_val;
    case TK_BIT_AND: return left_val & right_val;
    case TK_BIT_OR: return left_val | right_val;
    case TK_BIT_NOT: return ~left_val;
    case TK_LOGIC_NOT: return !left_val;
    case TK_DEREF: 
      if (!in_pmem(left_val)) {
        printf("invalid memory access at 0x%x\n", left_val);
        *success = false;
        return 0;
      }
      return *(word_t*)vaddr_read(left_val, 4);
    default: Assert(false, "invalid operator");
  }
}
  

word_t expr(char *e, bool *success) {
  *success = false;  // 初始化为false，只有完全成功才设为true
  
  // 1. 词法分析
  if (!make_token(e)) {
    // make_token已经打印了错误信息
    return 0;
  }
  word_t result = eval(0, nr_token - 1, success);
  return result;
}