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
    printf("\n");
}

int main()
{
    int a = INT_MIN;
    int b = -a;

    int c = 8;
    int d = -c;

    show_bytes((byte_pointer)&a, sizeof(int));
    show_bytes((byte_pointer)&b, sizeof(int));
    show_bytes((byte_pointer)&c, sizeof(int));
    show_bytes((byte_pointer)&d, sizeof(int));
    return 0;
}