// 3.17 写一个函数good_echo,他从标准输入读取一行，再把它写到标准输出。你的实现应该对任意长度的输入行都能正常工作。

#include <stdio.h>
#include <assert.h>
#define BUF_SIZE 12

void good_echo(void){
    char buf[BUF_SIZE];
    while(1)
    {
        char *p = fgets(buf, BUF_SIZE, stdin);
        if(p == NULL)
            break;
        printf("%s", p);
    }
    return;
}

int main(int argc, char const *argv[])
{
    good_echo();
    return 0;
}