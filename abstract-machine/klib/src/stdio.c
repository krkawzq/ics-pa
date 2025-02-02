#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 辅助函数：打印数字
static int print_num(unsigned int num, int base) {
    char digits[] = "0123456789ABCDEF";
    char buf[32];
    int i = 0;
    int count = 0;
    
    if (num == 0) {
        putch('0');
        return 1;
    }
    
    while (num) {
        buf[i++] = digits[num % base];
        num /= base;
    }
    
    count = i;
    while (--i >= 0) {
        putch(buf[i]);
    }
    return count;  // 返回打印的字符数
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = 0;
    char ch;
    
    while ((ch = *fmt++)) {
        if (ch != '%') {
            putch(ch);
            ret++;
            continue;
        }
        
        ch = *fmt++;
        switch(ch) {
            case 'd': {
                int num = va_arg(ap, int);
                if (num < 0) {
                    putch('-');
                    ret++;
                    num = -num;
                }
                ret += print_num(num, 10);
                break;
            }
            case 'x': {
                unsigned int num = va_arg(ap, unsigned int);
                ret += print_num(num, 16);
                break;
            }
            case 's': {
                char *str = va_arg(ap, char*);
                while (*str) {
                    putch(*str++);
                    ret++;
                }
                break;
            }
            case 'c': {
                putch(va_arg(ap, int));
                ret++;
                break;
            }
            default:
                putch(ch);
                ret++;
                break;
        }
    }
    
    va_end(ap);
    return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
