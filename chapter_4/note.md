# 处理器体系结构(by-bilibili)
## 4.1 指令系统结构

Y86-64 Instruction Set Architecture
- Programmer-Visible State
- Y86-64 Instructions
- Instruction Encoding
- Y86-64 Exceptions

### Programmer-Visible State
![alt text](image.png)
### Y86-64 Instructions
![alt text](image-1.png)
### Instruction Encoding
![alt text](image-2.png)
![alt text](image-3.png)
### Y86-64 Exceptions
![alt text](image-4.png)

## 4.2 数字电路与处理器设计

以寄存器文件为例
![alt text](image-5.png)
用Verilog描述寄存器文件
```verilog
module regfile(
    output reg[63:0]    data_out,
    input wire[63:0]    data_in,
    input wire[3:0]     addr,
    input wire          clock, we_, reset_
);
    reg[63:0] regfile[14:0];
    assign data_out = regfile[addr];
    ...
endmodule
```
寄存器文件的内部结构
![alt text](image-6.png)
双通道多路选择器的门级表示
![alt text](image-7.png)
D触发器
![alt text](image-8.png)
D触发器的Verilog描述
```verilog
module dflipflop(
    input D,
    input C,
    input G,
    input reg Q
);
    always @(posedge C) begin
        if (G) Q <= D;
    end
endmodule
```
Combinational Logic VS Sequential Logic
差异在于是否含有存储单元
- assign,用于描述组合逻辑
- always @(posedge clock),用于描述时序逻辑
- 模块调用

## 4.3 Y86-64的顺序实现

 **Organizing Processing into Stages**
- 取址阶段
- 译码阶段
- 执行阶段
- 访存阶段
- 写回阶段
- 更新PC
### Fetch Stage
取指阶段会根据指令代码来计算指令长度
### Decode Stage
![alt text](image-9.png)
### Execute Stage
![alt text](image-10.png)
CC:条件码寄存器
### Memory Stage
![alt text](image-11.png)
### Write Back Stage
![alt text](image-12.png)
### PC Update Stage
将PC设置成下一条指令的地址

### Example
![alt text](image-13.png)
![alt text](image-14.png)
![alt text](image-15.png)
![alt text](image-16.png)
![alt text](image-17.png)

## 4.4 Y86-64处理器硬件结构

### Fetch Stage
![alt text](image-18.png)
![alt text](image-19.png)
![alt text](image-20.png)

## Decode Stage
![alt text](image-21.png)
## Execute Stage
![alt text](image-22.png)
## Memory Stage
![alt text](image-23.png)
## Write Back Stage
![alt text](image-24.png)
## PC Update Stage

## 4.5 流水线的通用原理
非流水线处理器的指令执行过程
![alt text](image-25.png)
流水线处理器的指令执行过程
![alt text](image-26.png)
指令1进入B阶段后，指令2就可以进入A阶段了

增加流水线的阶段数，可以提升系统的吞吐量，但是过深的流水线同样也会导致系统性能的下降

指令互相之间会产生数据依赖和控制依赖。

## 4.6 流水线硬件结构

### Fetch Stage

## 4.7 数据冒险

## 4.8 控制冒险

## 4.9 Y86-64的流水线实现
## 4.10 流水线的控制逻辑