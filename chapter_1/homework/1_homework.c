#include <stdio.h>
#include <limits.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len)
{
    // sizeof的返回值为size_t类型
    size_t i;
    for (i = 0; i < len; i++)
    {
        printf(" %.2x", start[i]);
    }
    printf("\r\n");
}

// 2.55
// 2.56 ：用不同的示例值来运行show_bytes的代码
void show_different_examples() 
{
    int x = 123456;
    float y = 123.456f;
    double z = 123456.789;

    show_bytes((byte_pointer)&x, sizeof(int));
    show_bytes((byte_pointer)&y, sizeof(float));
    show_bytes((byte_pointer)&z, sizeof(double));
}

// 2.57 : 编写程序show_short, show_long, show_double,他们分别打印类型为short, long, double的c语言对象的字节表示

void show_short(short x) 
{
    show_bytes((byte_pointer)&x, sizeof(short));
}
void show_long(long x) 
{
    show_bytes((byte_pointer)&x, sizeof(long));
}
void show_double(double x) 
{
    show_bytes((byte_pointer)&x, sizeof(double));
}

// 2.58 : 编写过程is_little_endian, 判断机器的字节序,当在小端法机器上编译和运行时返回1, 否则返回0
int is_little_endian() 
{
    unsigned int x = 1;
    byte_pointer p = (byte_pointer)&x;
    return p[0] == 1;
}

// 2.59 编写一个c表达式,他生成一个字，由x的最低有效字节和y中剩下的字节组成，对于运算数x = 0x89abcdef和y = 0x76543210,该表达式的值为0x765432ef
unsigned int combine_bytes(unsigned int x, unsigned int y) 
{
    return (y & 0xFFFFFF00) | (x & 0x000000FF);
}

// 2.60 : eg : replace_byte(0x12345678,2,0xAB) 结果为0x12AB5678,replace_byte(0x12345678,0,0xAB) 结果为0x123456AB
unsigned int replace_byte(unsigned int x, int i, unsigned char b) 
{
    unsigned int num = 0xFFFFFFFF & ~(0xFF << (i * 8));
    return (x & num) | (b << (i * 8));
}

// 位级整数编码规则：
// 整数用补码的形式来表示
// 有符号数的右移是符号右移
// 数据类型int是 w 位长的，可以用sizeof(int) << 3来计算 w

// ban
// if,?:,循环,分支,函数,宏
// 除法,模运算,乘法
// 比较

// 允许
// 位级,；逻辑
// 左移,右移 但是位移量自能在0 和 w - 1
// 加,减
// == !=
// INT_MAX, INT_MIN
// 对int和unsigned int的转换

// 2.61 : 
int xallbitis1(int x)
{
    return !~x;
}

int xallbitis0(int x)
{
    return !x;
}

int xlsbis1(int x)
{
    return !((x & 0xff)^0xff);
}

int xmsbis0(int x)
{
    return !((x & (0xff << (sizeof(int) - 1) * 8)) | 0x00);
}

// 2.62
int int_shifts_are_arithmetic()
{
    int x = -1;
    return (x >> 1) == -1;
}

// 2.63
unsigned srl(unsigned x, int k)
{
    /* Perform shift arithmetically */
    unsigned xsra = (int)x >> k;
    int w = sizeof(int) << 3;
    unsigned mask = (1u << (w - k)) - 1;
    return xsra & mask;
}

unsigned sra(unsigned x, int k)
{
    /* Perform shift logically */
    int xsrl = (unsigned)x >> k;
    int w = sizeof(int) << 3;
    unsigned mask = (1u << (w - k)) - 1;
    return xsrl | ~mask;
}

// 2.64
/* Return 1 when any odd bit of x equals 1; 0 otherwise. Assume w = 32 */
int any_odd_one(unsigned x)
{
    unsigned mask = 0xAAAAAAAA;
    return !!(x & mask);
}

// 2.66

/*
 * Generate mask indicating leftmost 1 in x. Assume w = 32.
 * For example, 0xFF00 -> 0x8000, and 0x6600 --> 0x4000.
 * If x = 0, then return 0.
*/
int leftmost_one(unsigned x)
{
    // 先将x转换成形如[0...011...1]的位向量
    if (x == 0)
        return 0;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x & ~(x >> 1);

}

int main() 
{
    printf("Hello, World!\r\n");
    show_different_examples();
    printf("当前机器是否为小端序:%d\r\n", is_little_endian());
    printf("replace_byte(0x12345678,2,0xAB) = %#x\r\n", replace_byte(0x12345678, 2, 0xAB));
    printf("replace_byte(0x12345678,0,0xAB) = %#x\r\n", replace_byte(0x12345678, 0, 0xAB));
    unsigned int result = replace_byte(0x12345678, 2, 0xAB);
    show_bytes((byte_pointer)&result, sizeof(unsigned int));
    result = replace_byte(0x12345678, 0, 0xAB);
    show_bytes((byte_pointer)&result, sizeof(unsigned int));
    printf("xallbitis1(0xFFFFFFFF) = %d\r\n", xallbitis1(0xFFFFFFFF));
    printf("xallbitis1(0xFFF11FFF) = %d\r\n", xallbitis1(0xFFF11FFF));
    printf("xallbitis0(0x00000000) = %d\r\n", xallbitis0(0x00000000));
    printf("xallbitis0(0x00011000) = %d\r\n", xallbitis0(0x00011000));
    printf("xlsbis1(0x002000FF) = %d\r\n", xlsbis1(0x002000FF));
    printf("xlsbis1(0x002000F1) = %d\r\n", xlsbis1(0x002000F1));
    printf("xmsbis0(0x002000F1) = %d\r\n", xmsbis0(0x002000F1));
    printf("xmsbis0(0x012000F1) = %d\r\n", xmsbis0(0x012000F1));
    printf("int_shifts_are_arithmetic = %d\r\n", int_shifts_are_arithmetic());
    return 0;
}