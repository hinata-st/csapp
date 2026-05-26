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

代码见``./malloclab/textbook.c``

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

为了改进，我们选择**分离空闲链表**，在空闲块的分配策略上，我们选择**首次分配**

``make && ./mdriver -f traces/amptjp-bal.rep -v`` 

编写宏定义

```c
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4             /* Word and header/footer size (bytes) */
#define DSIZE 8             /* Double word size (bytes) */
#define CHUNKSIZE (1 << 12) /* Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))
#define PACK_ALL(size, alloc, prev_alloc) ((size) | (alloc) | (prev_alloc << 1))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) ((GET(p) & 0x2) >> 1)
#define SET_SIZE(p) (GET(p) & 0x7)
#define SET_ALLOC(p) (GET(p) & 0x1)
/* Given block ptr bp, compute address of header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// 构造分离空闲链表
// 8-24 24-32 32-64 64-80 80-120 120-240 240-480 480-960 960-1920 1920-3840 3840-7680 7680-15360 15360-30720 30720-61440 61440-0x7fffffff
#define FREE_LIST_NUM 15

static char **free_lists; // 分离空闲链表数组

// 计算偏移地址
#define OFFSET_ADDR(ptr) ((char *)(ptr) - (char *)mem_start_brk)
// 计算真实地址
#define REAL_ADDR(offset) ((char *)(mem_start_brk) + (offset))
// 得到前一个节点的地址，ptr是当前节点的指针，返回值是前一个节点的地址(都是偏移地址)
#define PREV_FREE(ptr) ((char *)(mem_start_brk) + GET(ptr))
// 得到后一个节点的地址，ptr是当前节点的指针，返回值是后一个节点的地址(都是偏移地址)
#define NEXT_FREE(ptr) ((char *)(mem_start_brk) + GET((char *)(ptr) + WSIZE))

// 设置前一个节点的地址，ptr是当前节点的指针(偏移量)，prev是要设置的前一个节点的地址(偏移量)
#define SET_PREV_FREE(ptr, prev) (PUT(ptr, prev))
// 设置后一个节点的地址，ptr是当前节点的指针(偏移量)，next是要设置的后一个节点的地址(偏移量)
#define SET_NEXT_FREE(ptr, next) (PUT((char *)(ptr) + WSIZE, next))


static char *heap_listp; // Pointer to first block
```

编写mm_init函数

```c

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // 初始化空闲链表
    free_lists = (char **)mem_sbrk(FREE_LIST_NUM * sizeof(char *));
    if (free_lists == (char **)-1) {
        return -1;
    }
    for (int i = 0; i < FREE_LIST_NUM; i++) {
        free_lists[i] = NULL;
    }
    
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    {
        return -1;
    }
    PUT(heap_listp, 0);                            /* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2 * WSIZE);

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    {
        return -1;
    }
    return 0;
}
```