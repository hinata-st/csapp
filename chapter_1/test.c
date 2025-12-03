#include <stdio.h>
#include <limits.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
    {
        printf(" %.2x", start[i]);
    }
    printf("\r\n");
}

int main()
{
    int aa = 0x12345678;
    int a = 1;
    int b = 2;
    printf("\r\n hello,world!\r\n");
    show_bytes((byte_pointer)&aa, sizeof(int));
    b = (1 & (a = 2)) | (0 & (a = 3));
    printf("a = %d, b = %d\r\n", a, b);
    return 0;
}