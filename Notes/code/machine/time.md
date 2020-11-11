## timer.h
用于模拟硬件计时器的数据结构。
硬件计时器每X毫秒生成一次CPU中断。
可用于实现时间分片，或用于使线程在特定时间段内进入睡眠状态。
通过调度每次stats-> totalTicks增加TimerTicks时发生的中断来模拟硬件计时器。
为了在时间分片中引入一些随机性，如果设置了“ doRandom”，那么中断将在随机的滴答声之后发生。

#### Timer::Timer(VoidFunctionPtr timerHandler, int callArg, bool doRandom)
初始化硬件计时器设备。 保存要在每个中断上调用的位置，然后安排计时器开始生成中断。
-“ timerHandler”是计时器设备的中断处理程序。
每次计时器到期时，都会在**禁用中断**的情况下调用它。
“ callArg”是要传递给中断处理程序的参数。
“ doRandom”-如果为true，则安排中断以随机而不是固定的间隔发生。

## interrupt.h
用于模拟低级中断硬件的数据结构。
硬件提供了一个例程（SetLevel）来启用或禁用中断。

为了模拟硬件，我们需要跟踪硬件设备将引起的所有中断以及应该在何时发生。

此模块还跟踪模拟时间。时间仅在发生以下情况时才进行：
- 重新启用中断
- 执行一条用户指令
- 准备就绪队列中没有任何内容

因此，与真正的硬件不同，中断（以及时间片上下文切换）**不能**在启用了中断的代码中的任何位置发生，而只能在代码中**模拟时间进行**的那些位置发生（这样就变成了时间）在硬件仿真中调用中断）。

- 中断状态：IntOff(禁止) IntOn(开启)
- 机器状态： 空闲态IdleMode, 系统态SystemMode, 用户态UserMode
- 中断类型：TimerInt, DiskInt, ConsoleWriteInt, ConsoleReadInt, 	ElevatorInt, NetworkSendInt, NetworkRecvInt(时间设备、控制台输入输出、键盘、网络)