# malloclab

阅读malloclab的README.md,了解我们的任务目标。

我们需要实现4个函数

```c
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
```
并用提供的trace文件测试我们的实现。
```
./mdriver -t ./traces -V
```

在csapp中，已经提供了一个简单的内存分配器实现，是基于隐式空闲链表的。我们先复刻一下再进行优化.

复刻后的结果如下：

```
Results for mm malloc:
trace  valid  util     ops      secs  Kops
 0       yes   99%    5694  0.003695  1541
 1       yes   99%    5848  0.003600  1624
 2       yes   99%    6648  0.005165  1287
 3       yes  100%    5380  0.003484  1544
 4       yes   66%   14400  0.000059244898
 5       yes   91%    4800  0.004554  1054
 6       yes   92%    4800  0.004326  1110
 7       yes   55%   12000  0.095660   125
 8       yes   51%   24000  0.120014   200
 9       yes   27%   14401  0.027774   519
10       yes   34%   14401  0.001318 10924
Total          74%  112372  0.269649   417

Perf index = 44 (util) + 28 (thru) = 72/100
```

lab用两个性能指标来评估我们的解决方案：

- 空间利用率(utilization)：驱动程序使用的内存总量(即通过malloc或realloc分配但未通过free释放)与分配程序使用的堆的大小之间的峰值比率，最佳比率为1.
- 吞吐量(throughput)：平均每秒完成的操作数.

P = wU + (1-w)min(1,T/Tlibc),其中w是权重，U是空间利用率，T是我们的实现的吞吐量，Tlibc是libc的吞吐量。w = 0.6.所以1性能指标更倾向于空间利用率而不是吞吐量。

我们尝试一下怎么改进.