# 适用于 8051 的纯 C 协程式 RTOS (8051-Coroutine-RTOS)

一个极其轻量级、专为 8051 架构（如 CA51M151）设计的协作式实时操作系统。

## 💡 核心特性
- **纯 C 语言上下文切换**：无需编写内嵌汇编（无需 `__asm`），利用 C 语言直接操作 `SP` 指针与 `RET` 指令机制实现任务切换。
- **硬件栈物理隔离**：将 8051 的 IDATA 划分两块，`0x00~0x7F` 为系统/中断栈，`0x80~0xFF` 为任务栈，实现完美隔离。
- **静态内存分配**：拒绝容易产生碎片的 `malloc`，所有任务栈使用静态 `xdata` 数组分配，极高的可靠性。
- **阻塞式写法**：支持在 `while(1)` 中直接使用 `RTOS_TaskDelay(500)`，代码逻辑线性直观。
- **完整 IPC 支持**：内置信号量（Semaphore）与消息队列（MsgQueue）。

## 📂 目录结构
- `RTOS/rtos.c` : RTOS 核心调度与切换逻辑
- `RTOS/rtos.h` : API 接口声明
- `RTOS/rtos_config.h` : 配置文件（栈大小、最大任务数）
- `main.c` : 演示示例（LED闪烁 + 按键消抖并发执行）

## 🛠️ 原理简介
本项目最大的亮点在于 `RTOS_DoSwitch` 函数：
```c
static void RTOS_DoSwitch(u8 sp) {
    SP = sp; 
}