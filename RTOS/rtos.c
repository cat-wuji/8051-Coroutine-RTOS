/**
 ******************************************************************************
 * @file    rtos.c
 * @brief   RTOS 实现（协程式上下文切换版，8051/Keil C51）
 *
 * @note    上下文切换原理：
 *          1. 任务调用 RTOS_Yield()/RTOS_TaskDelay() 等主动让出点；
 *          2. 最终进入 RTOS_TaskSwitch()；
 *          3. 保存硬件 SP，用 C 把 idata 栈区 [RTOS_STACK_BASE .. SP]
 *             拷贝到当前任务的 xdata 软件栈；
 *          4. 选出下一个任务，把它的软件栈内容拷回 idata；
 *          5. 用纯 C 语言函数设置 SP 为新任务的 SP，然后 return；
 *          6. C51 生成 RET 从新栈弹出返回地址，自动跳回新任务上次
 *             RTOS_TaskSwitch() 的返回点，即 Yield/Delay 之后继续执行。
 *
 *          定时器中断中的 RTOS_Scheduler() 只做 TickHandler，不切换任务，
 *          避免在中断栈帧上改 SP 导致崩溃。
 ******************************************************************************
 */

#include "RTOS/rtos.h"
#include "include/ca51m151sfr.h"

/*========================================================================== */
/*                              私有全局变量                                   */
/*========================================================================== */

static RTOS_TaskTCB xdata TaskTable[RTOS_MAX_TASKS];

/* 新增：静态分配所有任务的软件栈，避免 malloc 失败风险 */
static u8 xdata TaskStacks[RTOS_MAX_TASKS][RTOS_STACK_SIZE];

static u8  data CurrentTaskID  = 0;
static u8  data TaskCount      = 0;
static u8  data SchedulerLock  = 0;
static u16 data SysTick        = 0;

/*========================================================================== */
/*                          私有函数：栈拷贝                                    */
/*========================================================================== */

/**
 * @brief  把 idata 栈区 [RTOS_STACK_BASE .. sp] 拷贝到 xdata 软件栈
 */
static void RTOS_SaveStack(u8 xdata *dst, u8 sp) {
    u8 idata *src;
    u16 n;

    if (sp < RTOS_STACK_BASE) return;

    src = (u8 idata *)RTOS_STACK_BASE;
    n   = (u16)(sp - RTOS_STACK_BASE + 1);
    while (n--) {
        *dst++ = *src++;
    }
}

/**
 * @brief  把 xdata 软件栈内容恢复到 idata 栈区 [RTOS_STACK_BASE .. sp]
 */
static void RTOS_RestoreStack(u8 xdata *src, u8 sp) {
    u8 idata *dst;
    u16 n;

    if (sp < RTOS_STACK_BASE) return;

    dst = (u8 idata *)RTOS_STACK_BASE;
    n   = (u16)(sp - RTOS_STACK_BASE + 1);
    while (n--) {
        *dst++ = *src++;
    }
}

/*========================================================================== */
/*                          私有函数：选择下一个任务                             */
/*========================================================================== */

/**
 * @brief  选择优先级最高的就绪任务
 * @return 任务 ID；0xFF 表示没有就绪任务
 */
static u8 RTOS_SelectNext(void) {
    u8 i;
    u8 best      = 0xFF;
    u8 best_prio = 0;

    for (i = 0; i < TaskCount; i++) {
        if (TaskTable[i].State == TASK_DELAYED) {
            if (TaskTable[i].DelayTick > 0) continue;
            TaskTable[i].State = TASK_READY;
        }

        if (TaskTable[i].State == TASK_READY) {
            if (best == 0xFF || TaskTable[i].Priority > best_prio) {
                best      = i;
                best_prio = TaskTable[i].Priority;
            }
        }
    }
    return best;
}

/*========================================================================== */
/*                          私有函数：任务入口包装                              */
/*========================================================================== */

/**
 * @brief  新任务的启动入口
 */
static void RTOS_TaskEntry(void) {
    TaskTable[CurrentTaskID].Func();
    TaskTable[CurrentTaskID].State = TASK_SUSPENDED;
    RTOS_TaskSwitch();
    while (1);
}

/*========================================================================== */
/*                          任务切换核心 (纯 C 语言实现)                        */
/*========================================================================== */

/* 
 * 这个跳板函数只有一句代码，编译器会将其编译为 MOV SP, R7 然后直接 RET。
 * 当该函数 RET 时，硬件会自动从新栈弹出返回地址，完成无缝跳转。
 */
static void RTOS_DoSwitch(u8 sp) {
    SP = sp; 
}

