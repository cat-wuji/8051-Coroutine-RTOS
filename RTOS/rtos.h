#ifndef __RTOS_H__
#define __RTOS_H__

#include "RTOS/rtos_config.h"

typedef unsigned char  u8;
typedef unsigned int   u16;
typedef unsigned long  u32;

typedef void (*TaskFunc)(void);

typedef enum {
    RTOS_OK = 0,
    RTOS_ERR_FULL,
    RTOS_ERR_EMPTY,
    RTOS_ERR_PARAM,
    RTOS_ERR_TIMEOUT
} RTOS_Status;

typedef enum {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_DELAYED,
    TASK_BLOCKED,
    TASK_SUSPENDED
} TaskState;

typedef struct {
    TaskFunc   Func;                        /**< 任务入口函数 */
    u8         Priority;                    /**< 优先级，数值越大越高 */
    u8         State;                       /**< 任务状态 */
    u16        DelayTick;                   /**< 剩余延迟 tick */
    u8         TaskID;                      /**< 任务 ID */
    u8         SP;                          /**< 挂起时保存的硬件 SP */
    u8 xdata  *Stack;                       /**< 软件栈指针（保存 idata 栈区内容） */
} RTOS_TaskTCB;

typedef struct {
    u8  Count;
    u8  MaxCount;
} RTOS_Semaphore;

typedef struct {
    u8  Buf[RTOS_MSG_QUEUE_SIZE][RTOS_MSG_QUEUE_LEN];
    u8  Head;
    u8  Tail;
    u8  Count;
} RTOS_MsgQueue;

/* ---- 系统 ---- */
void RTOS_Init(void);
u8   RTOS_TaskCreate(TaskFunc func, u8 priority);
void RTOS_Start(void);

/* ---- 任务控制 ---- */
void RTOS_Yield(void);
void RTOS_TaskDelay(u16 ticks);
void RTOS_TaskSuspend(u8 task_id);
void RTOS_TaskResume(u8 task_id);

/* ---- 临界区 ---- */
void RTOS_EnterCritical(void);
void RTOS_ExitCritical(void);

/* ---- 信号量 ---- */
void        RTOS_SemCreate(RTOS_Semaphore *sem, u8 init_val);
RTOS_Status RTOS_SemTake(RTOS_Semaphore *sem, u16 timeout);
void        RTOS_SemGive(RTOS_Semaphore *sem);

/* ---- 消息队列 ---- */
void        RTOS_MsgQueueCreate(RTOS_MsgQueue *q);
RTOS_Status RTOS_MsgSend(RTOS_MsgQueue *q, u8 *pData, u16 timeout);
RTOS_Status RTOS_MsgReceive(RTOS_MsgQueue *q, u8 *pData, u16 timeout);

/* ---- 调度器 ---- */
void RTOS_Scheduler(void);
void RTOS_TickHandler(void);
void RTOS_TaskSwitch(void);  /* <-- 必须在这里声明，否则会报重定义错误 */
u8   RTOS_GetTaskCount(void);
u8   RTOS_GetCurrentTaskID(void);
u16  RTOS_GetTick(void);

#endif