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
| `x/s 0x4025cf` | 把地址0x4025cf处的字符串显示出来 |
| `p/c 0x41`|把0x41解释为字符|
| `p/s 0x4025cf`|把地址0x4025cf处的字符串解释为字符串|
| `p/d 0xef`|把0xef解释为十进制数|
| `p/d $rdi`|查看 `rdi` 寄存器中的十进制值|
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

```assembly
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
先大概看一下总的汇编代码，然后逐一分析
```assembly
0000000000400f43 <phase_3>:
  400f43:	48 83 ec 18          	sub    $0x18,%rsp
  400f47:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  400f4c:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  400f51:	be cf 25 40 00       	mov    $0x4025cf,%esi
  400f56:	b8 00 00 00 00       	mov    $0x0,%eax
  400f5b:	e8 90 fc ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  400f60:	83 f8 01             	cmp    $0x1,%eax
  400f63:	7f 05                	jg     400f6a <phase_3+0x27>
  400f65:	e8 d0 04 00 00       	callq  40143a <explode_bomb>
  400f6a:	83 7c 24 08 07       	cmpl   $0x7,0x8(%rsp)
  400f6f:	77 3c                	ja     400fad <phase_3+0x6a>
  400f71:	8b 44 24 08          	mov    0x8(%rsp),%eax
  400f75:	ff 24 c5 70 24 40 00 	jmpq   *0x402470(,%rax,8)
  400f7c:	b8 cf 00 00 00       	mov    $0xcf,%eax
  400f81:	eb 3b                	jmp    400fbe <phase_3+0x7b>
  400f83:	b8 c3 02 00 00       	mov    $0x2c3,%eax
  400f88:	eb 34                	jmp    400fbe <phase_3+0x7b>
  400f8a:	b8 00 01 00 00       	mov    $0x100,%eax
  400f8f:	eb 2d                	jmp    400fbe <phase_3+0x7b>
  400f91:	b8 85 01 00 00       	mov    $0x185,%eax
  400f96:	eb 26                	jmp    400fbe <phase_3+0x7b>
  400f98:	b8 ce 00 00 00       	mov    $0xce,%eax
  400f9d:	eb 1f                	jmp    400fbe <phase_3+0x7b>
  400f9f:	b8 aa 02 00 00       	mov    $0x2aa,%eax
  400fa4:	eb 18                	jmp    400fbe <phase_3+0x7b>
  400fa6:	b8 47 01 00 00       	mov    $0x147,%eax
  400fab:	eb 11                	jmp    400fbe <phase_3+0x7b>
  400fad:	e8 88 04 00 00       	callq  40143a <explode_bomb>
  400fb2:	b8 00 00 00 00       	mov    $0x0,%eax
  400fb7:	eb 05                	jmp    400fbe <phase_3+0x7b>
  400fb9:	b8 37 01 00 00       	mov    $0x137,%eax
  400fbe:	3b 44 24 0c          	cmp    0xc(%rsp),%eax
  400fc2:	74 05                	je     400fc9 <phase_3+0x86>
  400fc4:	e8 71 04 00 00       	callq  40143a <explode_bomb>
  400fc9:	48 83 c4 18          	add    $0x18,%rsp
  400fcd:	c3                   	retq   
```
看到0x400f51和0x400f5b行，调用了sscanf函数，调试一下看看输入的格式是什么
```
(gdb) x/s $rsi
0x4025cf:       "%d %d"
(gdb) x/s $rdi
0x603820 <input_strings+160>:   "hello world"
(gdb) 
```
可以看到要求是输入两个整数   
尝试输入两个数字1 9
```
(gdb) x/16gx $rsp
0x7fffffffd6f0: 0x0000000000402210      0x0000000900000001
0x7fffffffd700: 0x0000000000000000      0x0000000000400e77
0x7fffffffd710: 0x0000000000402210      0x00007ffff7dee083
0x7fffffffd720: 0x0000000000000050      0x00007fffffffd808
0x7fffffffd730: 0x00000001f7fb27a0      0x0000000000400da0
0x7fffffffd740: 0x0000000000402210      0x6a63719956b9e925
0x7fffffffd750: 0x0000000000400c90      0x00007fffffffd800
0x7fffffffd760: 0x0000000000000000      0x0000000000000000
```
可以看到输入的数字被存储在了$rsp + 8,$rsp + 12的位置

看到``jmpq   *0x402470(,%rax,8)``
设计了一个switch跳转表，通过调试看到跳转表
```
(gdb) x/8gx 0x402470
0x402470:       0x0000000000400f7c      0x0000000000400fb9
0x402480:       0x0000000000400f83      0x0000000000400f8a
0x402490:       0x0000000000400f91      0x0000000000400f98
0x4024a0:       0x0000000000400f9f      0x0000000000400fa6
```
假设第一个数输入0，跳转到0x0000000000400f7c的位置，通过汇编代码可以看出，第二个输入的数字与分支赋值的值一样即可通过。及第二个输入的数字为207，尝试输入
```
(gdb) run
Starting program: /home/emilia/emilia/csapplab/bomblab/bomb/bomb 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
0 207
Halfway there!
```
可见phase_3已通过，应该有7组答案，``0 207只是其中一组``

## phase 4
我们先来看汇编代码
```assembly
000000000040100c <phase_4>:
  40100c:	48 83 ec 18          	sub    $0x18,%rsp
  401010:	48 8d 4c 24 0c       	lea    0xc(%rsp),%rcx
  401015:	48 8d 54 24 08       	lea    0x8(%rsp),%rdx
  40101a:	be cf 25 40 00       	mov    $0x4025cf,%esi
  40101f:	b8 00 00 00 00       	mov    $0x0,%eax
  401024:	e8 c7 fb ff ff       	callq  400bf0 <__isoc99_sscanf@plt>
  401029:	83 f8 02             	cmp    $0x2,%eax
  40102c:	75 07                	jne    401035 <phase_4+0x29>
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	callq  400fce <func4>
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c>
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
  401056:	74 05                	je     40105d <phase_4+0x51>
  401058:	e8 dd 03 00 00       	callq  40143a <explode_bomb>
  40105d:	48 83 c4 18          	add    $0x18,%rsp
  401061:	c3                   	retq