void RTOS_TaskSwitch(void) {
    u8 next;
    u8 sp;
    u8 cur_state;

    /* 1. 保存当前硬件 SP */
    sp = SP;

    /* 2. 当前 RUNNING 临时改为 READY，参与优先级比较 */
    cur_state = TaskTable[CurrentTaskID].State;
    if (cur_state == TASK_RUNNING) {
        TaskTable[CurrentTaskID].State = TASK_READY;
    }

    /* 3. 选下一个 */
    next = RTOS_SelectNext();

    if (next == 0xFF || next == CurrentTaskID) {
        /* 不切换：恢复状态后直接返回 */
        if (cur_state == TASK_RUNNING) {
            TaskTable[CurrentTaskID].State = TASK_RUNNING;
        }
        return;
    }

    /* 4. 保存当前任务上下文 */
    TaskTable[CurrentTaskID].SP = sp;
    RTOS_SaveStack(TaskTable[CurrentTaskID].Stack, sp);

    /* 5. 恢复下一个任务上下文 */
    sp = TaskTable[next].SP;
    RTOS_RestoreStack(TaskTable[next].Stack, sp);
    TaskTable[next].State = TASK_RUNNING;
    CurrentTaskID = next;

    /* 6. 触发跳板函数进行切换 */
    RTOS_DoSwitch(sp);
}

/*========================================================================== */
/*                          系统初始化与启动                                   */
/*========================================================================== */

void RTOS_Init(void) {
    u8 i;

    for (i = 0; i < RTOS_MAX_TASKS; i++) {
        TaskTable[i].Func      = 0;
        TaskTable[i].Priority  = 0;
        TaskTable[i].State     = TASK_SUSPENDED;
        TaskTable[i].DelayTick = 0;
        TaskTable[i].TaskID    = i;
        TaskTable[i].SP        = 0;
        TaskTable[i].Stack     = 0;
    }

    TaskCount     = 0;
    CurrentTaskID = 0;
    SchedulerLock = 0;
    SysTick       = 0;
}
 
u8 RTOS_TaskCreate(TaskFunc func, u8 priority) {
    u8  id;
    u16 entry;

    if (TaskCount >= RTOS_MAX_TASKS) return 0xFF;
    if (func == 0) return 0xFF;

    id = TaskCount;

    TaskTable[id].Func      = func;
    TaskTable[id].Priority  = priority;
    TaskTable[id].State     = TASK_READY;
    TaskTable[id].DelayTick = 0;
    TaskTable[id].TaskID    = id;

    /* 使用静态分配的栈，彻底告别 malloc 失败风险 */
    TaskTable[id].Stack = TaskStacks[id];

    /* 构造初始栈 */
    entry = (u16)RTOS_TaskEntry;
    TaskTable[id].Stack[0] = 0x00;             /* 栈底占位 */
    TaskTable[id].Stack[1] = (u8)entry;        /* 返回地址低字节 */
    TaskTable[id].Stack[2] = (u8)(entry >> 8); /* 返回地址高字节 */
    TaskTable[id].SP       = RTOS_STACK_BASE + 2;

    TaskCount++;
    return id;
}

void RTOS_Start(void) {
    u8 i;

    /* 找到第一个就绪任务作为初始运行任务 */
    for (i = 0; i < TaskCount; i++) {
        if (TaskTable[i].State == TASK_READY) {
            CurrentTaskID = i;
            TaskTable[i].State = TASK_RUNNING;
            break;
        }
    }

    if (i >= TaskCount) {
        while (1);
    }

    /* 恢复第一个任务的栈，设置 SP，return 时 RET 跳到 RTOS_TaskEntry */
    RTOS_RestoreStack(TaskTable[CurrentTaskID].Stack, TaskTable[CurrentTaskID].SP);
    RTOS_DoSwitch(TaskTable[CurrentTaskID].SP);
    
    /* RTOS_DoSwitch 执行后不会回到这里 */
    while(1);
}

/*========================================================================== */
/*                          任务控制 API                                       */
/*========================================================================== */

void RTOS_Yield(void) {
    if (SchedulerLock == 0) {
        RTOS_TaskSwitch();
    }
}

void RTOS_TaskDelay(u16 ticks) {
    if (ticks == 0) {
        RTOS_Yield();
        return;
    }
    if (CurrentTaskID >= TaskCount) return;

    TaskTable[CurrentTaskID].DelayTick = ticks;
    TaskTable[CurrentTaskID].State     = TASK_DELAYED;

    /* 循环直到延迟到期或被恢复 */
    while (TaskTable[CurrentTaskID].State == TASK_DELAYED) {
        RTOS_TaskSwitch();
    }
    TaskTable[CurrentTaskID].State = TASK_RUNNING;
}

