#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

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

///////////////////////////////////////////////////////
// homework    * : 已订正答案    x : 不会
////////////////////////////////////////////////////////

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

int offer_is_little_endian()
{
    int test_num = 0xff;
    byte_pointer byte_start = (byte_pointer)&test_num;

    if (byte_start[0] == 0xff)
    {
        return 1;
    }
    return 0;
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

unsigned offer_replace_byte(unsigned x, int i, unsigned char b)
{
    if (i < 0)
    {
        printf("error: i is negetive\n");
        return x;
    }
    if (i > sizeof(unsigned) - 1)
    {
        printf("error: too big i");
        return x;
    }

    // 1 byte has 8 bits, << 3 means * 8
    unsigned mask = ((unsigned)0xFF) << (i << 3);
    unsigned pos_byte = ((unsigned)b) << (i << 3);

    return (x & ~mask) | pos_byte;
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

// 2.61 : *
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
//-------------------offer solution-----------------------
int offerA(int x)
{
    return !~x;
}

int offerB(int x)
{
    return !x;
}

int offerC(int x)
{
    return offerA(x | ~0xff);
}

int offerD(int x)
{
    return offerB((x >> ((sizeof(int) - 1) << 3)) & 0xff);
}
//--------------------------
// 2.62 *
int int_shifts_are_arithmetic()
{
    int x = -1;
    return (x >> 1) == -1;
}
//-------------------------------offer solution-----------------------
int offer_int_shifts_are_arithemetic()
{
    int num = -1;
    return !(num ^ (num >> 1));
}
//------------------------------------------------------------------
// 2.63 *
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
//------------------------------offer solution-----------------------
unsigned offer_srl(unsigned x, int k)
{
    unsigned xsra = (int)x >> k;

    int w = sizeof(int) << 3;
    int mask = (int)-1 << (w - k);
    return xsra & ~mask;
}

int offer_sra(int x, int k)
{
    int xsrl = (unsigned)x >> k;

    int w = sizeof(int) << 3;
    int mask = (int)-1 << (w - k);
    // let mask remain unchanged when the first bit of x is 1, otherwise 0.
    int m = 1 << (w - 1);
    mask &= !(x & m) - 1;
    return xsrl | mask;
}
//------------------------------------------------------------------
// 2.64 *
/* Return 1 when any odd bit of x equals 1; 0 otherwise. Assume w = 32 */
int any_odd_one(unsigned x)
{
    unsigned mask = 0xAAAAAAAA;
    return !!(x & mask);
}

// 2.66  *

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

int offer_leftmost_one(unsigned x)
{
    /*
     * first, generate a mask that all bits after leftmost one are one
     * e.g. 0xFF00 -> 0xFFFF, and 0x6000 -> 0x7FFF
     * If x = 0, get 0
     */
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    /*
     * then, do (mask >> 1) + (mask && 1), in which mask && 1 deals with case x = 0, reserve leftmost bit one
     * that's we want
     */
    return (x >> 1) + (x && 1);
}

// 2.67  *
/* The following code does not run properly on some machine */
// A:如果右移数值为负或大于或等于字长则结果未定义

int bad_int_size_is_32_B()
{
    /* Set most significant bit (msb) of 32-bit machine */
    int set_msb = 1 << 31;
    /* Shift past msb of 32-bit word */
    int beyond_msb = set_msb << 1;
    /* set_msb is nonzero when word size >= 32 beyond_msb is zero when word size <= 32 */
    return set_msb && !beyond_msb;
}

int bad_int_size_is_32_C()
{
    /* Set most significant bit (msb) of 32-bit machine */
    int set_msb = 1 << 15 << 15 << 1;
    /* Shift past msb of 32-bit word */
    int beyond_msb = set_msb << 1;
    /* set_msb is nonzero when word size >= 32 beyond_msb is zero when word size <= 32 */
    return set_msb && !beyond_msb;
}

// 2.68  *
/*
 * Mask with least signficant n bits set to 1.
 * Examples: n = 6 --> 0x3F, n = 17 --> 0x1FFFF
 * Assume 1 <= n <= w 
*/
int lower_one_mask(int n)
{
    // 边界有问题 w = 32
    return (1 << n) - 1;
}
//------------------------------offer solution-----------------------
/*
 * Mask with least signficant n bits set to 1
 * Example: n = 6 -> 0x3F, n = 17 -> 0x1FFFF
 * Assume 1 <= n <= w
 */
int offer_lower_one_mask(int n)
{
    int w = sizeof(int) << 3;
    return (unsigned)-1 >> (w - n);
}
//------------------------------------------------------------------
// 2.69 *
/*
 * Do rotating left shift. Assume 0 <= n < w
 * Example: when x = 0x123455678 and w = 32:
 * n = 4 --> 0x23456781, n = 20 --> 0x67812345
 */
unsigned rotate_left(unsigned x, int n)
{
    return (x << n) | (x >> (32 - n));
}
//------------------------------offer solution-----------------------
/*
 * Do rotate left shift. Assume 0 <= n < w
 * Example when x = 0x12345678 and w = 32:
 *   n = 4 -> 0x23456781, n = 20 -> 0x67812345
 */
unsigned offer_rotate_left(unsigned x, int n)
{
    int w = sizeof(unsigned) << 3;
    /* pay attention when n == 0 */
    return x << n | x >> (w - n - 1) >> 1;
}
//------------------------------------------------------------------
// 2.70  *
/*
 * Return 1 when x can be represented as an n-bit, 2's-complement
 * number; 0 otherwise
 * Assume 1 <= n <= w
 */
int fits_bits(int x, int n)
{
    return !((unsigned int)x >> (n - 1) >> 1);
}
//------------------------------offer solution-----------------------
int offer_fits_bits(int x, int n)
{
    /*
     * 1 <= n <= w
     *
     * assume w = 8, n = 3
     * if x > 0
     *   0b00000010 is ok, 0b00001010 is not, and 0b00000110 is not yet (thanks itardc@163.com)
     * if x < 0
     *   0b11111100 is ok, 0b10111100 is not, and 0b11111000 is not yet
     *
     * the point is
     *   x << (w-n) >> (w-n) must be equal to x itself.
     *
     */
    int w = sizeof(int) << 3;
    int offset = w - n;
    return (x << offset >> offset) == x;
}
//------------------------------------------------------------------
// 2.71 *
/* Declaration of data type where 4 bytes are packed into an unsigned */
typedef unsigned packed_t;

/* Extract byte from word.Return as signed integer */
int xbyte(packed_t word, int bytenum)
{
    return (int)word << ((3 - bytenum) << 3) >> 24;
}

//A :没有处理符号位

// 2.72  *
/* Copy integer into buffer if space is available */
/* WARNING : The following code is buggy */
void copy_int(int val, void *buf, int maxbytes)
{
    if (maxbytes-sizeof(val) >= 0)
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}
// A : maxbytes - sizeof(val) 可能会溢出变成负数
//
void copy_int_fix(int val, void *buf, int maxbytes)
{
    // 应该将 sizeof(val) 转换成int类型再进行比较
    if (maxbytes >= sizeof(val))
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}

void offer_copy_int(int val, void *buf, int maxbytes)
{
    /* compare two signed number, avoid someone set maxbytes a negetive value */
    if (maxbytes >= (int)sizeof(val))
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}

// 2.73 *
/* Addition that saturates to Tmin or Tmax */
int saturating_add(int x, int y)
{
    int sum = x + y;
    int w = sizeof(int) << 3;
    // sign bit : 0 正数 1 负数 
    int sign_x = !!(x >> (w - 1));
    int sign_y = !!(y >> (w - 1));
    int sign_sum = !!(sum >> (w - 1));
    int overflow_1 = (!sign_x & !sign_y) & sign_sum; // 正溢出
    int overflow_2 = (sign_x & sign_y) & !sign_sum; // 负溢出
    int tmax = ~(1 << (w - 1));
    int tmin = 1 << (w - 1);
    int temp = (overflow_1 && (sum = tmax) || (overflow_2 && (sum = tmin)) || (!(overflow_1 | overflow_2) && sum));
    return sum;
}
//------------------------------offer solution-----------------------
int saturating_add(int x, int y)
{
    int sum = x + y;
    int sig_mask = INT_MIN;
    /*
     * if x > 0, y > 0 but sum < 0, it's a positive overflow
     * if x < 0, y < 0 but sum >= 0, it's a negetive overflow
     */
    int pos_over = !(x & sig_mask) && !(y & sig_mask) && (sum & sig_mask);
    int neg_over = (x & sig_mask) && (y & sig_mask) && !(sum & sig_mask);

    (pos_over && (sum = INT_MAX) || neg_over && (sum = INT_MIN));

    return sum;
}
//------------------------------------------------------------------

// 2.74 *
/* Determine whether arguments can be subtracted without overflow */
int tsub_ok(int x, int y)
{
    // 没有处理 y = INT_MIN 的情况
    int y_neg = -y;
    int sum = x + y_neg;
    int w = sizeof(int) << 3;
    int sign_x = !!(x >> (w - 1));
    int sign_y = !!(y_neg >> (w - 1));
    int sign_sum = !!(sum >> (w - 1));
    int overflow = ((sign_x & sign_y) & !sign_sum) || ((!sign_x & !sign_y) & sign_sum); // 溢出
    return !overflow;
}
//------------------------------offer solution-----------------------
/* Determine whether arguments can be substracted without overflow */
int offer_tsub_ok(int x, int y)
{
    int res = 1;

    (y == INT_MIN) && (res = 0);
    // if (y == INT_MIN) res = 0;

    int sub = x - y;
    int pos_over = x > 0 && y < 0 && sub < 0;
    int neg_over = x < 0 && y > 0 && sub > 0;

    res = res && !(pos_over || neg_over);

    return res;
}
//------------------------------------------------------------------
// 2.75 *
/*
unsigned int unsigned_high_prod(unsigned int x, unsigned int y)
{
    unsigned long long int temp =  (unsigned long long int)x * (unsigned long long int)y;
    return temp >> 31 >> 1;
}
int signed_high_prod(int x, int y)
{
    long long int temp =  (long long int)x * (long long int)y;
    return temp >> 31 >> 1;
}*/

int signed_high_prod(int x, int y)
{
    int64_t mul = (int64_t)x * y;
    return (mul >> 32);
}

// xu*yu = (xs + x31 * 2^32) * (ys + y31 * 2^32)
//     = xs*ys + x31*ys*2^32 + y31*xs*2^32 + x31*y31*2^64
// 高32位 = signed_high_prod(x,y) + x31*ys + y31*xs
unsigned unsigned_high_prod(unsigned x, unsigned y)
{
    int sig_x = x >> 31;
    int sig_y = y >> 31;
    int signed_prod = signed_high_prod(x, y);
    return signed_prod + sig_x * y + sig_y * x;
}

/* a theorically correct version to test unsigned_high_prod func */
unsigned another_unsigned_high_prod(unsigned x, unsigned y)
{
    uint64_t mul = (uint64_t)x * y;
    return (mul >> 32);
}

// 2.76
void *calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0)
        return NULL;
    size_t total_size = nmemb * size;
    // 没有检查溢出
    void *ptr = malloc(total_size);
    if (ptr)
        memset(ptr, 0, total_size);
    return ptr;
}
//-----------offer solution -----------
/* rename to avoid conflict */
void *another_calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0)
    {
        return NULL;
    }
    size_t buf_size = nmemb * size;
    /* a good way to check overflow or not */
    if (nmemb == buf_size / size)
    {
        void *ptr = malloc(buf_size);
        if (ptr != NULL)
        {
            memset(ptr, 0, buf_size);
        }
        return ptr;
    }
    return NULL;
}
//--------------------------------------
// 2.77

