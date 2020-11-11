
### scheduler.cc
- 选择并调度下一个要运行的线程
- 调度策略:FIFO
假定中断已被禁用。如果禁用了中断，则可以假定互斥（因为我们在单处理器上）。
不能使用锁来提供互斥:如果我们需要等待锁，而锁正忙，我们最终将调用FindNextToRun（），这将使我们陷入无限循环。

#### Scheduler::Scheduler()
初始化就绪队列

#### Scheduler::~Scheduler()
回收就绪队列线程

####  void Scheduler::ReadyToRun (Thread *thread)
将线程变成就绪态，但不运行，并追加到就绪队列

####  Thread* FindNextToRun();
返回下一个上CPU的线程，且该线程会被移出就绪列表。若不存在返回NULL。

#### void Run(Thread* nextThread);
将CPU分配给下一个线程，通过调用机器相关的上下文切换例程SWITCH，保存旧线程状态，加载新线程的状态。
假定先前运行的线程已经变为阻塞态或就绪态。
全局变量currentThread会指向nextThread。

#### void Print();
打印就绪队列的线程。


### switch.s
机器相关的上下文切换事项。
定义了两个结构：
- ThreadRoot(InitialPC, InitialArg, WhenDonePC, StartupPC)
  InitialPC ：运行程序的程序计数器
  InitialArg：线程参数
  WhenDonePC：线程返回时调用
  StartupPC：线程启动时调用
第一次启动一个现线程的时候被SWITCH调用

- SWITCH(oldThread, newThread)

  oldThread—当前正在运行的线程，其中要保存CPU寄存器状态。
  newThread—要运行的新线程，从中加载CPU寄存器状态。