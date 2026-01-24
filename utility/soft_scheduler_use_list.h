/**
  ******************************************************************************
  * @file    soft_scheduler_use_list.h
  * @brief   基于链表的动态任务调度器
  *          支持模块化开发，任务自注册，无最大数量限制
  ******************************************************************************
  */

#ifndef __SOFT_SCHEDULER_USE_LIST_H__
#define __SOFT_SCHEDULER_USE_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* 
 * 任务控制块 (TCB) 
 * 注意：用户在使用时，应在自己的.c文件中定义该结构体的静态变量，
 * 不要作为局部变量定义在函数栈中。
 */
typedef struct soft_task_node {
    void (*p_func)(void);           /* 任务函数指针 */
    uint32_t interval_ms;           /* 运行周期 (ms) */
    uint32_t last_run_tick;         /* 上次运行时间戳 (内部维护) */
    struct soft_task_node *next;    /* 指向下一个任务的指针 (链表核心) */
} soft_task_t;

/* ================== API 接口 ================== */

/* 系统初始化 */
void Sched_List_Init(void);

/* 注册一个新任务 (核心函数) */
void Sched_List_Register(soft_task_t *task_node, void (*p_func)(void), uint32_t interval_ms);

/* 在SysTick中断中调用 */
void Sched_List_Tick_Inc(void);

/* 在主循环 while(1) 中调用 */
void Sched_List_Run(void);

/* 获取系统当前Tick */
uint32_t Sched_List_Get_Tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_SCHEDULER_USE_LIST_H__ */