// A : K = 17 : x << 4 + x
// B : k = -7 : -( x << 3 - x)
// c : K = 60 : x << 6 - x << 2
// D : K = -112 : -( x << 7 - x << 4)

// 2.78  *
/* Divide by power of 2. Assume 0 <= k <= w-1 */
int divide_power2(int x, int k)
{
    int is_neg = x & INT_MIN;
    (is_neg) && (x = x + (1 << k) - 1);
    return x >> k;
}

// 2.79 *
int my_mul3div4(int x)
{
    int mul3 = (x << 1) + x;
    int sign = mul3 >> 31;
    // 生成掩码
    int bias = sign & 3;
    return (mul3 + bias) >> 2;
}

int mul3div4(int x)
{
    int mul3 = (x << 1) + x;
    return divide_power2(mul3, 2);
}

// 2.80  *  ×   题目有误,是计算 x * 3/4  不是 3/(4x)
int my_threefouths(int x)
{
    int sign = x >> 31;
    int bias = sign & 3;
    int div4 = (x + bias) >> 2;
    return div4 + (div4 << 1);
}

/*
 * calculate 3/4x, no overflow, round to zero
 *
 * no overflow means divide 4 first, then multiple 3, diffrent from 2.79 here
 *
 * rounding to zero is a little complicated.
 * every int x, equals f(first 30 bit number) plus l(last 2 bit number)
 *
 *   f = x & ~0x3
 *   l = x & 0x3
 *   x = f + l
 *   threeforths(x) = f/4*3 + l*3/4
 *
 * f doesn't care about round at all, we just care about rounding from l*3/4
 *
 *   lm3 = (l << 1) + l
 *
 * when x > 0, rounding to zero is easy
 *
 *   lm3d4 = lm3 >> 2
 *
 * when x < 0, rounding to zero acts like divide_power2 in 2.78
 *
 *   bias = 0x3    // (1 << 2) - 1
 *   lm3d4 = (lm3 + bias) >> 2
 */
