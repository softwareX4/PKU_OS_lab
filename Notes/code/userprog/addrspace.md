## addrspace.cc
### AddrSpace类
TranslationEntry *pageTable  假设是线性的页表
unsigned int numPages 虚拟地址空间需要的页数

####  AddrSpace::AddrSpace(OpenFile *executable)
从executable中加载程序并设置，以便运行用户指令，假设目标代码是NOFF格式的。
把程序的内存变换为物理内存，现在是简单的1：1转换（因为是单编程且只有一个未分段的页表）
executable是包含code.o的要加载进内存的文件

NoffHeader ：noff.h 定义了 .o的格式，其中noffHeader的格式为：
- 幻数
- 代码段（segment类型）
- 初始化了的数据段
- 未初始化的数据段
其中segment又包含：
- 段的虚拟地址
- 段在此文件中的位置
- 段大小

int ReadAt(char *into, int numBytes, int position)
从position开始读入

转换地址格式为NOP

分配地址空间：
 size = noffH.code.size + noffH.initData.size + noffH.uninitData.size  + UserStackSize;

 计算需要的页数：
 numPages = size /PageSize + ((size % PageSize > 0) ? 1 : 0)

 计算空间：
  size = numPages * PageSize;

  创建页表：
  pageTable = new TranslationEntry[numPages];

  初始设置页表
  现在  虚拟页号=物理页号

将整个地址空间清零，将未初始化的数据段和堆栈段清零
  把代码和数据读进主存对应位置