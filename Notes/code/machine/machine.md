## machine\machine.h machine.cc
模拟在Nachos之上的用户程序的运行。
用户程序会被加载进主存。Nachos内核也在。
用户程序一次被模拟器运行一条指令。
每个内存引用都会被转换和检查。

- 页大小pagesize :128
- 物理页面NumPhysPages：32
- TLB大小TLBSize：44

定义了一些异常类型

定义了用户程序的CPU状态，包括用户栈指针、程序返回地址、当前、下一个和前一个的程序计数等等，是MIPS指令集的扩展，由于需要在两个指令之间启动或者终止用户程序，因此我们需要跟踪。

定义了Instruction类()和Machine类

### Instruction类
定义了一条指令，以两种未解码的**二进制形式**表示
经过decode 以识别要对寄存器执行的操作，来对任何立即数操作
- value ： 指令的二进制表示
- opcode ： 指令的类型（与指令的opcode 字段不一样）
- rs rt rd 三个指令寄存器
- extra ： 立即或目标或残影字段或偏移量。
           立即数被符号扩展。

### Machine类
此类定义了模拟的主机工作站硬件，如用户程序所见：CPU寄存器，主存等。

用户程序不应知道它们正在我们的模拟器还是真实硬件上运行，除非我们不支持浮点指令，Nachos的系统调用接口与UNIX不同（Nachos中有10个系统调用，而UNIX中有200个！）

如果要实现更多的UNIX系统调用，则应该能够在Nachos之上运行Nachos

此类的函数在machine.cc，mipssim.cc和translate.cc中定义。

#### 可直接调用的：
- Run()
在machine\mipssim.cc中定义
运行一个用户程序
- int ReadRegister(int num)
读取CPU寄存器的内容
-  void WriteRegister(int num, int value)
往CPU寄存器中存储一个值


#### 不可以直接调用：
- **void OneInstruction(Instruction \*instr)** 	
【在machine\mipssim.cc中定义】
运行用户程序的一个指令
如果存在任何类型的异常或中断，将调用异常处理程序，并在异常处理程序返回时返回Run（），它将在循环中重新调用我们。
可重入的

- void DelayedLoad(int nextReg, int nextVal);  	
【在machine\mipssim.cc中定义】
				// Do a pending delayed load (modifying a reg)
    
- **bool ReadMem(int addr, int size, int\* value)**
【在machine\translate.cc中定义】
把在虚拟内存中addr处的size（1，2，4）字节读入value指向的地方，
如果虚拟内存向物理内存转换失败返回False

- **bool WriteMem(int addr, int size, int value)**
【在machine\translate.cc中定义】
往虚拟内存中读写1、2 或 4 字节，如果不能被正确的转换返回false    			
    
- **ExceptionType Translate(int virtAddr, int\* physAddr, int size,bool writing)**
【在machine\translate.cc中定义】
使用页表或TLB将虚拟地址转换为物理地址。 检查是否对齐以及各种其他错误，如果一切正常，请在转换表条目中设置use / dirty位，并将转换后的物理地址存储在“ physAddr”中。 如果有错误，则返回异常的类型。



- **void RaiseException(ExceptionType which, int badVAddr)**
控制权从用户态转换到内核态
由于系统调用或者其他异常自陷到Nachos内核（如地址转换失败）
处理完会返回用户态 

- void Debugger()	
用户程序debugger

- void DumpState()	
打印用户CPU和内存状态	

### 成员变量
- char *mainMemory
存储用户程序运行时代码和数据的物理内存

- int registers[NumTotalRegs]
NumTotalRegs = 40
运行用户程序的CPU寄存器

用户程序中虚拟地址到物理地址（相对于“ mainMemory”的开头）的硬件转换可以由以下之一控制：
- 传统的线性页表
- 一个软件加载转换的后备缓冲区（tlb）-虚拟页号到物理页号的映射的缓存
 如果“ tlb”为NULL，则使用线性页表
 如果“ tlb”为非NULL，则Nachos内核负责管理TLB的内容。 但是内核可以使用它想要的任何数据结构（例如分段分页）来处理TLB缓存未命中。

为简单起见，页表指针和TLB指针都是公共的。 但是，虽然可以有多个页表（每个地址空间一个，存储在内存中），但是只有一个TLB（在硬件中实现）。
因此，尽管内核软件可以自由修改TLB的内容，但应该将TLB指针视为只读。


###  外部函数
void ExceptionHandler(ExceptionType which)
【在userprog/exception.cc中定义】

处理用户的系统调用或异常

### 工具函数

用于将字和短字与模拟机器的little endian格式相互转换。
 如果主机是 little endian （DEC和Intel），则它们最终将成为NOP。
每种格式存储的内容：
    主机字节顺序：
        内核数据结构
        用户寄存器 
    模拟机器字节顺序：
        主存的内容


- unsigned int WordToHost(unsigned int word);
- unsigned short ShortToHost(unsigned short shortword);
-unsigned int WordToMachine(unsigned int word);
- unsigned short ShortToMachine(unsigned short shortword);


## machine\translate.cc
用于管理虚拟页号->物理页号中的转换的数据结构，在用户程序的角度管理物理内存。

此文件中的数据结构是“双重使用”的-它们既用作页表条目，又用作TLB中的条目。
无论哪种方式，每个条目都采用形式：<虚拟页号，物理页号>。

### TranslationEntry类
TranslationEntry类在页表或TLB中中定义了一个条目。 每个条目定义从一个虚拟页面到一个物理页面的映射。
还有一些用于访问控制的附加位（有效和只读），还有一些用于使用信息的位（use和dirty）。
- valid位被设置：忽略转换，即实体还没被初始化
- readOnly位被设置：用户程序不可更改页内内容
- use :被硬件设置，在每次页被引用或者修改后
- dirty : 被硬件设置，在每次页被修改后

##  userprog\exception.cc 
导致控制权从用户态转换回内核态：
- 系统调用syscall
目前只有Halt()
- 异常exceptions
用户代码的要求CPU不能处理

中断（也能导致控制权更改）在其他地方处理

目前这里只处理Halt()，其他所有核心转储

#### void ExceptionHandler(ExceptionType which)
进入Nachos内核的入口点。 当用户程序正在执行并且执行系统调用或生成寻址或算术异常时调用。
对于系统调用，以下是调用约定：

系统调用代码-r2
 arg1-r4
 arg2-r5
 arg3-r6
  arg4-r7

系统调用的结果（如果有的话）必须放回到r2中。
并且不要忘记在返回前增加pc的数量。 （否则将永远循环进行相同的系统调用