int threeforths(int x)
{
    
    int is_neg = x & INT_MIN;

    // 拆分 x (目的：比起/ 4 * 3,保留了更高的精度,因为低位可以先 * 3 再 /4,减少了除法带来的误差)
    int f = x & ~0x3; // 高位部分，清除最后2位(前30位，能被4整除)
    int l = x & 0x3;  // 低位部分，只保留最后2位(后2位,0,1,2,3)
    // 计算高位部分的 3/4
    int fd4 = f >> 2;
    int fd4m3 = (fd4 << 1) + fd4;
    // 计算低位部分的3/4
    int lm3 = (l << 1) + l;
    int bias = (1 << 2) - 1;
    (is_neg && (lm3 += bias));
    int lm3d4 = lm3 >> 2;

    return fd4m3 + lm3d4;
}

// 2.81
// A : ~((0x1 << k) - 1)
int A(int k)
{
    return ~((0x1 << k) - 1);
}
// B : ((0x1 << k) - 1) << j
int B(int k, int j)
{
    return ((0x1 << k) - 1) << j;
}
//-------offer solution
int offer_A(int k)
{
    return -1 << k;
}
int offer_B(int k, int j)
{
    return ~offer_A(k) << j;
}
//--------

// 2.82

/* Access bit-level representation floating-point number */
typedef unsigned float_bits;

