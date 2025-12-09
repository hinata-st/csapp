# CSAPP 第二章笔记：

## 知识笔记

![知识笔记](/chapter_1/homework/第二章_01.png)

## 作业笔记

## 目录
- 2.58 is_little_endian
- 2.60 replace_byte
- 2.61 位级判断
- 2.62 int_shifts_are_arithmetic
- 2.63 srl 和 sra
- 2.66 leftmost_one
- 2.68 lower_one_mask
- 2.69 rotate_left
- 2.70 fits_bits
- 2.72 copy_int
- 2.73 saturating_add
- 2.74 tsub_ok
- 2.76 calloc
- 2.80 threeforths
- 2.81 位模式生成
- 2.95 float_half
- 2.97 float_i2f

---

## 2.58 is_little_endian

### 我的解法
```c
int is_little_endian() 
{
    unsigned int x = 1;
    byte_pointer p = (byte_pointer)&x;
    return p[0] == 1;
}
```

### 官方解法
```c
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
```

### 对比分析

| 方面 | 我的解法 | 官方解法 |
|------|----------|----------|
| 测试值 | `1` | `0xff` |
| 返回方式 | 直接返回表达式 | if-else 结构 |
| 简洁性 | ✓ 更简洁 | 稍冗长 |

### ✅ 重点
- 两种方法思路相同：检查最低地址存储的是高位还是低位
- 小端序：低位在低地址；大端序：高位在低地址
- 我的写法更简洁，`return p[0] == 1` 直接返回布尔结果

---

## 2.60 replace_byte

题目 ： 假设我们将一个w位的字中的字节从0(最低有效字节)到w/8-1(最高位)编号。他会返回一个无符号值，其中参数x的字节i被替换成字节b:   
`unsigned replace_byte(unsigned x, int i,unsigned char b);`   
`例子：replace_byte(0x12345678,0,0xAB) --> 0x123456AB`

### 我的解法
```c
unsigned int replace_byte(unsigned int x, int i, unsigned char b) 
{
    unsigned int num = 0xFFFFFFFF & ~(0xFF << (i * 8));
    return (x & num) | (b << (i * 8));
}
```

### 官方解法
```c
unsigned offer_replace_byte(unsigned x, int i, unsigned char b)
{
    if (i < 0) { printf("error: i is negetive\n"); return x; }
    if (i > sizeof(unsigned) - 1) { printf("error: too big i"); return x; }

    unsigned mask = ((unsigned)0xFF) << (i << 3);
    unsigned pos_byte = ((unsigned)b) << (i << 3);

    return (x & ~mask) | pos_byte;
}
```

### 对比分析

| 方面 | 我的解法 | 官方解法 |
|------|----------|----------|
| 边界检查 | ❌ 无 | ✓ 有 |
| 移位表示 | `i * 8` | `i << 3` |
| 类型转换 | 隐式 | 显式 `(unsigned)` |

### ⚠️ 我的不足
1. **缺少边界检查**：没有验证 `i` 的有效范围
2. **冗余代码**：`0xFFFFFFFF & ~(...)` 中 `0xFFFFFFFF &` 是多余的

### ✅ 官方优点
1. **健壮性**：检查 `i < 0` 和 `i > sizeof(unsigned) - 1`
2. **位运算替代乘法**：`i << 3` 比 `i * 8` 更高效
3. **显式类型转换**：避免隐式转换的潜在问题

---

## 2.61 位级判断

### 我的解法 (C: 最低字节全1)
```c
int xlsbis1(int x)
{
    return !((x & 0xff)^0xff);
}
```

### 官方解法
```c
int offerC(int x)
{
    return offerA(x | ~0xff);  // offerA 返回 !~x
}
```

### 对比分析

| 方面 | 我的解法 | 官方解法 |
|------|----------|----------|
| 方法 | 提取后异或比较 | 高位填1后检查全1 |
| 可读性 | 一般 | 更巧妙 |

