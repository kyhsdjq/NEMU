# NEMU

According to: https://github.com/NJU-ProjectN/nemu

Instruction: [NEMU/doc/guide](https://github.com/kyhsdjq/NEMU/blob/main/doc/guide/index.md)

## PA 1-1 数据在计算机内的存储

## PA 1-2 整数的表示、存储和运算

## PA 1-3 浮点数的表示和运算

## PA 2-1 指令解码与执行

## PA 2-2 装载ELF文件

困难：

- elf.c的实现没有很大问题，但从pa-2-1遗留下jmp_rm_v未实现。实现过程中发现其instr_execute_1op函数定义与jmp_rm_b不同，并且涉及到len置0，最终使用宏定义解决。

心得：

- 作为os，kernal一方面与硬件层的端口直接进行交互，另一方面为软件提供更高层的抽象，可见构造一个完整的机器需要硬件和软件的同时支撑。

## PA 2-3 可选任务：完善调试器

NEMU的断点在平坦地址下存在问题：
    在程序的一开始插入地址大于0x100000的断点，断点会无效。只有eip>0x100000时插入，该断点才生效。

原因：
    断点作用机制是将对应地址的指令替换为0xf1，即opcode表中的break_point。但是，添加kernel之后，装载elf表时会覆盖之前添加的断点，导致之前设定断点替换的0xf1指令被覆盖

可行的解决方案：
    当程序运行到eip== 0x100000时，重新在所有断点地址处的指令替换为0xf1，同时将断点结构中保存的原指令更新。

修改位置：

```
void exec(uint32_t) in pa_nju/nemu/src/cpu/cpu.c

void fetch_breakpoints in pa_nju/nemu/src/monitor/breakpoint.c
```

## PA 3-1 Cache的模拟

在NEMU中采用了cache后，其命中率可以达到99%左右，当cache读写速度是memory读写速度十倍的时候，效率提高9.2倍左右，性能提升显著。

## PA 3-2 保护模式（分段机制）

心得：

- 同样是将部分数据从读取速度慢的容器移动到读取速度快的容器中，segment register采用了和cache不同的实现方式，原因在于segment register只关注若干特殊指令和固定的segments，而cache需要关注所有数据的读写。

## PA 3-3 分页机制 & 虚拟地址转换

心得：

- kernel在初始化时，首先通过固定的偏移量为自己建立页表（同时建了两个页表以确保无论cr0是否开启，kernel都能访问到正确内存），此后，将第二个页表（映射0xc0000000-0xc7ffffff的）继承给testcase，让它切换到kernel时可以直接使用这一映射。

## PA 4-0 SSH和X11转发配置说明

## PA 4-1 异常和中断的响应

心得：

- 操作系统的异常处理函数由内核提供，因此同一套硬件可以对于同一个异常进行不同的处理，但是控制流的切换取决于硬件实现，因此一定相同。

## PA 4-2 外设与I/O

心得：

- 在实现上，异常和中止在raise_intr()之后完全相同。不同之处在于，自陷可以通过指令直接进入，而中止需要cpu定期检测针脚的信号才能转入。此外，所有中止信号都通过pic芯片处理后再传输给cpu，从而实现中止的优先级处理。

- DMA控制法是另一种I/O控制方式。直接控制法中，cpu通过频繁的中止与I/O交互，导致耗时较长。因此，把cpu对缓冲区的处理功能独立出来，通过独立的DMA芯片直接进行内存和外设之间的数据传输，从而加快输入输出。
