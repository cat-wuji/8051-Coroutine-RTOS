/**
 ******************************************************************************
 * @file    rtos_config.h
 * @brief   RTOS 配置文件（协程式上下文切换版）
 *
 * @note    重要：RTOS_STACK_BASE 必须 >= Keil C51 链接器分配的 ?STACK 地址。
 *          请查看 .map 文件中的 ?STACK 符号，或修改 STARTUP.A51 把栈底
 *          固定到 RTOS_STACK_BASE。默认 0x80，适用于 idata 为 256 字节、
 *          全局变量不超过 0x80 的工程。
 ******************************************************************************
 */

#ifndef __RTOS_CONFIG_H__
#define __RTOS_CONFIG_H__

/** 最大任务数（受 xdata 容量限制，每个任务软件栈 RTOS_STACK_SIZE 字节） */
#define RTOS_MAX_TASKS         2

/** 系统心跳周期（ms），由定时器中断调用 RTOS_Scheduler() 提供 */
#define RTOS_TICK_MS           1

/**
 * @brief  idata 硬件栈区起始地址
 * @note   必须 >= 链接器 ?STACK 符号的值。栈区范围 [RTOS_STACK_BASE, 0xFF]。
 *         每个任务的软件栈大小 = 0x100 - RTOS_STACK_BASE。
 */
#define RTOS_STACK_BASE        0x80

/** 每个任务的软件栈大小（字节），自动计算 */
#define RTOS_STACK_SIZE        (0x100 - RTOS_STACK_BASE)

/** 信号量最大计数值 */
#define RTOS_SEM_COUNT_MAX     255

/** 消息队列深度 */
#define RTOS_MSG_QUEUE_SIZE    8

/** 每条消息长度（字节） */
#define RTOS_MSG_QUEUE_LEN     4

#endif