### ✅ 官方思路
- `x | ~0xff`：把高 24 位都变成 1
- 如果低 8 位也全是 1，则整个数全是 1
- 复用 `offerA`（判断全1）的逻辑

---

## 2.62 int_shifts_are_arithmetic

### 我的解法
```c
int int_shifts_are_arithmetic()
{
    int x = -1;
    return (x >> 1) == -1;
}
```

### 官方解法
```c
int offer_int_shifts_are_arithemetic()
{
    int num = -1;
    return !(num ^ (num >> 1));
}
```

### 对比分析

| 方面 | 我的解法 | 官方解法 |
|------|----------|----------|
| 使用 `==` | ✓ 是 | ✗ 否 |
| 纯位运算 | ✗ 否 | ✓ 是 |
| 可读性 | ✓ 更直观 | 需要理解异或 |

### ✅ 重点
- `-1` 的补码是全 1（`0xFFFFFFFF`）
- 算术右移：高位补符号位（1），结果仍是 `-1`
- 逻辑右移：高位补 0，结果是 `0x7FFFFFFF`
- 官方用 `!(a ^ b)` 代替 `a == b`，符合某些题目禁用比较运算符的要求

---

## 2.63 srl 和 sra

题目：函数srl用算术右移来完成逻辑右移，函数sra用逻辑右移

### 我的解法 (sra 有问题)
```c
unsigned sra(unsigned x, int k)
{
    int xsrl = (unsigned)x >> k;
    int w = sizeof(int) << 3;
    unsigned mask = (1u << (w - k)) - 1;
    return xsrl | ~mask;  // ❌ 无论正负都填1
}
```

### 官方解法
```c
int offer_sra(int x, int k)
{
    int xsrl = (unsigned)x >> k;
    int w = sizeof(int) << 3;
    int mask = (int)-1 << (w - k);
    int m = 1 << (w - 1);
    mask &= !(x & m) - 1;  // 关键：根据符号位决定是否应用 mask
    return xsrl | mask;
}
```

### ⚠️ 我的严重错误
- **没有判断符号位**：我的实现无条件在高位填 1
- 正数的算术右移应该填 0，负数才填 1

### ✅ 官方关键技巧
```c
mask &= !(x & m) - 1;
```
| x 的符号位 | `!(x & m)` | `!(x & m) - 1` | 效果 |
|-----------|-----------|----------------|------|
| 0（正数） | 1 | 0 | `mask = 0`，高位不填 |
| 1（负数） | 0 | -1 = 全1 | `mask` 不变，高位填1 |

---

## 2.66 leftmost_one

### 我的解法
```c
int leftmost_one(unsigned x)
{
    if (x == 0) return 0;  // ❌ 使用了 if
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x & ~(x >> 1);
}
```

### 官方解法
```c
int offer_leftmost_one(unsigned x)
{
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return (x >> 1) + (x && 1);  // 巧妙处理 x=0
}
```

### 对比分析

| 方面 | 我的解法 | 官方解法 |
|------|----------|----------|
| 使用 if | ✓ 是 | ✗ 否 |
| 处理 x=0 | 单独判断 | `(x && 1)` 技巧 |

### ✅ 官方巧妙之处
- `x && 1`：当 `x = 0` 时返回 0，否则返回 1
- `(x >> 1) + (x && 1)`：
  - `x = 0` 时：`0 + 0 = 0` ✓
  - `x ≠ 0` 时：相当于 `(x >> 1) + 1 = x & ~(x >> 1)` ✓

---

## 2.68 lower_one_mask

### 我的解法
```c
int lower_one_mask(int n)
{
    return (1 << n) - 1;  // ❌ n=32 时未定义行为
}
```

### 官方解法
```c
int offer_lower_one_mask(int n)
{
    int w = sizeof(int) << 3;
    return (unsigned)-1 >> (w - n);
}
```

