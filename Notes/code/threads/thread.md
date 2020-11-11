### main.cc
引导代码，初始化操作系统内核。
为简化调试和测试，允许直接调用内部操作系统功能，
只会初始化数据结构，并启动用户程序以打印登录提示。
在以后的分配之前，不需要此文件的大多数内容。


检查命令行参数，初始化数据结构，调用测试程序（可选）

#### int main(int argc, char **argv) 
根据命令行参数初始化系统，包括线程、用户程序、文件系统、网络、中断处理、定时器、调度机制等

### threadtest.cc
进程任务的测试程序
创建两个线程，通过调用Thread::Yield让前后台切换上下文，说明线程系统的内部工作。

#### void SimpleThread(int which)
选取一个线程，循环5次，每次迭代将CPU让给另一个就绪的线程

#### void ThreadTest1()
创建一个新线程，Fork函数SimpleThread

#### void ThreadTest()
默认用ThreadTest1()测试程序

### thread.h
管理线程需要的数据结构。
程序计数器、处理器寄存器、运行栈
可能会出现栈溢出，表现形式之一是页错误。
fork一个线程要先new再fork。
- 18个CPU寄存器（上下文切换用）
- 线程私有运行栈：4*1024
- 线程状态： JUST_CREATED, RUNNING, READY, BLOCKED ;

线程控制块：
- stackTop  and stack
运行栈
- machineState
不运行的时候存储CPU寄存器
- status
running/ready/blocked

一些线程属于用户地址空间，只运行在内核的线程地址空间为空。

### thread.cc
管理线程的一些操作，包括Fork Finish Yield Sleep 
- Fork：创建一个线程，来并行的运行函数。分为两步：首先分配线程对象，然后调用Fork。
- Finish：fork结束的时候调用，处理后事
- Yield：放弃CPU并转给另一个就绪的线程
- Sleep：放弃CPU，但线程是阻塞的。即不会被再次运行除非被放回就绪队列。

#### Thread::Thread(char* threadName)
初始化线程控制块

#### Thread::~Thread()
回收线程
当前线程不能直接删除自身，因为他仍运行在我们需要删除的栈上。
如果是主线程，不能删除栈，因为我们无法回收它，它是Nachos启动的一部分。

#### void Thread::Fork(VoidFunctionPtr func, void *arg)
“func”是要同时运行的函数。
“arg”是要传递的单个参数。
允许调用方和被调用方并发执行。
定义只允许将单个整数参数传递给过程，但可以通过将多个参数设为结构的字段，并将指针作为“ arg”传递给结构来传递多个参数。
实现为以下步骤：
 1.分配一个堆栈
 2.初始化堆栈，以便对SWITCH的调用将导致其运行该过程
 3.将线程放入就绪队列

 #### void Thread::CheckOverflow()
 检查线程是否栈溢出
 Nachos 不会捕获所有的栈溢出条件，因此程序有可能因为溢出而崩溃。
 如果遇到一些奇怪的结果如页错误，可能需要扩展栈大小。
 不要定义很大的数据结构。

 #### void Thread::Finish ()
 线程完成fork时被根线程调用
 不会马上回收线程数据结构或运行栈，因为还在运行线程并且还在栈上。取而代之的是设置"threadToBeDestroyed",当运行不同现成的上下文环境时让Scheduler::Run()调用销毁器。
 禁用中断，因此在设置threadToBeDestroyed时我们不会得到时间片，转而sleep。

 #### void Thread::Yield ()
 如果有就绪的线程，放弃CPU。
 将线程放到就绪队列的末尾，这样它最终会被重新调用。
 如果就绪队列中没有线程立即返回。
 否则当线程在就绪队列头部且被重新调度时返回。
我们禁用中断，以便可以自动查看就绪列表前面的线程并切换到该线程。
 返回时，如果在禁用中断的情况下调用，则将中断级别重置为其原始状态
 有点像Thread::Sleep()，但有些不同

 #### void Thread::Sleep ()
 放弃CPU，当前线程因等待同步变量 (Semaphore, Lock, or Condition)被阻塞。
 最终这个线程会被一些线程唤醒并进入就绪队列尾，能被重新调度。
 如果就绪队列中没有线程，那么调用Interrupt::Idle，直到下次IO中断到来之前CPU都是空闲的。（唯一能导致线程从ready到run的事）
 我们假设中断已被禁用，因为它是从同步例程中调用的，同步例程必须禁用中断以实现**原子性**。 我们需要中断，以便在将第一个线程从就绪列表中拉出并切换到该线程之间没有时间片。

 #### static void ThreadFinish() 
 currentThread是指向当前正在运行的进程的指针，操作系统通过把函数地址压栈、指令读取的方式运行函数。要运行当前进程的finish()函数，无法通过currentThread->Finish()获取地址，因此设置哑函数，后续通过直接引用函数名的方式获取地址。
 - 哑函数：什么都不干的函数
 - InterruptEnable、ThreadPrint功能同上

 #### void Thread::StackAllocate (VoidFunctionPtr func, void *arg)
 分配并初始化执行栈。
 这个栈用ThreadRoot栈帧进行初始化：
 - 启用中断
 - 调用(*func)(arg)
 - 调用Thread::Finish。
 machineState：寄存器指针
