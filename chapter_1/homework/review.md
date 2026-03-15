# CSAPP 学习文档 - chapter_1/homework

## 文档定位
- 目标：以学习为主，展示关键代码片段和实现思路。
- 参考源码：`chapter_1/homework/1_homework.c`、`chapter_1/homework/bits.c`
- 参考风格：`note.md` 的“我的解法/分析”和 `datalab.md` 的“函数 + 思路 + 代码”。

## 目录
- 1. 字节表示与端序
- 2. Data Lab 整数题关键片段
- 3. 位移边界与未定义行为
- 4. 溢出与健壮性问题
- 5. 浮点位级题关键片段
- 6. 学习总结

---

## 1. 字节表示与端序

### 1.1 is_little_endian

#### 关键代码
```c
int is_little_endian() 
{
    unsigned int x = 1;
    byte_pointer p = (byte_pointer)&x;
    return p[0] == 1;
}
```

#### 思路
- 把 `1` 写入内存后，检查最低地址字节。
- 若最低地址是 `0x01`，说明低位字节在低地址，即小端。

### 1.2 replace_byte

#### 当前实现
```c
unsigned int replace_byte(unsigned int x, int i, unsigned char b) 
{
    unsigned int num = 0xFFFFFFFF & ~(0xFF << (i * 8));
    return (x & num) | (b << (i * 8));
}
```

#### 推荐写法
```c
unsigned offer_replace_byte(unsigned x, int i, unsigned char b)
{
    unsigned mask = ((unsigned)0xFF) << (i << 3);
    unsigned pos_byte = ((unsigned)b) << (i << 3);
    return (x & ~mask) | pos_byte;
}
```

#### 思路
- 先“清位”：清除目标字节。
- 再“放位”：把新字节移动到目标位置后合并。
- `i << 3` 与 `i * 8` 等价，但更符合位级表达。

---

## 2. Data Lab 整数题关键片段

### 2.1 bitXor(x, y)

#### 关键代码
```c
int bitXor(int x, int y)
{
    return ~(~(x & ~y) & ~(~x & y));
}
```

#### 思路
- 目标是在只允许 `~` 和 `&` 的条件下实现异或。
- 先写出 `x ^ y = (x & ~y) | (~x & y)`，再用德摩根消去 `|`。

### 2.2 conditional(x, y, z)

#### 关键代码
```c
int conditional(int x, int y, int z)
{
    int sig = !!x;
    int mask = ~sig + 1;
    return (y & mask) | (z & ~mask);
}
```

#### 思路
- `mask` 在 `x != 0` 时为全 1，在 `x == 0` 时为全 0。
- 用掩码选择，替代条件分支。

### 2.3 isLessOrEqual(x, y)

#### 关键代码
```c
int isLessOrEqual(int x, int y)
{
    int sig = ((y + (~x + 1)) >> 31) & 1;
    int x_sign = (x >> 31) & 1;
    int y_sign = (y >> 31) & 1;
    int sign_diff = x_sign ^ y_sign;
    sig = (x_sign & sign_diff) | (!sig & !sign_diff);
    return sig;
}
```

#### 思路
- 先分“异号/同号”两种情况。
- 异号时结果由 `x` 的符号直接决定；同号时看 `y - x` 的符号。

### 2.4 howManyBits(x)

#### 关键代码
```c
int howManyBits(int x)
{
    int sign = x >> 31;
    int neg_x = x ^ sign;
    int bits = 1;
    bits += ((!!(neg_x >> 16)) << 4);
    neg_x = neg_x >> ((!!(neg_x >> 16)) << 4);
    bits += ((!!(neg_x >> 8)) << 3);
    neg_x = neg_x >> ((!!(neg_x >> 8)) << 3);
    bits += ((!!(neg_x >> 4)) << 2);
    neg_x = neg_x >> ((!!(neg_x >> 4)) << 2);
    bits += ((!!(neg_x >> 2)) << 1);
    neg_x = neg_x >> ((!!(neg_x >> 2)) << 1);
    bits += (!!(neg_x >> 1));
    neg_x = neg_x >> (!!(neg_x >> 1));
    bits += neg_x;
    return bits;
}
```

#### 思路
- 负数先转成等价的“非负形态”（`x ^ sign`）统一处理。
- 通过二分探测最高有效位，计算最小位数。

---

## 3. 位移边界与未定义行为

### 3.1 lower_one_mask(n)

#### 当前实现
```c
int lower_one_mask(int n)
{
    return (1 << n) - 1;
}
```

