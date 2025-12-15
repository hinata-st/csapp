### 1 
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
### 2
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