/* If f is denorm, return 0, Otherwise,return f */
float_bits float_denorm_zero(float_bits f)
{
    /* Decompose bit repressentation into parts */
    unsigned sign = f >> 31;
    unsigned exp = f >> 23 & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    if (exp == 0)
    {
        /* Denormalized. Set fraction to 0 */
        frac = 0;
    }
    /* Reassemble bits */
    return (sign << 31) | (exp << 23) | frac;
}
// 2.92 *
/* Compute -f. If f is NaN, then return f. */
float_bits float_negate(float_bits f)
{
    /* Decompose bit representation into parts */
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    /* Check for NaN */
    if (exp == 0xFF && frac != 0)
    {
        /* Return argument if NaN */
        return f;
    }
    /* Negate sign bit */
    sign = !sign;
    /* Reassemble bits */
    return (sign << 31) | (exp << 23) | frac;
}

// 2.93
/* Compute |f|. If f is NaN, then return f. */
float_bits float_absval(float_bits f)
{
    /* Decompose bit representation into parts */
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    /* Check for NaN */
    if (exp == 0xFF && frac != 0)
    {
        /* Return argument if NaN */
        return f;
    }
    /* Clear sign bit to get absolute value */
    sign = 0;
    /* Reassemble bits */
    return (sign << 31) | (exp << 23) | frac;
}

