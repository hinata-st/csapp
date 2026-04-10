# makefile

Makefile的规则

- 目标文件 : 依赖文件
- 要执行的命令

```makefile
hello:main.o message.o
	gcc -o hello main.o message.o

main.o: main.c
	gcc -c main.c

message.o: message.c 
	gcc -c message.c
```
```
> make 
> ./hello
```
## 伪目标

不生成文件的目标，通常用来执行一些命令，如clean
```makefile
hello:main.o message.o
	gcc -o hello main.o message.o

main.o: main.c
	gcc -c main.c

message.o: message.c 
	gcc -c message.c

clean:
	del -f *.o hello
```
```
> make clean
```
当该文件下有一个名为clean的文件时，make会认为clean是一个目标文件，而不是一个伪目标，这时就需要在clean前面加上.PHONY来告诉make clean是一个伪目标
```makefile
.PHONY: clean

hello:main.o message.o
	gcc -o hello main.o message.o

main.o: main.c
	gcc -c main.c

message.o: message.c 
	gcc -c message.c

clean:
	del -f *.o hello
```

all也是一个常见的伪目标，通常用来执行所有的编译命令
```makefile
.PHONY: clean all

all: hello world
	@echo "all done!"

hello world:main.o message.o
	gcc -o $@ main.o message.o

main.o: main.c
	gcc -c main.c

message.o: message.c 
	gcc -c message.c

clean:
	del -f *.o hello world
```

也可以单独执行make hello或make world来编译hello或world
```
> make all
> make hello
> make world
```

## 定义变量
```makefile
.PHONY: clean all
CFLAGS = -Wall -g -O2
targets = hello world
sources = main.c message.c
objects = main.o message.o

all: $(targets)
	@echo "all done!"

$(targets): $(objects)
	gcc $(CFLAGS) -o $@ $(objects)

main.o: main.c
	gcc $(CFLAGS) -c main.c

message.o: message.c 
	gcc $(CFLAGS) -c message.c

clean:
	del -f *.o hello world
```

### 自动变量
- $@ : 目标文件
- $< : 第一个依赖文件   
- $^ : 所有依赖文件

```makefile
.PHONY: clean all
CFLAGS = -Wall -g -O2
targets = hello world
sources = main.c message.c
objects = main.o message.o

all: $(targets)
	@echo "all done!"

$(targets): $(objects)
	gcc $(CFLAGS) -o $@ $(objects)

main.o: main.c
	gcc $(CFLAGS) -c $< -o $@

message.o: message.c 
	gcc $(CFLAGS) -c $< -o $@

clean:
	del -f *.o hello world
```

## 通配符

```makefile
.PHONY: clean all
CFLAGS = -Wall -g -O2
targets = hello world
sources = main.c message.c
objects = main.o message.o

all: $(targets)
	@echo "all done!"

$(targets): $(objects)
	gcc $(CFLAGS) -o $@ $(objects)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	del -f *.o hello world
```

## makefile的参数

- make -f <makefile> : 指定makefile文件
- make <target> : 指定目标文件
- make -n : 显示执行的命令，但不执行
- make -j <num> : 并行执行命令，num为并行
- make -k : 即使有错误也继续执行
- make -s : 不显示执行的命令
- make -B : 强制重新编译所有目标文件
- make -C <dir> : 在指定目录下执行make
- make -e : 环境变量覆盖makefile中的变量
- make -n : 显示执行的命令，但不执行
- make -p : 显示makefile中的所有变量和规则
- make -q : 检查目标文件是否需要更新，但不执行命令
- make -r : 不使用内置规则和变量

# cmake

```cmake
cmake_minimum_required(VERSION 3.10)

project(HelloWorld)

set(SOURCES main.c message.c)

add_executable(hello ${SOURCES})

```
```
> cmake -G "MinGW Makefiles" .
> mingw32-make
> mingw32-make clean
> ./hello
```

