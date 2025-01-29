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
// 需要添加的头文件
#include <memory/vaddr.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 0,
  
  // 非运算符token
  TK_NUM,      // number
  TK_HEX,      // hex
  TK_OCT,      // oct
  TK_REG,      // $
  TK_IDENT,    // identifier
  
  // 运算符开始
  TK_OPERATOR_START,  // 运算符的起始标记
  
  // 二元运算符
  TK_PLUS,     // +
  TK_MINUS,    // -
  TK_DOT,      // * (乘法)
  TK_DIV,      // /
  TK_EQ,       // ==
  TK_UE,       // !=
  TK_GT,       // >
  TK_LT,       // <
  TK_GE,       // >=
  TK_LE,       // <=
  TK_AND,      // &&
  TK_OR,       // ||
  TK_BIT_AND,  // &
  TK_BIT_OR,   // |
  
  // 一元运算符
  TK_BIT_NOT,  // ~
  TK_DEREF,    // * (解引用)
  
  // 括号
  TK_LBRACKET, // (
  TK_RBRACKET, // )
  
  TK_OPERATOR_END    // 运算符的结束标记
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = { // 从上至下，确保优先级
  {" +",  TK_NOTYPE},    // spaces
  // number
  {"0x[0-9abcdefABCDEF]+", TK_HEX}, // HEX
  {"0[01234567]+", TK_OCT}, //OCT
  {"\\+ *\\[-\\+]?[0-9]+", TK_PLUS},    // plus followed a signed number
  {"- *[-\\+]?[0-9]+", TK_MINUS},        // minus followed a signed number
  {"[+-]?[0-9]+", TK_NUM},     // number
  {"\\+", TK_PLUS},      // plus
  {"-",   TK_MINUS},     // minus

  {">",   TK_GT},        // greater than
  {"<",   TK_LT},        // less than
  {">=",  TK_GE},        // greater equal
  {"<=",  TK_LE},        // less equal
  {"&",   TK_BIT_AND},   // bitwise and
  {"\\|",   TK_BIT_OR},    // bitwise or
  {"~",   TK_BIT_NOT},   // bitwise not
  {"==",  TK_EQ},        // equal
  {"!=",  TK_UE},        // unequal
  {"&&",  TK_AND},       // logical and
  {"\\|\\|",  TK_OR},    // logical or
  {"\\*", TK_DOT},       // multiply or dereference
  {"/",   TK_DIV},       // divide
  {"\\(", TK_LBRACKET},  // left bracket
  {"\\)", TK_RBRACKET},  // right bracket
  {"\\$(0|ra|sp|gp|tp|t[0-6]|s([0-9]|10|11)|a[0-7])", TK_REG},  // register
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
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        // 检查token数组是否溢出
        if (nr_token >= 32) {
          printf("Error: Too many tokens\n");
          return false;
        }

        // 检查token长度
        if (substr_len >= 32) {
          printf("Error: Token too long\n");
          return false;
        }

        // 根据不同的token类型进行处理
        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;

          case TK_NUM:
          case TK_HEX:
          case TK_OCT: {
            // 数字处理需要特别注意前缀
            if (rules[i].token_type == TK_HEX && substr_len > 2) {
              // 去掉0x前缀
              memcpy(tokens[nr_token].str, substr_start + 2, substr_len - 2);
              tokens[nr_token].str[substr_len - 2] = '\0';
            } else if (rules[i].token_type == TK_OCT && substr_len > 1) {
              // 去掉0前缀
              memcpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
              tokens[nr_token].str[substr_len - 1] = '\0';
            } else {
              memcpy(tokens[nr_token].str, substr_start, substr_len);
              tokens[nr_token].str[substr_len] = '\0';
            }
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          }

          case TK_REG: {
            // 寄存器名处理，去掉$前缀
            if (substr_len > 1) {
              memcpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
              tokens[nr_token].str[substr_len - 1] = '\0';
            } else {
              printf("Error: Invalid register name\n");
              return false;
            }
            tokens[nr_token].type = TK_REG;
            nr_token++;
            break;
          }

          case TK_PLUS:
          case TK_MINUS: {
            // 检查是否是一元运算符
            if (nr_token == 0 || 
                (tokens[nr_token-1].type > TK_OPERATOR_START && 
                 tokens[nr_token-1].type < TK_OPERATOR_END)) {
              // 如果是表达式开始或前一个token是运算符，则当前的+/-是一元运算符
              // 将其与下一个数字合并
              char next_char = e[position + 1];
              if (next_char == ' ') {
                // 跳过空格
                int j = position + 1;
                while (e[j] == ' ') j++;
                next_char = e[j];
              }
              if (isdigit(next_char)) {
                // 如果下一个字符是数字，暂存当前符号
                tokens[nr_token].str[0] = substr_start[0];
                tokens[nr_token].str[1] = '\0';
                tokens[nr_token].type = rules[i].token_type;
                nr_token++;
              }
            } else {
              // 作为二元运算符处理
              tokens[nr_token].type = rules[i].token_type;
              tokens[nr_token].str[0] = '\0';
              nr_token++;
            }
            break;
          }

          case TK_DOT: {
            // 检查是否是解引用运算符
            if (nr_token == 0 || 
                (tokens[nr_token-1].type > TK_OPERATOR_START && 
                 tokens[nr_token-1].type < TK_OPERATOR_END)) {
              tokens[nr_token].type = TK_DEREF;
            } else {
              tokens[nr_token].type = TK_DOT;  // 作为乘法运算符
            }
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          }

          case TK_AND:
          case TK_OR:
          case TK_EQ:
          case TK_UE:
          case TK_GT:
          case TK_LT:
          case TK_GE:
          case TK_LE:
          case TK_BIT_AND:
          case TK_BIT_OR:
          case TK_BIT_NOT:
          case TK_DIV:
          case TK_LBRACKET:
          case TK_RBRACKET: {
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          }

          default:
            printf("Error: Unknown token type\n");
            return false;
        }

        position += substr_len;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("Error: No match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/*
<expr> ::= <decimal-number>
  | <hexadecimal-number>    # 以"0x"开头
  | <reg_name>              # 以"$"开头
  | "(" <expr> ")"
  | <expr> "+" <expr>
  | <expr> "-" <expr>
  | <expr> "*" <expr>
  | <expr> "/" <expr>
  | <expr> "&" <expr>       # 按位与
  | <expr> "|" <expr>       # 按位或
  | "~" <expr>              # 按位取反
  | <expr> ">" <expr>       # 大于
  | <expr> "<" <expr>       # 小于
  | <expr> ">=" <expr>      # 大于等于
  | <expr> "<=" <expr>      # 小于等于
  | <expr> "==" <expr>      # 相等
  | <expr> "!=" <expr>      # 不等
  | <expr> "&&" <expr>      # 逻辑与
  | <expr> "||" <expr>      # 逻辑或
  | "*" <expr>              # 指针解引用
  | "$" <expr>              # 寄存器引用
*/

enum level {
  LVL_L_OR = 1,        // ||
  LVL_L_AND = 2,       // &&
  LVL_EQ = 3,          // ==, !=
  LVL_CMP = 4,         // >, <, >=, <=
  LVL_B_OR = 5,        // |
  LVL_B_AND = 6,       // &
  LVL_ADD = 7,         // +, -
  LVL_DIV = 8,         // *, /
  LVL_UNARY = 9,       // ~ (一元运算符), * (解引用)
  LVL_PAREN = 10       // ()
};

static inline int get_priority(int op_type) {
  switch (op_type) {
    case TK_OR: return LVL_L_OR;
    case TK_AND: return LVL_L_AND;
    case TK_EQ:
    case TK_UE: return LVL_EQ;
    case TK_GT:
    case TK_LT:
    case TK_GE:
    case TK_LE: return LVL_CMP;
    case TK_BIT_OR: return LVL_B_OR;
    case TK_BIT_AND: return LVL_B_AND;
    case TK_PLUS:
    case TK_MINUS: return LVL_ADD;
    case TK_DOT:
    case TK_DIV: return LVL_DIV;
    case TK_DEREF:
    case TK_BIT_NOT: return LVL_UNARY;
    case TK_LBRACKET:
    case TK_RBRACKET: return LVL_PAREN;
    default: return 0;  // 非运算符
  }
}

static word_t cal(short left, short right, bool *success);

word_t expr(char *e, bool *success) {
  *success = false;  // 初始化为false，只有完全成功才设为true
  
  // 1. 词法分析
  if (!make_token(e)) {
    // make_token已经打印了错误信息
    return 0;
  }

  // 2. 处理解引用和乘法
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_DOT) {
      if (i == 0 || 
          (tokens[i-1].type > TK_OPERATOR_START && 
           tokens[i-1].type < TK_OPERATOR_END)) {
        tokens[i].type = TK_DEREF;
      }
    }
  }

  // 3. 检查括号匹配
  int bracket_count = 0;
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_LBRACKET) {
      bracket_count++;
    } else if (tokens[i].type == TK_RBRACKET) {
      bracket_count--;
      if (bracket_count < 0) {
        printf("Error: Unmatched right bracket\n");
        return 0;
      }
    }
  }
  if (bracket_count != 0) {
    printf("Error: Unmatched left bracket\n");
    return 0;
  }

  // 4. 计算表达式
  bool eval_success = true;  // 用于跟踪计算过程是否成功
  word_t result = cal(0, nr_token - 1, &eval_success);
  if (!eval_success) {
    // cal函数已经打印了具体错误信息
    return 0;
  }

  // 5. 所有检查都通过，设置成功标志
  *success = true;
  return result;
}

