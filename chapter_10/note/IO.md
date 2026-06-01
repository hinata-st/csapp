# 系统级I/O

### 目的：

- 了解Unix I/O将帮助你理解其他的系统概念
- 有时你除了使用Unix I/O以外别无选择

## Unix I/O

一个Linux文件就是一个m个字节的序列

Unix I/O使得所有的输入输出都能以一种同一且一致的方式来执行

- **打开文件**. 一个应用程序通过要求内核打开对应的文件，内核返回一个小的非负整数，叫做**描述符**，它在后续对此文件的所有操作中标识这个文件
- Linux shell创建的每个进程开始时都有三个打开的文件：标准输入(描述符为0)，标准输出(描述符为1)，标准错误(描述符为2)
- 改变当前的文件位置
- 读写文件
- 关闭文件

## 文件

- 普通文件包含任意数据.
- 目录是包含一组链接的文件，其中每个链接都将一个文件名映射到一个文件，这个文件可能是另一个目录.``.``是到该目录自身的链接，``..``是到该目录父目录的链接
- 套接字是用来与另一个进程进行跨网络通信的文件

目录层次结构中的位置用路径名来指定，有两种形式：
- 绝对路径名以一个斜杆开始``/home/droh/hello.c``
- 相对路径名以文件名开始，表示从当前工作目录开始的路径.``./hello.c``

## 打开和关闭文件

进程是通过调用open函数来打开一个已存在的文件或者创建一个新文件的：

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int open(const char *filename, int flags, mode_t mode);
```

open函数将filename转换为一个文件描述符，并且返回描述符数字。

flags参数指明了进程打算如何访问这个文件:
- O_RDONLY: 以只读方式打开文件
- O_WRONLY: 以只写方式打开文件
- O_RDWR: 以读写方式打开文件

flags参数还可以包含一些可选的标志:
- O_CREAT: 如果文件不存在则创建它
- O_TRUNC: 如果文件已经存在则将它截断为长度为0
- O_APPEND: 每次写入都追加到文件末尾

mode参数指定了新文件的访问权限位。

最后，进程通过调用close函数来关闭一个文件：

```c
#include <unistd.h>
int close(int fd);
```
关闭一个已关闭的描述符会出错

## 读和写文件

```c
#incldude <unistd.h>
// 返回：成功时返回读写的字节数，失败时返回-1，若读到文件末尾则返回0
ssize_t read(int fd, void *buf, size_t count);
// 返回：成功时返回读写的字节数，失败时返回-1
ssize_t write(int fd, const void *buf, size_t count);
```
read函数从描述符为fd的当前文件位置复制最多n个字节到内存位置buf   
write函数从内存位置buf复制至多n个字节到描述符fd的当前文件位置

在某些情况下，read和write传送的字节比应用程序要求的要少，这些不足值不表示有错误。出现这样情况的原因有：
- 读写时遇到EOF(文件末尾)
- 从终端读文本行
- 读和写网络套接字(socket)

## 用RIO包健壮地读写

RIO(Robust I/O)包，它会自动为你处理不足值。RIO提供了方便、健壮和高效地I/O。提供两类不同地函数：

- 无缓冲地输入输出函数
- 带缓冲地输入输出函数

### RIO的无缓冲的输入输出函数

```c
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
// 返回：若成功过则为传送的字节数，若EOF则为0(只对rio_readn)，若出错则为-1
```

rio_readn函数从描述符fd的当前文件位置最多传送n个字节到内存位置usrbuf。  
rio_writen函数从内存位置usrbuf传送n个字节到描述符fd的当前文件位置。

### RIO的带缓冲的输入输出函数

```c
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd; // 描述符
    int rio_cnt; // rio_buf中未读的字节数
    char *rio_bufptr; // rio_buf中下一个未读字节的地址
    char rio_buf[RIO_BUFSIZE]; // 内部缓冲区
} rio_t;
```

rio_readlineb函数从一个内部读缓冲区复制一个文本行，当缓冲区变空时，会自动地调用read重新填满缓冲区。

每打开一个描述符，都会调用一次rio_readinitb函数，它将描述符fd和地址rp处地一个类型为rio_t的读缓冲区联系起来。

rio_readlineb函数从文件rp读出下一个文本行，将他复制到内存位置usrbuf,并且用NULL字符来结束这个文本行。rio_readlineb函数最多复制maxlen-1个字节，并且在usrbuf的末尾添加一个NULL字符。

## 读取文件元数据

应用程序能够通过调用stat和fstat函数，检索到关于文件的信息

```c
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *filename, struct stat *buf);
int fstat(int fd, struct stat *buf);
```

stat函数以一个文件名字作为输入，并填写stat数据结构中的各个成员。fstat函数以一个文件描述符作为输入，并填写stat数据结构中的各个成员。

```c
struct stat {
    dev_t st_dev; // 文件所在设备的ID
    ino_t st_ino; // 文件的i节点号
    mode_t st_mode; // 文件的类型和访问权限
    nlink_t st_nlink; // 文件的链接数
    uid_t st_uid; // 文件所有者的用户ID
    gid_t st_gid; // 文件所有者的组ID
    dev_t st_rdev; // 设备文件的设备ID
    off_t st_size; // 普通文件的大小，以字节为单位
    blksize_t st_blksize; // 最佳I/O块大小
    blkcnt_t st_blocks; // 占用文件数据块的数量
    time_t st_atime; // 上次访问时间
    time_t st_mtime; // 上次修改时间
    time_t st_ctime; // 上次状态改变时间
};
```

Linux在sys/stat.h中定义了宏谓词来确定st_mode成员的文件类型

- S_ISREG(m): 判断是否为一个普通文件
- S_ISDIR(m): 判断是否为一个目录
- S_ISSOCK(m): 判断是否为一个套接字

```c
#include <sys/stat.h>

