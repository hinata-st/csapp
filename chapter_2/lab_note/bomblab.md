### 实验2 炸弹实验

首先打开bomb.c文件,可以看到6个phase,对应6个解除炸弹的地方，我们先看第一个phase

## phase 1
```c
/* Hmm...  Six phases must be more secure than one phase! */
    input = read_line();             /* Get input                   */
    phase_1(input);                  /* Run the phase               */
    phase_defused();                 /* Drat!  They figured it out!
				      * Let me know how they did it. */
    printf("Phase 1 defused. How about the next one?\n");
```
由于lab为了保密，只提供了bomb.0文件，我们只能反汇编bomb.0文件

输入```> objdump -d ./bomb >> bomb.s```来得到汇编代码

启动gdb调试器运行
```> gdb ./bomb```

启动程序，输入一个字符串，先乱输入一个，果然炸弹爆炸了，我们打开`bomb.s`文件看看这个函数的汇编代码发生了什么？
```
(gdb) run
Starting program: /home/emilia/emilia/csapplab/bomblab/bomb/bomb 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
hellow world

BOOM!!!
The bomb has blown up.
```
找到phase_1函数的汇编代码
```assembly
0000000000400ee0 <phase_1>:
  400ee0:	48 83 ec 08          	sub    $0x8,%rsp
  400ee4:	be 00 24 40 00       	mov    $0x402400,%esi
  400ee9:	e8 4a 04 00 00       	callq  401338 <strings_not_equal>
  400eee:	85 c0                	test   %eax,%eax
  400ef0:	74 05                	je     400ef7 <phase_1+0x17>
  400ef2:	e8 43 05 00 00       	callq  40143a <explode_bomb>
  400ef7:	48 83 c4 08          	add    $0x8,%rsp
  400efb:	c3                   	retq
```

由汇编代码可以发现，我们输入的字符串应该放到%rdi寄存器中,然后通过一个`mov`指令一个字符串地址移动到%esi寄存器，然后调用`strings_not_equal`函数对这两个字符串进行比较,如果返回值为0,则跳转至`phase_1+0x17`地址,调用`explode_bomb`函数,否则继续执行,会调用explode_bomb,炸弹爆炸。

我们进行调试看看猜测是否正确

```
break phase_1
break strings_not_equal
run
continue
输入字符串(先乱输入一个hello world)
x/s $rdi
x/s $rsi
```
结果如下
```
(gdb) break phase_1
Breakpoint 1 at 0x400ee0
(gdb) break strings_not_equal
Breakpoint 2 at 0x401338
(gdb) run
Starting program: /home/emilia/emilia/csapplab/bomblab/bomb/bomb 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
hello world

Breakpoint 1, 0x0000000000400ee0 in phase_1 ()
(gdb) continue
Continuing.

Breakpoint 2, 0x0000000000401338 in strings_not_equal ()
(gdb) x/s $rdi
0x603780 <input_strings>:       "hello world"
(gdb) x/s $rsi
0x402400:       "Border relations with Canada have never been better."
(gdb) 
```
可以发现我们输入的字符串果然放到了%rdi寄存器中，而不出所料需要输入的正确字符串为"Border relations with Canada have never been better."

我们重启程序，输入正确的字符串
```
(gdb) run
Starting program: /home/emilia/emilia/csapplab/bomblab/bomb/bomb 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
hello world

BOOM!!!
The bomb has blown up.
[Inferior 1 (process 14382) exited with code 010]
```
可以发现第一个phase解除了。
虽然解决了phase_1，但是我们来看看phase_1的strings_length的汇编实现,怎么实现计算字符串长度的计算。

```
000000000040131b <string_length>:
  40131b:	80 3f 00             	cmpb   $0x0,(%rdi)
  40131e:	74 12                	je     401332 <string_length+0x17>
  401320:	48 89 fa             	mov    %rdi,%rdx
  401323:	48 83 c2 01          	add    $0x1,%rdx
  401327:	89 d0                	mov    %edx,%eax
  401329:	29 f8                	sub    %edi,%eax
  40132b:	80 3a 00             	cmpb   $0x0,(%rdx)
  40132e:	75 f3                	jne    401323 <string_length+0x8>
  401330:	f3 c3                	repz retq 
  401332:	b8 00 00 00 00       	mov    $0x0,%eax
  401337:	c3                   	retq 
```
等价c代码
- 若s[0] == 0,返回0
- 否则从s开始递增指针p,直到*p==0
- 返回p - s


## phase 2
```c