// 2.94 *
/* Compute 2*f .If f is NaN, then return f. */
float_bits float_twice(float_bits f)
{
    /* Decompose bit representation into parts */
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    /* Check for NaN and 00 */
    if (exp == 0xFF)
    {
        /* Return argument if NaN */
        return f;
    }
    if (exp == 0)
    {
        /* Denormalized. Shift fraction */
        frac = frac << 1;
    }
    //-------------------------------------------fixed here---------------------------------------
    // 忘记考虑到规格化*2后变成00的情况
    else if (exp == 0xFE)
    {
        /* Normalized to NaN or Inf */
        exp = 0xFF;
        frac = 0;
    }
    //------------------------------------------------------------------------------------------
    else 
    {
        /* Normalized. Increment exponent */
        exp = exp + 1;
    }
    /* Reassemble bits */
    return (sign << 31) | (exp << 23) | frac;
}

// 2.95 *
/* Compute 0.5*f. If f is NaN, then return f. */
float_bits float_half(float_bits f)
{
    /* Decompose bit representation into parts */
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    unsigned rest = f & 0x7FFFFFFF;
    /*
     * round to even, we care about last 2 bits of frac
     *
     * 00 => 0 just >>1
     * 01 => 0 (round to even) just >>1
     * 10 => 1 just >>1
     * 11 => 1 + 1 (round to even) just >>1 and plus 1
     */
    int addition = (frac & 0x3) == 0x3;
    /* Check for NaN */
    if (exp == 0xFF && frac != 0)
    {
        /* Return argument if NaN */
        return f;
    }
    if (exp == 0)
    {
        /* Denormalized. Shift fraction */
        frac = frac >> 1;
        frac = frac + addition;
    }
    //-------------------------------------------fixed here---------------------------------------
    // 忘记考虑到规格化/2后变成非规格化的情况
    else if(exp == 1)
    {
        /* Normalized to denormalized */
        // 这就是为什么 IEEE 754 选择 E = 1 - bias 而不是 E = 0 - bias = -127 的原因——为了让规格化和非规格化之间的过渡无缝衔接。

        rest >>= 1;
        rest += addition;
        exp = rest >> 23 & 0xFF;
        frac = rest & 0x7FFFFF;
    }
    //------------------------------------------------------------------------------------------
    else 
    {
        /* Normalized. Decrement exponent */
        exp = exp - 1;
    }
    /* Reassemble bits */
    return (sign << 31) | (exp << 23) | frac;
}

// 2.96 *
/*
 * Compute (int) f.
 * If conversion cause overflow of f is NaN, return 0x80000000
*/
int float_f2i(float_bits f)
{
    /* Decompose bit representation into parts */
    unsigned sign = f >> 31;
    unsigned exp = (f >> 23) & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    int E = exp - 127; // exponent bias = 127
    unsigned M;
    int val;
    /* Check for NaN and infinity */
    if (exp == 0xFF)
    {
        return 0x80000000;
    }
    /* Denormalized */
    if (exp == 0)
    {
        M = frac;
        E = -126;
    }
    else 
    {
        M = frac | 0x800000; // add implicit leading 1
    }
    /* Handle cases */
    if (E < 0)
    {
        /* Absolute value less than 1 */
        val = 0;
    }
    else if (E > 31)
    {
        /* Overflow */
        val = 0x80000000;
    }
    else if (E > 23)
    {
        val = M << (E - 23);
    }
    else 
    {
        val = M >> (23 - E);
    }
    /* Apply sign */
    if (sign)
    {
        val = -val;
    }
    return val;
}

/*
 * Compute (float) f
 * If conversion cause overflow or f is NaN, return 0x80000000
 */
