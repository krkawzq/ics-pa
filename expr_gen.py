'''
用于生成表达式
由于寄存器与解引用的访问无法直接使用c语言直接计算
因此单独测试寄存器访问和解引用
常规计算，使用c语法
当前脚本用于生成常规表达式
operator：
+ - * /
| & ~
== !=
> < >= <=
&& ||
( )
'''

import os
unary_operators = ['~']  # bitwise NOT
binary_operators = ['+', '-', '*', '/', '|', '&']  # arithmetic and bitwise operators
logical_operators = ['==', '!=', '>', '<', '>=', '<=', '&&', '||']

# 使用随机概率生成
import random
import subprocess
import tempfile
# 最大递归深度
max_depth = 5
'''
递归规则
1 每次递归尝试生成一个二元表达式，并使用括号包裹
2 如果当前深度为0，则生成一个随机整数
3 深度不为0，有概率直接生成一个数 alpha -> num, 1 - alpha -> expr
4 二元main op 随机概率： theta = 0.8 -> b, 1 - theta -> logic
5 每个表达式有概率有一元运算符修饰： ~ gamma = 0.1
'''

alpha = 0.5
theta = 0.8
gamma = 0.1
beta = 0.5 # 使用0x的概率



def generate_expression(depth) -> str:
    if depth == 0:
        if random.random() < beta:
            return "0x{:x}".format(random.randint(0, 100))
        else:
            return str(random.randint(0, 100))
    else:
        # 生成第一个表达式
        if random.random() < gamma:
            expr1 = random.choice(unary_operators)
        else:
            expr1 = ""

        if random.random() < alpha:
            if random.random() < beta:
                expr1 += "0x{:x}".format(random.randint(0, 100))
            else:
                expr1 += str(random.randint(0, 100))
        else:
            expr1 += '(' + generate_expression(depth - 1) + ')'

        # 生成运算符
        if random.random() < theta:
            op = random.choice(binary_operators)
        else:
            op = random.choice(logical_operators)

        # 生成第二个表达式
        if random.random() < gamma:
            expr2 = random.choice(unary_operators)
        else:
            expr2 = ""

        if random.random() < alpha:
            if random.random() < beta:
                expr2 += "0x{:x}".format(random.randint(0, 100))
            else:
                expr2 += str(random.randint(0, 100))
        else:
            expr2 += '(' + generate_expression(depth - 1) + ')'

        return expr1 + ' ' + op + ' ' + expr2


def evaluate_with_c(expr: str) -> int:
    
    # 创建一个临时C文件
    with tempfile.NamedTemporaryFile(suffix='.c', mode='w', delete=False) as f:
        f.write(f'''
        #include <stdio.h>
        int main() {{
            printf("%d\\n", {expr});
            return 0;
        }}
        ''')
    
    # 编译并运行
    try:
        subprocess.run(['gcc', f.name, '-o', 'temp_expr.exe'])
        result = subprocess.check_output(['./temp_expr.exe']).decode().strip()
        return int(result)
    except Exception as e:
        print(f"C计算出错: {e}")
        return None
    finally:
        os.unlink(f.name)
        os.unlink('temp_expr.exe')

# 在主代码中使用
expr = generate_expression(max_depth)
print(f"生成的表达式: {expr}")
result = evaluate_with_c(expr)
if result is not None:
    print(f"计算结果: {result}")


max_iter = 100

def main():
    # 检查文件是否存在,不存在则创建
    if not os.path.exists('test.input'):
        open('test.input', 'w').close()
    else:
        # 文件存在则清空内容
        with open('test.input', 'w') as f:
            f.write('')
            
    for i in range(max_iter):
        expr = generate_expression(max_depth)
        result = evaluate_with_c(expr)
        if result is not None:
            with open('test.input', 'a') as f:
                f.write(f"{expr}\n{result}\n")

'''
在Linux上使用的test函数
打开 /nemu/build/riscv64-nemu-interpreter
使用这个程序，测试test.input中的表达式
每次取一行表达式输入给程序
得到stdout作为输出
比较输出的值和test.input中的下一行
如果相等，则计数器加1
'''
def test():
    with open('test.input', 'r') as f:
        lines = f.readlines()
    for i in range(len(lines) // 2):
        result = os.popen(f'echo "{lines[i * 2]}" | /nemu/build/riscv64-nemu-interpreter').read()
        
        if int(result) != int(lines[i * 2 + 1]):
            print(f"第{i}行错误，正确值为{lines[i * 2 + 1]}，实际值为{result}")
        


if __name__ == "__main__":
    test()

