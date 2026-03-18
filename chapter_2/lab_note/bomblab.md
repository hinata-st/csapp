### 实验2 炸弹实验

首先打开bomb.c文件,可以看到6个phase,对应6个解除炸弹的地方，我们先看第一个phase

由于需要用到gdb调试器，下面给出常用的gdb指令
| 指令 | 说明 |
| --- | --- |
| `run` | 运行程序,到第一个断点 |
| `continue` | 继续运行，直到下一个断点 |
| `break <function_name>` | 在函数入口处设置断点 |
| `break *0x400ee9` | 在指定地址设置断点 |
| `info breakpoints` | 查看所有断点 |
| `delete <breakpoint_number>` | 删除指定断点 |
| `x/s $rdi` | 查看 `rdi` 寄存器中的字符串 |
| `si` | 单步执行，进入函数 |
| `ni` | 单步执行，不进入函数 |
| `info frame` | 查看当前栈帧信息 |
| `print $rdi` | 查看 `rdi` 寄存器中的值 |
| `info registers` | 查看所有寄存器信息 |
| `x/16gx $rsp` | 查看 `rsp` 寄存器中的值（16: 看16个单元，g: 每个单元是8字节，x: 十六进制） |

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
/* The second phase is harder.  No one will ever figure out
  * how to defuse this... */
input = read_line();
phase_2(input);
phase_defused();
printf("That's number 2.  Keep going!\n");
```
我们同样打开bomb.s文件找到phase_2函数的汇编代码
```assembly
0000000000400efc <phase_2>:
  400efc:	55                   	push   %rbp
  400efd:	53                   	push   %rbx
  400efe:	48 83 ec 28          	sub    $0x28,%rsp
  400f02:	48 89 e6             	mov    %rsp,%rsi
  400f05:	e8 52 05 00 00       	callq  40145c <read_six_numbers>
  400f0a:	83 3c 24 01          	cmpl   $0x1,(%rsp)
  400f0e:	74 20                	je     400f30 <phase_2+0x34>
  400f10:	e8 25 05 00 00       	callq  40143a <explode_bomb>
  400f15:	eb 19                	jmp    400f30 <phase_2+0x34>
  400f17:	8b 43 fc             	mov    -0x4(%rbx),%eax
  400f1a:	01 c0                	add    %eax,%eax
  400f1c:	39 03                	cmp    %eax,(%rbx)
  400f1e:	74 05                	je     400f25 <phase_2+0x29>
  400f20:	e8 15 05 00 00       	callq  40143a <explode_bomb>
  400f25:	48 83 c3 04          	add    $0x4,%rbx
  400f29:	48 39 eb             	cmp    %rbp,%rbx
  400f2c:	75 e9                	jne    400f17 <phase_2+0x1b>
  400f2e:	eb 0c                	jmp    400f3c <phase_2+0x40>
  400f30:	48 8d 5c 24 04       	lea    0x4(%rsp),%rbx
  400f35:	48 8d 6c 24 18       	lea    0x18(%rsp),%rbp
  400f3a:	eb db                	jmp    400f17 <phase_2+0x1b>
  400f3c:	48 83 c4 28          	add    $0x28,%rsp
  400f40:	5b                   	pop    %rbx
  400f41:	5d                   	pop    %rbp
  400f42:	c3                   	retq  
```
通过gdb调试发现,我们输入的6个整数分别存储在%rsp+0x4,%rsp+0x8,%rsp+0xc,%rsp+0x10,%rsp+0x14,%rsp+0x18
```
(gdb) x/16gx $rsp
0x7fffffffd6a0: 0x0000000200000001      0x0000000400000003
0x7fffffffd6b0: 0x0000000600000005      0x0000000000401431
0x7fffffffd6c0: 0x0000000000402210      0x0000000000402210
0x7fffffffd6d0: 0x0000000000000000      0x0000000000400e5b
0x7fffffffd6e0: 0x0000000000402210      0x00007ffff7dee083
0x7fffffffd6f0: 0x0000000000000050      0x00007fffffffd7d8
0x7fffffffd700: 0x00000001f7fb27a0      0x0000000000400da0
0x7fffffffd710: 0x0000000000402210      0x0bbbffe1e0e91c59
```


通过阅读汇编代码，可以发现等价与下面c代码
```c
int number[6];//输入的6个整数
int cnt = 0;
if(number[cnt] != 1)
    explode_bomb();
for(int i = 0; i < 6; i++){
    cnt++;
    if(number[cnt] != 2 * number[i])
        explode_bomb();
}
```
可以发现输入的第一个数字必须是1，而且后面的每个数字必须是前面一个数字的两倍，可以得到phase_2要求输入的6个数字为``1 2 4 8 16 32``
```
(gdb) run
Starting program: /home/emilia/emilia/csapplab/bomblab/bomb/bomb 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
```
可以发现phase_2成功解除了。

## phase 3