int main(int argc, char **argv){
    struct stat stat;
    char *type, *readok;

    Stat(argv[1], &stat);
    if (S_ISREG(stat.st_mode))
        type = "regular";
    else if (S_ISDIR(stat.st_mode))
        type = "directory";
    else
        type = "other";

    if ((stat.st_mode & S_IRUSR) && (stat.st_mode & S_IWUSR))
        readok = "yes";
    else
        readok = "no";

    printf("type: %s, read: %s\n", type, readok);
    exit(0);
}
```

## 读取目录内容

应用程序可以用readdir系列函数来读取目录的内容

```c
#include <dirent.h>
#include <sys/types.h>

// 返回：若成功，则为处理的指针；若出错，则为NULL
DIR *opendir(const char *name);
```

每次对readdir函数的调用都会返回目录中的下一个目录项，直到没有更多的目录项为止，此时readdir返回NULL

```c
struct dirent {
    ino_t d_ino; // 目录项的i节点号
    char d_name[MAXNAMLEN]; // 目录项的文件名
};
```

```c
#include <dirent.h>

int closedir(DIR *dirp);
```

函数closedir关闭一个由opendir打开的目录流，并且释放与之相关的资源。

用readdir来读取目录的内容

```c
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char **argv){
    DIR *streamp;
    struct dirent *dep;

    streamp = opendir(argv[1]);

    errno = 0;
    while ((dep = readdir(streamp)) != NULL)
        printf("%s\n", dep->d_name);
    if (errno != 0)

    closedir(streamp);
    exit(0);
}
```

```terminal
PS C:\Users\86199\learn-git\csapp\chapter_10\note> ./readdir C:\Users\86199\learn-git\csapp\chapter_10\note
.
..
IO.md
readdir.c
readdir.exe
test.c
test.exe
```

## 共享文件

内核用三个相关的数据结构来表示打开的文件：
- **描述符表**: 每个进程都有他独立的描述符表，它的表项是由进程打开的文件描述符来索引的。每个打开的描述符表项指向文件表中的一个表项。
- **文件表**: 打开文件的集合是由一张文件表来表示的，所有进程共享这张表。
- **v-node表**: 文件表中的每个表项都指向v-node表中的一个表项，v-node表包含了文件的元数据。所有进程共享这张表。

多个描述符也可以通过不同的文件表表项来引用同一个文件。每个描述符都有他们自己的文件位置，所以对不同描述符的读操作可以从文件的不同位置获取数据。
```c
int main(){
    char c;

    int fd1 = open("foobar.txt", O_RDONLY, 0);
    int fd2 = open("foobar.txt", O_RDONLY, 0);
    read(fd1, &c, 1); // 从文件的第一个字节读取
    read(fd2, &c, 1); // 从文件的第一个字节读取
    printf("fd1: %c, fd2: %c\n", c, c);
    exit(0);
}

// fd1: f, fd2: f
```

父子进程共享相同的打开文件表集合，**因此共享相同的文件位置**,在内核删除相应文件表表项之前,父子进程必须都关闭了他们的描述符。

```c
int main(){
    char c;

    int fd = open("foobar.txt", O_RDONLY, 0);
    if (fork() == 0){
        read(fd, &c, 1); // 从文件的第一个字节读取
        exit(0);
    }
    wait(NULL);
    read(fd, &c, 1); // 从文件的第二个字节读取
    printf("fd: %c\n", c);
    exit(0);
}

// fd: o
```

## I/O重定向

Linux shell提供了I/O重定向操作符，允许用户将磁盘文件和标准输入输出联系起来

```terminal
linux> ls > foo.txt
```

使得shell加载和执行ls程序，将标准输出重定向到磁盘文件foo.txt

I/O重定向是通过调用dup2函数来实现的：

```c
#include <unistd.h>
int dup2(int oldfd, int newfd);
```

dup2函数复制描述符表表项oldfd到描述符表项newfd，覆盖描述符表表项newfd以前的内容，如果newfd已经打开，dup2会复制oldfd之下关闭newfd

## 标准I/O

标准I/O库将一个打开的文件模型化为一个流，对于程序员而言，一个流就是一个指向FILE类型的结构的指针。每个ANSI C程序开始时都有3个打开的流stdin、stdout和stderr，分别对应与标准输入、标准输出和标准错误

```c
#include <stdio.h>
extern FILE *stdin, *stdout, *stderr;
```

类型为FILE的流是对文件描述符和流缓冲区的抽象。流缓冲区的目的和RIO读缓冲区的一样，就是**使开销较高的Linux I/O系统调用的数量尽可能小**,例如，假设我们有一个程序，它反复的调用标准I/O的getc函数，每次调用返回文件的下一个字符。当第一次调用getc时，库通过调用一次read函数来填充流缓冲区，然后将缓冲区中的第一个字节返回给应用程序。只要缓冲区中还有未读的字节，接下来对getc的调用就能直接从流缓冲区得到服务。

## 综合：我们该使用哪些I/O函数？

- 只要有可能就使用标准I/O，对磁盘和终端设备I/O来说，标准I/O函数是首选方法。 
- 不要使用scanf或rio_readlineb来读二进制文件。
- 对网络套接字的I/O使用RIO函数。套接字由文件描述符来引用，在这种情况下称为套接字描述符，应用程序进程通过读写套接字描述符来与运行在其他计算机的进程实现通信

标准I/O流有时候会相互冲突

- 限制一：跟在输出函数之后的输入函数
- 限制二：跟在输入函数之后的输出函数

建议在网络套接字上不要使用标准I/O函数来进行输入和输出，而要使用健壮的RIO函数。