### ⚠️ 我的严重问题
- **`1 << 32` 是未定义行为**：移位量 ≥ 类型位宽时结果未定义

### ✅ 官方解法优点
- 右移不会有边界问题：当 `n = 32` 时，`(unsigned)-1 >> 0 = 0xFFFFFFFF`
- 通过右移 `-1`（全1）来生成掩码，更安全

### 📝 记忆要点
> **生成低 n 位为 1 的掩码，优先使用右移方式：`(unsigned)-1 >> (w - n)`**

---

## 2.69 rotate_left

### 我的解法
```c
unsigned rotate_left(unsigned x, int n)
{
    return (x << n) | (x >> (32 - n));  // ❌ n=0 时未定义
}
```

### 官方解法
```c
unsigned offer_rotate_left(unsigned x, int n)
{
    int w = sizeof(unsigned) << 3;
    return x << n | x >> (w - n - 1) >> 1;
}
```

### ⚠️ 我的问题
- **n = 0 时**：`x >> 32` 是未定义行为

### ✅ 官方技巧
```c
x >> (w - n - 1) >> 1
```
- 将一次移位拆成两次，避免移位量等于位宽
- 当 `n = 0` 时：`x >> 31 >> 1 = x >> 32`（分两步完成）

---

## 2.70 fits_bits
```c
/*
 * Return 1 when x can be represented as an n-bit, 2's-complement
 * number; 0 otherwise
 * Assume 1 <= n <= w
 */
int fits_bits(int x, int n);
```
### 我的解法
```c
int fits_bits(int x, int n)
{
    return !((unsigned int)x >> (n - 1) >> 1);
}
```

### 官方解法
```c
int offer_fits_bits(int x, int n)
{
    int w = sizeof(int) << 3;
    int offset = w - n;
    return (x << offset >> offset) == x;
}
```

### 对比分析

| 方面 | 我的解法 | 官方解法 |
|------|----------|----------|
| 方法 | 检查高位是否全0 | 符号扩展后比较 |
| 正确性 | ⚠️ 只适用于正数 | ✓ 正负数都正确 |

### ⚠️ 我的问题
- **没有正确处理负数**：负数的高位应该全是 1，不是 0

### ✅ 官方思路
- 左移 `w-n` 位，再算术右移 `w-n` 位
- 这会做符号扩展：如果 n 位能表示 x，则恢复后等于原值

---

## 2.72 copy_int

### 原始错误代码
```c
void copy_int(int val, void *buf, int maxbytes)
{
    if (maxbytes-sizeof(val) >= 0)  // ❌ 永远为真
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}
```

### 官方修复
```c
void offer_copy_int(int val, void *buf, int maxbytes)
{
    if (maxbytes >= (int)sizeof(val))
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}
```

### ✅ 重点：有符号与无符号比较的陷阱

| 表达式 | 问题 |
|--------|------|
| `maxbytes - sizeof(val) >= 0` | `sizeof` 返回 `size_t`（无符号），减法结果也是无符号，**永远 ≥ 0** |
| `maxbytes >= (int)sizeof(val)` | 两边都是有符号数，比较正确 |

### 📝 记忆要点
> **与 `sizeof` 结果比较时，要么强转为 `int`，要么确保另一边也是无符号**

---

## 2.73 saturating_add

### 我的解法
```c
int saturating_add(int x, int y)
{
    // ... 使用 && 和 || 的短路特性
    int temp = (overflow_1 && (sum = tmax) || ...);
    return sum;
}
```

### 官方解法
```c
int saturating_add(int x, int y)
{
    int sum = x + y;
    int sig_mask = INT_MIN;
    int pos_over = !(x & sig_mask) && !(y & sig_mask) && (sum & sig_mask);
    int neg_over = (x & sig_mask) && (y & sig_mask) && !(sum & sig_mask);

    (pos_over && (sum = INT_MAX) || neg_over && (sum = INT_MIN));

    return sum;
}
```