int offerfloat_f2i(float_bits f)
{
    unsigned sig = f >> 31;
    unsigned exp = f >> 23 & 0xFF;
    unsigned frac = f & 0x7FFFFF;
    unsigned bias = 0x7F;

    int num;
    unsigned E;
    unsigned M;

    /*
     * consider positive numbers
     *
     * 0 00000000 00000000000000000000000
     *   ===>
     * 0 01111111 00000000000000000000000
     *   0 <= f < 1
     * get integer 0
     *
     * 0 01111111 00000000000000000000000
     *   ===>
     * 0 (01111111+31) 00000000000000000000000
     *   1 <= f < 2^31
     * integer round to 0
     *
     * 0 (01111111+31) 00000000000000000000000
     *   ===>
     * greater
     *   2^31 <= f < oo
     * return 0x80000000
     */
    if (exp >= 0 && exp < 0 + bias)
    {
        /* number less than 1 */
        num = 0;
    }
    else if (exp >= 31 + bias)
    {
        /* number overflow */
        /* or f < 0 and (int)f == INT_MIN */
        num = 0x80000000;
    }
    else
    {
        E = exp - bias;
        M = frac | 0x800000;
        if (E > 23)
        {
            num = M << (E - 23);
        }
        else
        {
            /* whether sig is 1 or 0, round to zero */
            num = M >> (23 - E);
        }
    }

    return sig ? -num : num;
}

// 2.97  *
/* Compute (float) i */
/*
 * Assume i > 0
 * calculate i's bit length
 *
 * e.g.
 * 0x3 => 2
 * 0xFF => 8
 * 0x80 => 8
 */
int bits_length(int i)
{
    if ((i & INT_MIN) != 0)
    {
        return 32;
    }

    unsigned u = (unsigned)i;
    int length = 0;
    while (u >= (1 << length))
    {
        length++;
    }
    return length;
}

/*
 * generate mask
 * 00000...(32-l) 11111....(l)
 *
 * e.g.
 * 3  => 0x00000007
 * 16 => 0x0000FFFF
 */
unsigned bits_mask(int l)
{
    return (unsigned)-1 >> (32 - l);
}

/*
 * Compute (float) i
 */
float_bits float_i2f(int i)
{
    unsigned sig, exp, frac, rest, exp_sig /* except sig */, round_part;
    unsigned bits, fbits;
    unsigned bias = 0x7F;

    if (i == 0)
    {
        sig = 0;
        exp = 0;
        frac = 0;
        return sig << 31 | exp << 23 | frac;
    }
    if (i == INT_MIN)
    {
        sig = 1;
        exp = bias + 31;
        frac = 0;
        return sig << 31 | exp << 23 | frac;
    }

    sig = 0;
    /* 2's complatation */
    if (i < 0)
    {
        sig = 1;
        i = -i;
    }

    bits = bits_length(i);
    fbits = bits - 1;
    exp = bias + fbits;

    rest = i & bits_mask(fbits);
    if (fbits <= 23)
    {
        frac = rest << (23 - fbits);
        exp_sig = exp << 23 | frac;
    }
    else
    {
        int offset = fbits - 23;
        int round_mid = 1 << (offset - 1);

        round_part = rest & bits_mask(offset);
        frac = rest >> offset;
        exp_sig = exp << 23 | frac;

        /* round to even */
        if (round_part < round_mid)
        {
            /* nothing */
        }
        else if (round_part > round_mid)
        {
            exp_sig += 1;
        }
        else
        {
            /* round_part == round_mid */
            if ((frac & 0x1) == 1)
            {
                /* round to even */
                exp_sig += 1;
            }
        }
    }

    return sig << 31 | exp_sig;
}

// Compute bits_length
int my_bits_length(int i)
{
    if ((i & INT_MIN) != 0)
    {
        return 32;
    }
    unsigned u = (unsigned)i;
    int length = 0;
    while(u >= (1 << length))
    {
        length++;
    }
    return length;
}

unsigned my_bits_mask(int l)
{
    return (unsigned)-1 >> (32 - l);
}

/*
 * Compute (float) i
 */
