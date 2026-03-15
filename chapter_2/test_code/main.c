#include <stdio.h>

void multstore(long x, long y, long *);
long mult2(long a, long b);

int main()
{
    long d;
    //multstore(2, 3, &d);  
    d = mult2(2, 3);
    printf("2 * 3 --> %ld\n", d);
    return 0;
}

long mult2(long a, long b)
{
    return a * b;
}