### ✅ 溢出检测逻辑

| 溢出类型 | 条件 |
|----------|------|
| **正溢出** | x > 0 且 y > 0 且 sum < 0 |
| **负溢出** | x < 0 且 y < 0 且 sum ≥ 0 |

### ⚠️ 短路赋值技巧
```c
(condition && (var = value));
```
等价于 `if (condition) var = value;`

但这种写法属于**灰色地带**，虽然绕过了 `if` 关键字，本质仍是条件执行。

---

## 2.74 tsub_ok

### 我的解法
```c
int tsub_ok(int x, int y)
{
    int y_neg = -y;  // ❌ y = INT_MIN 时溢出
    int sum = x + y_neg;
    // ...
}
```

### 官方解法
```c
int offer_tsub_ok(int x, int y)
{
    int res = 1;
    (y == INT_MIN) && (res = 0);  // 特殊处理 INT_MIN

    int sub = x - y;
    int pos_over = x > 0 && y < 0 && sub < 0;
    int neg_over = x < 0 && y > 0 && sub > 0;

    res = res && !(pos_over || neg_over);
    return res;
}
```

### ⚠️ 我的严重问题
- **`-INT_MIN` 溢出**：`-(-2147483648) = 2147483648`，超出 `int` 范围
- 结果仍是 `INT_MIN`，导致判断错误

### ✅ 官方处理
- **单独处理 `y == INT_MIN`**：直接返回 0（不安全）
- 减法溢出判断不依赖取反操作

---

## 2.76 calloc

### 我的解法
```c
void *calloc(size_t nmemb, size_t size)
{
    size_t total_size = nmemb * size;  // ❌ 没有检查溢出
    void *ptr = malloc(total_size);
    if (ptr) memset(ptr, 0, total_size);
    return ptr;
}
```

### 官方解法
```c
void *another_calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0) return NULL;
    
    size_t buf_size = nmemb * size;
    if (nmemb == buf_size / size)  // ✓ 检查溢出
    {
        void *ptr = malloc(buf_size);
        if (ptr != NULL) memset(ptr, 0, buf_size);
        return ptr;
    }
    return NULL;
}
```

### ✅ 溢出检查技巧
```c
if (nmemb == buf_size / size)
```
- 如果 `nmemb * size` 没有溢出，则 `(nmemb * size) / size == nmemb`
- 如果溢出了，等式不成立

---

## 2.80 threeforths

### 核心思路
```c
// 计算 x * 3/4，不溢出，向零舍入
int threeforths(int x)
{
    int f = x & ~0x3;  // 高位（能被4整除）
    int l = x & 0x3;   // 低位（0, 1, 2, 3）
    
    // threeforths(x) = f/4*3 + l*3/4
    int fd4m3 = (f >> 2) * 3;      // 先除后乘，不丢精度
    int lm3d4 = (l * 3 + bias) / 4; // 先乘后除，需要舍入
    
    return fd4m3 + lm3d4;
}
```

### ✅ 为什么要拆分？

| 直接 `(x/4)*3` | 拆分法 |
|----------------|--------|
| `7/4*3 = 1*3 = 3` ❌ | `(4/4)*3 + (3*3)/4 = 3+2 = 5` ✓ |
| 丢失低位精度 | 低位单独处理，保留精度 |

### ✅ 与 mul3div4 (2.79) 的区别

| 题目 | 要求 | 方法 |
|------|------|------|
| 2.79 `mul3div4` | 可以溢出 | `(x*3)/4` |
| 2.80 `threeforths` | **不能溢出** | 拆分 `(f/4)*3 + (l*3)/4` |

---

## 2.81 位模式生成

### 我的解法
```c
int A(int k) { return ~((0x1 << k) - 1); }
int B(int k, int j) { return ((0x1 << k) - 1) << j; }
```