void RTOS_TaskSuspend(u8 task_id) {
    if (task_id >= TaskCount) return;

    TaskTable[task_id].State = TASK_SUSPENDED;

    if (task_id == CurrentTaskID) {
        while (TaskTable[CurrentTaskID].State == TASK_SUSPENDED) {
            RTOS_TaskSwitch();
        }
        TaskTable[CurrentTaskID].State = TASK_RUNNING;
    }
}

void RTOS_TaskResume(u8 task_id) {
    if (task_id >= TaskCount) return;
    if (TaskTable[task_id].State == TASK_SUSPENDED) {
        TaskTable[task_id].State = TASK_READY;
    }
}

/*========================================================================== */
/*                          临界区                                            */
/*========================================================================== */

void RTOS_EnterCritical(void) { SchedulerLock = 1; }
void RTOS_ExitCritical(void)  { SchedulerLock = 0; }

/*========================================================================== */
/*                          信号量                                            */
/*========================================================================== */

void RTOS_SemCreate(RTOS_Semaphore *sem, u8 init_val) {
    if (sem) {
        sem->Count    = init_val;
        sem->MaxCount = RTOS_SEM_COUNT_MAX;
    }
}

RTOS_Status RTOS_SemTake(RTOS_Semaphore *sem, u16 timeout) {
    u16 waited = 0;

    if (sem == 0) return RTOS_ERR_PARAM;

    while (sem->Count == 0) {
        if (timeout == 0) return RTOS_ERR_TIMEOUT;
        if (timeout != 0xFFFF && waited >= timeout) return RTOS_ERR_TIMEOUT;

        RTOS_TaskDelay(1);
        waited++;
    }

    sem->Count--;
    return RTOS_OK;
}

void RTOS_SemGive(RTOS_Semaphore *sem) {
    if (sem) {
        if (sem->Count < sem->MaxCount) {
            sem->Count++;
        }
    }
}

/*========================================================================== */
/*                          消息队列                                          */
/*========================================================================== */

void RTOS_MsgQueueCreate(RTOS_MsgQueue *q) {
    if (q) {
        q->Head  = 0;
        q->Tail  = 0;
        q->Count = 0;
    }
}

RTOS_Status RTOS_MsgSend(RTOS_MsgQueue *q, u8 *pData, u16 timeout) {
    u16 waited = 0;
    u8  i;

    if (q == 0 || pData == 0) return RTOS_ERR_PARAM;

    while (q->Count >= RTOS_MSG_QUEUE_SIZE) {
        if (timeout == 0) return RTOS_ERR_TIMEOUT;
        if (timeout != 0xFFFF && waited >= timeout) return RTOS_ERR_TIMEOUT;

        RTOS_TaskDelay(1);
        waited++;
    }

    for (i = 0; i < RTOS_MSG_QUEUE_LEN; i++) {
        q->Buf[q->Tail][i] = pData[i];
    }
    q->Tail++;
    if (q->Tail >= RTOS_MSG_QUEUE_SIZE) q->Tail = 0;
    q->Count++;

    return RTOS_OK;
}

RTOS_Status RTOS_MsgReceive(RTOS_MsgQueue *q, u8 *pData, u16 timeout) {
    u16 waited = 0;
    u8  i;

    if (q == 0 || pData == 0) return RTOS_ERR_PARAM;

    while (q->Count == 0) {
        if (timeout == 0) return RTOS_ERR_TIMEOUT;
        if (timeout != 0xFFFF && waited >= timeout) return RTOS_ERR_TIMEOUT;

        RTOS_TaskDelay(1);
        waited++;
    }

    for (i = 0; i < RTOS_MSG_QUEUE_LEN; i++) {
        pData[i] = q->Buf[q->Head][i];
    }
    q->Head++;
    if (q->Head >= RTOS_MSG_QUEUE_SIZE) q->Head = 0;
    q->Count--;

    return RTOS_OK;
}

/*========================================================================== */
/*                          调度器与系统信息                                   */
/*========================================================================== */

void RTOS_TickHandler(void) {
    u8 i;

    SysTick++;

    for (i = 0; i < TaskCount; i++) {
        if (TaskTable[i].State == TASK_DELAYED) {
            if (TaskTable[i].DelayTick > 0) {
                TaskTable[i].DelayTick--;
                if (TaskTable[i].DelayTick == 0) {
                    TaskTable[i].State = TASK_READY;
                }
            }
        }
    }
}

/**
 * @brief  定时器中断中调用：只处理 tick，不切换任务
 */
void RTOS_Scheduler(void) {
    RTOS_TickHandler();
}

u8  RTOS_GetTaskCount(void)     { return TaskCount; }
u8  RTOS_GetCurrentTaskID(void) { return CurrentTaskID; }
u16 RTOS_GetTick(void)          { return SysTick; }