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