```
看到又有sscanf函数，调试一下看看输入的格式是什么
```
(gdb) x/s $rsi   
0x4025cf:       "%d %d"
```
可以看到要求输入两个整数，输入一个测试一下0,10,看看被放置在栈上的什么位置
```
(gdb) x/16gx $rsp
0x7fffffffd6f0: 0x0000000000402210      0x0000000a00000000
0x7fffffffd700: 0x0000000000000000      0x0000000000400e93
0x7fffffffd710: 0x0000000000402210      0x00007ffff7dee083
0x7fffffffd720: 0x0000000000000050      0x00007fffffffd808
0x7fffffffd730: 0x00000001f7fb27a0      0x0000000000400da0
0x7fffffffd740: 0x0000000000402210      0xed4eab77e63b349c
0x7fffffffd750: 0x0000000000400c90      0x00007fffffffd800
0x7fffffffd760: 0x0000000000000000      0x0000000000000000
```
可以看到输入的数字被存储在了$rsp + 8,$rsp + 12的位置  
继续分析汇编代码
```assembly
  40102e:	83 7c 24 08 0e       	cmpl   $0xe,0x8(%rsp)
  401033:	76 05                	jbe    40103a <phase_4+0x2e>
  401035:	e8 00 04 00 00       	callq  40143a <explode_bomb>
```
可以看到输入的第一个数字必须小于等于14，否则炸弹爆炸
```assembly
  40103a:	ba 0e 00 00 00       	mov    $0xe,%edx
  40103f:	be 00 00 00 00       	mov    $0x0,%esi
  401044:	8b 7c 24 08          	mov    0x8(%rsp),%edi
  401048:	e8 81 ff ff ff       	callq  400fce <func4>
```
可以看到输入的第一个数字被传递给了func4函数，第二个参数为0，第三个参数为14     
继续分析func4函数的汇编代码
```assembly
0000000000400fce <func4>:
  400fce:	48 83 ec 08          	sub    $0x8,%rsp
  400fd2:	89 d0                	mov    %edx,%eax
  400fd4:	29 f0                	sub    %esi,%eax
  400fd6:	89 c1                	mov    %eax,%ecx
  400fd8:	c1 e9 1f             	shr    $0x1f,%ecx
  400fdb:	01 c8                	add    %ecx,%eax
  400fdd:	d1 f8                	sar    %eax
  400fdf:	8d 0c 30             	lea    (%rax,%rsi,1),%ecx
  400fe2:	39 f9                	cmp    %edi,%ecx
  400fe4:	7e 0c                	jle    400ff2 <func4+0x24>
  400fe6:	8d 51 ff             	lea    -0x1(%rcx),%edx
  400fe9:	e8 e0 ff ff ff       	callq  400fce <func4>
  400fee:	01 c0                	add    %eax,%eax
  400ff0:	eb 15                	jmp    401007 <func4+0x39>
  400ff2:	b8 00 00 00 00       	mov    $0x0,%eax
  400ff7:	39 f9                	cmp    %edi,%ecx
  400ff9:	7d 0c                	jge    401007 <func4+0x39>
  400ffb:	8d 71 01             	lea    0x1(%rcx),%esi
  400ffe:	e8 cb ff ff ff       	callq  400fce <func4>
  401003:	8d 44 00 01          	lea    0x1(%rax,%rax,1),%eax
  401007:	48 83 c4 08          	add    $0x8,%rsp
  40100b:	c3                   	retq 
```
看到400fe2,推测此时的$eax的值为7,$ecx的值也为7,打个断点看看是否正确
```
(gdb) p/d $ecx
$10 = 7
(gdb) p/d $eax
$11 = 7
```
看到猜测正确，继续往下看，可以发现当输入的第一个数字等于7时，函数返回0,当输入的第一个数字大于7时，函数返回2*func4(输入的第一个数字-1)，当输入的第一个数字小于7时，函数返回2*func4(输入的第一个数字+1)+1,回看phase_4的汇编代码
```assembly
  40104d:	85 c0                	test   %eax,%eax
  40104f:	75 07                	jne    401058 <phase_4+0x4c>
  401051:	83 7c 24 0c 00       	cmpl   $0x0,0xc(%rsp)
  401056:	74 05                	je     40105d <phase_4+0x51>
```
输入的第二个数字必须为0，否则炸弹爆炸，所以答案为``7 0``
```
(gdb) run
Starting program: /home/emilia/emilia/csapplab/bomblab/bomb/bomb 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
0 207
Halfway there!
7 0
So you got that one.  Try this one.
```
可以看到phase_4成功解除了.

## phase 5