static int find_main_op(int left, int right) {
  int op = -1;
  int min_prior = 100;
  int in_paren = 0;

  // 从右向左扫描，这样可以正确处理同优先级的左结合
  for (int i = right; i >= left; i--) {
    if (tokens[i].type == TK_RBRACKET) {
      in_paren++;
      continue;
    }
    if (tokens[i].type == TK_LBRACKET) {
      in_paren--;
      continue;
    }
    if (in_paren > 0) continue;

    // 获取当前token的优先级
    int curr_prior = get_priority(tokens[i].type);
    if (curr_prior > 0) {  // 是运算符
      // 对于相同优先级，我们选择最左边的运算符（右结合）
      // 对于一元运算符，我们需要特殊处理
      if (curr_prior == LVL_UNARY) {
        if (i == left || (tokens[i-1].type > TK_OPERATOR_START && 
                         tokens[i-1].type < TK_OPERATOR_END)) {
          if (curr_prior < min_prior) {
            min_prior = curr_prior;
            op = i;
          }
        }
      } else {
        if (curr_prior < min_prior) {
          min_prior = curr_prior;
          op = i;
        }
      }
    }
  }
  return op;
}

// 左闭右闭
static word_t cal(short left, short right, bool *success) {
  if (!*success) return 0;  // 如果已经失败，直接返回
  
  if (left > right) {
    printf("Error: Invalid expression range\n");
    *success = false;
    return 0;
  }

  // 处理单个token的情况
  if (left == right) {
    switch (tokens[left].type) {
      case TK_NUM: {
        char *endptr;
        word_t val = strtoul(tokens[left].str, &endptr, 10);
        if (*endptr != '\0') {
          printf("Error: Invalid decimal number format: %s\n", tokens[left].str);
          *success = false;
          return 0;
        }
        return val;
      }
      case TK_HEX: {
        char *endptr;
        word_t val = strtoul(tokens[left].str, &endptr, 16);
        if (*endptr != '\0') {
          printf("Error: Invalid hex number format: %s\n", tokens[left].str);
          *success = false;
          return 0;
        }
        return val;
      }
      case TK_OCT: {
        char *endptr;
        word_t val = strtoul(tokens[left].str, &endptr, 8);
        if (*endptr != '\0') {
          printf("Error: Invalid octal number format: %s\n", tokens[left].str);
          *success = false;
          return 0;
        }
        return val;
      }
      case TK_REG: {
        bool reg_success = true;
        word_t val = isa_reg_str2val(tokens[left].str, &reg_success);
        if (!reg_success) {
          printf("Error: Invalid register name: %s\n", tokens[left].str);
          *success = false;
          return 0;
        }
        return val;
      }
      default:
        printf("Error: Invalid token type at position %d\n", left);
        *success = false;
        return 0;
    }
  }

  // 处理括号
  if (tokens[left].type == TK_LBRACKET && tokens[right].type == TK_RBRACKET) {
    return cal(left + 1, right - 1, success);
  }

  // 寻找主运算符
  int op = find_main_op(left, right);
  if (op < 0) {
    printf("Error: Cannot find main operator between positions %d and %d\n", left, right);
    *success = false;
    return 0;
  }

  // 处理一元运算符
  if (get_priority(tokens[op].type) == LVL_UNARY) {
    word_t val = cal(op + 1, right, success);
    if (!*success) return 0;

    switch (tokens[op].type) {
      case TK_BIT_NOT: return ~val;
      case TK_DEREF: {
        // 添加地址检查
        if (val % 4 != 0) {
          printf("Error: Misaligned memory access at address 0x%x\n", val);
          *success = false;
          return 0;
        }
        return vaddr_read(val, 4);
      }
      default:
        printf("Error: Unknown unary operator at position %d\n", op);
        *success = false;
        return 0;
    }
  }

  // 处理二元运算符
  word_t val1 = cal(left, op - 1, success);
  if (!*success) return 0;
  
  word_t val2 = cal(op + 1, right, success);
  if (!*success) return 0;

  switch (tokens[op].type) {
    case TK_PLUS: return val1 + val2;
    case TK_MINUS: return val1 - val2;
    case TK_DOT: return val1 * val2;
    case TK_DIV:
      if (val2 == 0) {
        printf("Error: Division by zero at position %d\n", op);
        *success = false;
        return 0;
      }
      return val1 / val2;
    case TK_EQ: return val1 == val2;
    case TK_UE: return val1 != val2;
    case TK_GT: return val1 > val2;
    case TK_LT: return val1 < val2;
    case TK_GE: return val1 >= val2;
    case TK_LE: return val1 <= val2;
    case TK_AND: return val1 && val2;
    case TK_OR: return val1 || val2;
    case TK_BIT_AND: return val1 & val2;
    case TK_BIT_OR: return val1 | val2;
    default:
      printf("Error: Unknown binary operator at position %d\n", op);
      *success = false;
      return 0;
  }
}