### 官方解法
```c
int offer_A(int k) { return -1 << k; }
int offer_B(int k, int j) { return ~offer_A(k) << j; }
```

### ✅ 官方更简洁
- `-1` 的二进制是全 1
- `-1 << k` 直接生成高位全1、低k位全0的模式
- 官方解法复用了 `offer_A`

---

## 2.95 float_half

### ✅ 关键：规格化到非规格化的转换

```c
else if(exp == 1)
{
    // exp = 1 除以 2 后变成 exp = 0（非规格化）
    rest >>= 1;        // 整体右移，exp 的最低位滑入 frac 最高位
    rest += addition;  // 舍入处理
    exp = rest >> 23 & 0xFF;
    frac = rest & 0x7FFFFF;
}
```

### ✅ 为什么 `rest >>= 1` 能正确工作？

| 操作前 (exp=1) | 操作后 (exp=0) |
|----------------|----------------|
| `1.frac × 2^(-126)` | `0.1frac × 2^(-126)` |
| 隐含的 1 | 显式地放入 frac 最高位 |

### 📝 IEEE 754 设计要点
- **非规格化的 E = 1 - bias = -126**（与 exp=1 的规格化数相同）
- 这是为了实现**平滑过渡**（Gradual Underflow）

---

## 2.97 float_i2f

### 核心步骤
```c
float_bits my_float_i2f(int i)
{
    // 1. 处理特殊情况（0 和 INT_MIN）
    // 2. 处理符号
    // 3. 计算位长度：bits = my_bits_length(i)
    // 4. 计算指数：exp = bias + (bits - 1)
    // 5. 提取尾数
    // 6. 舍入到偶数（当尾数超过 23 位时）
    // 7. 组装结果
}
```

### ✅ 关键公式
```
bits  = 整数的二进制位数
fbits = bits - 1 = 真实指数 E = 小数部分位数
exp   = bias + fbits = 127 + E
```

### ✅ 舍入到偶数
```c
if (round_part == round_mid)  // 恰好在中间
{
    if ((frac & 0x1) == 1)    // 当前尾数是奇数
    {
        exp_sig += 1;          // 进位使其变偶数
    }
}
```

---

## 总结：常见问题与技巧

### 输出数据的字节表示

```cpp
typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len)
{
    // sizeof的返回值为size_t类型,为unsigned int64
    size_t i;
    for (i = 0; i < len; i++)
    {
        printf(" %.2x", start[i]);
    }
    printf("\r\n");
}
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
```
### 生成掩码
```c
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
```
### 计算`(int) f,(float)i`

```c
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
```

### ⚠️ 我的常见错误

| 错误类型 | 示例 |
|----------|------|
| **未处理边界** | `1 << 32`、`x >> 32` 未定义行为 |
| **忽略特殊值** | `INT_MIN`、`-INT_MIN` 溢出 |
| **有符号/无符号混用** | `sizeof` 返回无符号，比较时需注意 |
| **未考虑负数** | 算术右移要根据符号位决定填充 |

### ✅ 官方常用技巧

| 技巧 | 示例 |
|------|------|
| **用 `i << 3` 替代 `i * 8`** | 更高效 |
| **用 `!(a ^ b)` 替代 `a == b`** | 避免比较运算符 |
| **用 `(unsigned)-1` 生成全1** | 可移植性好 |
| **分两次移位** | `x >> 31 >> 1` 避免移 32 位 |
| **用 `condition && (var = value)` 替代 if** | 绕过 if 限制 |
| **检查乘法溢出** | `a == (a*b)/b` |

### 📝 核心记忆点

1. **移位量必须在 `[0, w-1]` 范围内**
2. **`-INT_MIN = INT_MIN`**（溢出回到自身）
3. **`sizeof` 返回 `size_t`（无符号）**
4. **IEEE 754 非规格化 E = 1 - bias = -126**（与最小规格化相同）
5. **舍入到偶数**：中间值时看最低位，奇数进位