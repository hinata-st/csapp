### bitXor(x,y)
```c
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  // p ^ q = (p & ~q) | (~p & q)
  // ~(p | q) = (~p) & (~q)
  // p ^ q = ~(~(p & ~q) & ~(~p & q))
  return ~(~(x & ~y) & ~(~x & y));
}
```
### tmin()
```c
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {

  return 0x1 << 31;

}
```
### isTmax(x)
思路就是(Tmax + 1)^(Tmax) = -1(用~0来表示)，同时-1也满足上诉式子，所以需要剔除-1的情况(-1 + 1) = 0,使用!!号把结果输出为1,0两种情况，此时&就起到了&&的作用
```c
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  return !((x + 1)^(x)^(~0)) & !!(x + 1);
}
```

### allOddBits(x)
生成0xAAAAAAAA去提取目标的奇位，然后和0xAAAAAAAA异或,相同则返回全0,!0 = 1,
```c
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int mask = 0xAA + (0xAA << 8);
  mask = mask + (mask << 16);
  return !((x & mask) ^ mask);
}
```
### negate(x)
负数的补码等于其正数的反码＋1
```c
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
```
### isAsciDigit(x)
减法等于加上负数的补码，0x39 - x >= 0
x - 0x30 >= 0,再提取出两个符号位判断是否都大于0.
```c
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int sig = 1 << 31;
  int lower_bound = x + (~0x30 + 1); // x - 0x30
  int upper_bound = 0x39 + (~x + 1); // 0x39 - x
  return !((lower_bound & sig) | (upper_bound & sig));
}
```
### conditional(x,y,z)
判断x是否等于0，并用0和1生成全1和全0的掩码，用掩码来进行选择。
```c
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int sig = !!x; // 0 or 1
  int mask = ~sig + 1; // all 0s or all 1s
  return (y & mask) | (z & ~mask);
}
```
### isLessOrEqual(x,y)
提取出x,y,y-x的符号  
- 当x,y同号时，不会产生溢出，结果为y-x的符号取反
- 当x,y不同号时，分为溢出和不溢出两种情况，列表观察可得，结果取决为x的符号。
```c
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int sig = ((y + (~x + 1)) >> 31) & 1; // sign of y - x,1 : x <= y
  int x_sign = (x >> 31) & 1;
  int y_sign = (y >> 31) & 1;
  int sign_diff = x_sign ^ y_sign;
  sig = (x_sign & sign_diff) | (!sig & !sign_diff);
  return sig;
}
```

### logicalNeg(x)
思路：0 和 0的负数(~x+1)的符号位都是0,以此来判断x是不是0,因为is_zero为0时要返回1，1要返回0(不能使用!)，使用(is_zero + 1) & 1来实现
```c
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  int neg_x = ~x + 1;
  int is_zero = (neg_x >> 31) | (x >> 31); // 0 if x is 0
  return (is_zero + 1) & 1;
}
```
### howManyBits(x)
思路：先把负数转成正数，这里取反后不加1，为了```转换前后需要的字节数一样(处理补码表示范围的不对称性)```,e.g.(-8) = 4,(7) = 4,(8) = 5,然后采用二分法逐步判断字节数。利用```(!!(neg_x >> 16))```来判断是否加以及是否位移。
```c
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int sign = x >> 31;
  int neg_x = x ^ sign; // if x < 0, ~x; else x
  int bits = 1; // start with sign bit
  // check each half
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

### floatScale2(uf)
思路：先把sign,exp,frac分别提取出来，注意一下特殊值的处理即可，这里非规格值*2后变成规格值不用特殊处理，因为frac左移一位后变成非规格值中隐含的1了。
```c
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = (uf >> 23) & 0xFF;
  unsigned frac = uf & 0x7FFFFF;
  /* Check for NaN and 00*/
  if (exp == 0xFF)
  {
    return uf;
  }
  if (exp == 0 && frac != 0)
  {
    frac = frac << 1;
  }
  else if(exp == 0 && frac == 0)
  {
    ;
  }
  else if(exp == 0xFE)
  {
    exp = 0xFF;
    frac = 0;
  }
  else
  {
    exp = exp + 1;
  }
  return (sign << 31) | (exp << 23) | frac;
}
```

### floatFloat2Int(uf)
思路：依旧是sign,exp,frac各部分分开，注意处理特殊情况，注意的地方是**M手动加上隐含的1，然后这里的M是原理中M(1.M,M是小数点右边的23位)左移23位后的结果，所以这里位移时得减去或加上这23位**。
```c
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  unsigned sign = uf >> 31;
  unsigned exp = (uf >>23) & 0xFF;
  unsigned frac = uf & 0x7FFFFF;
  int E = exp - 127;
  unsigned M;
  int val;
  /* Check for NaN and infinity*/
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
    // add implicit leading 1
    M = frac | 0x800000;
  }
  /* Handle cases*/
  if (E < 0)
  {
    /* Absolute value less than 1 */
    val = 0;
  }
  else if (E > 31)
  {
    /* Overflow*/
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
```

### floatPower2(x)
思路：先处理边界情况，too large,too small,denormalized,normalized  
通过原理表达式```(V = (-1)^s * 1.M * 2^(E) = 2^x)```来求解frac和exp的值   
注意在规格化下，由于2^x = 1.0*2^x，意味着尾数部分 = 1.000...000,隐含的1已经提供，所以frac = 0。
```c
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  unsigned frac = 0,exp = 0;
  // too large
  if (x >= 128)
  {
    exp = 0xFF;
    frac = 0;
  }
  // too small
  else if (x < -149)
  {
    exp = 0;
    frac = 0;
  }
  // Denormalized
  else if (x < -126 )
  {
    exp = 0;
    // v = f/2^n * 2^-126 =f * 2 ^ (-126 -n) = 2 ^ x -> f = 2 ^ (x + 126 + n)
    frac = 1 << (x + 126 + 23);
  }
  // Normalized
  else
  {
    frac = 0;
    exp = 127 + x;
  }
  return (exp << 23) | frac;
}
```

### finish datalab!!!
```
root@LAPTOP-0PTH5R3A:/home/emilia/emilia/csapplab/datalab/datalab-handout# ./btest
Score   Rating  Errors  Function
 1      1       0       bitXor
 1      1       0       tmin
 1      1       0       isTmax
 2      2       0       allOddBits
 2      2       0       negate
 3      3       0       isAsciiDigit
 3      3       0       conditional
 3      3       0       isLessOrEqual
 4      4       0       logicalNeg
 4      4       0       howManyBits
 4      4       0       floatScale2
 4      4       0       floatFloat2Int
 4      4       0       floatPower2
Total points: 36/36
```