#### 推荐实现
```c
int offer_lower_one_mask(int n)
{
    int w = sizeof(int) << 3;
    return (unsigned)-1 >> (w - n);
}
```

#### 关键点
- 当 `n == 32` 时，`1 << 32` 未定义。
- 用全 1 右移构造掩码，边界更安全。

### 3.2 rotate_left(x, n)

#### 当前实现
```c
unsigned rotate_left(unsigned x, int n)
{
    return (x << n) | (x >> (32 - n));
}
```

#### 推荐实现
```c
unsigned offer_rotate_left(unsigned x, int n)
{
    int w = sizeof(unsigned) << 3;
    return x << n | x >> (w - n - 1) >> 1;
}
```

#### 关键点
- 当 `n == 0` 时，`x >> 32` 未定义。
- 拆成两次右移可避免“位移量等于字长”。

### 3.3 fits_bits(x, n)

#### 当前实现
```c
int fits_bits(int x, int n)
{
    return !((unsigned int)x >> (n - 1) >> 1);
}
```

#### 推荐实现
```c
int offer_fits_bits(int x, int n)
{
    int w = sizeof(int) << 3;
    int offset = w - n;
    return (x << offset >> offset) == x;
}
```

#### 关键点
- 常规模板是“截断再还原看是否变化”。
- 对正负数都统一成立。

---

## 4. 溢出与健壮性问题

### 4.1 copy_int

#### 有风险写法
```c
void copy_int(int val, void *buf, int maxbytes)
{
    if (maxbytes - sizeof(val) >= 0)
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}
```

#### 修正写法
```c
void offer_copy_int(int val, void *buf, int maxbytes)
{
    if (maxbytes >= (int)sizeof(val))
    {
        memcpy(buf, (void *)&val, sizeof(val));
    }
}
```

#### 思路
- 减法比较更容易引入类型与边界问题。
- 直接比较“容量是否足够”更稳。

### 4.2 tsub_ok(x, y)

#### 当前代码片段
```c
int tsub_ok(int x, int y)
{
    int y_neg = -y;
    int sum = x + y_neg;
    ...
}
```

#### 关键点
- `y == INT_MIN` 时，`-y` 会溢出。
- 做减法溢出题时，需要先处理极值输入。

### 4.3 saturating_add(x, y)

#### 关键代码片段
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

#### 思路
- 同号相加后符号翻转，说明发生溢出。
- 根据方向饱和到 `INT_MAX` 或 `INT_MIN`。

---

## 5. 浮点位级题关键片段

### 5.1 floatScale2(uf)

#### 关键代码
```c
unsigned floatScale2(unsigned uf)
{
    unsigned sign = uf >> 31;
    unsigned exp = (uf >> 23) & 0xFF;
    unsigned frac = uf & 0x7FFFFF;
    if (exp == 0xFF) return uf;
    if (exp == 0 && frac != 0) frac = frac << 1;
    else if (exp == 0xFE) { exp = 0xFF; frac = 0; }
    else if (exp != 0) exp = exp + 1;
    return (sign << 31) | (exp << 23) | frac;
}
```

#### 思路
- 先拆字段，再分类处理：NaN/Inf、非规格化、规格化。
- 对规格化数，乘 2 的本质是指数加 1。

### 5.2 float_half(f)

#### 关键代码片段
```c
else if(exp == 1)
{
    rest >>= 1;
    rest += addition;
    exp = rest >> 23 & 0xFF;
    frac = rest & 0x7FFFFF;
}
```

#### 思路
- `exp == 1` 是规格化向非规格化过渡的关键边界。
- 这一段是处理“过渡 + 舍入”的核心。

### 5.3 float_i2f(i)

#### 关键代码片段
```c
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
    if (round_part > round_mid || (round_part == round_mid && (frac & 0x1)))
    {
        exp_sig += 1;
    }
}
```

#### 思路
- 先求最高有效位，得到指数部分。
- 尾数超过 23 位时做 round-to-even，这是正确性的核心。

---

## 6. 学习总结（代码导向）
- 这组练习的价值在于“可复用模板”，而不是背答案。
- 当前最关键的可迁移能力：
  - 掩码构造与条件选择
  - 符号位驱动的比较与溢出判断
  - 浮点 `sign/exp/frac` 拆解与边界处理
- 后续学习建议：每个函数保留“当前版 + 修正版 + 为什么修”，持续积累自己的位级模式库。