float_bits my_float_i2f(int i)
{
    unsigned sign, exp, frac,exp_sig;
    unsigned bits, fbits;
    unsigned bias = 0x7F;
    if (i == 0)
    {
        sign = 0;
        exp = 0;
        frac = 0;
        return sign << 31 | exp << 23 | frac;
    }
    if (i == INT_MIN)
    {
        sign = 1;
        exp = bias + 31;
        frac = 0;
        return sign << 31 | exp << 23 | frac;
    }

    sign = 0;
    if (i < 0)
    {
        sign = 1;
        i = -i;
    }

    bits = my_bits_length(i);
    fbits = bits - 1;
    exp = bias + fbits;

    unsigned rest = i & my_bits_mask(fbits);
    if (fbits <= 23)
    {
        frac = rest << (23 - fbits);
        exp_sig = exp << 23 | frac;
    }
    else
    {
        int offset = fbits - 23;
        int round_mid = 1 << (offset - 1);
        unsigned round_part = rest & my_bits_mask(offset);
        frac = rest >> offset;
        exp_sig = exp << 23 | frac;
        if (round_part < round_mid)
        {
            ;
        }
        else if (round_part > round_mid)
        {
            exp_sig += 1;
        }
        else
        {
            if ((frac & 0x1) == 1)
            {
                exp_sig += 1;
            }
        }
    }
    return sign << 31 | exp_sig;
}

int main()
{
    printf("\r\nHello, World!\r\n");
    /*
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
    printf("int_shifts_are_arithmetic = %d\r\n", int_shifts_are_arithmetic());*/
    assert(leftmost_one(0xFF00) == 0x8000);
    assert(leftmost_one(0x6600) == 0x4000);
    assert(bad_int_size_is_32_B());
    assert(bad_int_size_is_32_C());
    assert(lower_one_mask(6) == 0x3F);
    assert(lower_one_mask(17) == 0x1FFFF);
    assert(rotate_left(0x12345678, 4) == 0x23456781);
    assert(rotate_left(0x12345678, 20) == 0x67812345);
    assert(rotate_left(0x11111111,0) == 0x11111111);
    assert(fits_bits(0xffffffff, 32) == 1);
    assert(fits_bits(-16, 4) == 0);
    assert(xbyte(0x12345678, 0) == 0x78);
    assert(xbyte(0x12345678, 1) == 0x56);
    assert(xbyte(0x12345678, 2) == 0x34);
    assert(xbyte(0x12345678, 3) == 0x12);
    assert(saturating_add(0x7FFFFFFF, 1) == 0x7FFFFFFF);
    assert(saturating_add(0x80000000, -1) == 0x80000000);
    assert(saturating_add(0x00000004, 0x00000005) == 0x00000009);
    assert(tsub_ok(0x80000000, 1) == 0);
    assert(tsub_ok(0x7FFFFFFF, -1) == 0);
    assert(tsub_ok(0x00000005, 0x00000003) == 1);
    assert(unsigned_high_prod(0x80000000, 0x00000004) == 0x00000002);
    assert(signed_high_prod(0x40000000, 0x00000004) == 0x00000001);
    assert(another_unsigned_high_prod(0x12345678, 0xFFFFFFFF) == unsigned_high_prod(0x12345678, 0xFFFFFFFF));
    int x = 2;
    assert((x * 17) == (x << 4) + x);
    assert((x * -7) == -((x << 3) - x));
    assert((x * 60) == (x << 6) - (x << 2));
    assert((x * -112) == -((x << 7) - (x << 4)));
    x = 0x80000007;
    assert(divide_power2(x, 1) == (x / 2));
    assert(divide_power2(x, 2) == (x / 4));
    x = 0x87654321;
    assert(my_mul3div4(x) == (x * 3) / 4);
    assert(mul3div4(x) == (x * 3) / 4);
    assert(threeforths(-8) == -6);
    assert(threeforths(-9) == -6);
    assert(threeforths(-10) == -7);
    assert(threeforths(-11) == -8);
    assert(threeforths(-12) == -9);
    assert(float_negate(0x7F800001) == 0x7F800001); // NaN
    assert(float_absval(0x7F800001) == 0x7F800001); // NaN
    assert(float_negate(0xBF800000) == (unsigned)0x3F800000); // -1.0 -> 1.0
    assert(float_absval(0xBF800000) == (unsigned)0x3F800000); // -1.0 -> 1.0
    assert(A(8) == 0xFFFFFF00);
    assert(B(16, 8) == 0x00FFFF00);
    printf("If you look at is,you have pass!\r\n");

    return 0;
}