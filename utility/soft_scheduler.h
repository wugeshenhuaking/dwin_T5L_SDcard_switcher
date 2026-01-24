/**
  ******************************************************************************
  * @file    soft_scheduler.h
  * @author  Embeded Engineer
  * @brief   轻量级裸机时间片调度器 (Software Scheduler)
  *          包含任务调度核心与非阻塞微型定时器辅助工具
  ******************************************************************************
  */

#ifndef __SOFT_SCHEDULER_H__
#define __SOFT_SCHEDULER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

/* ========================================================================= */
/*                               核心类型定义                                */
/* ========================================================================= */

/**
  * @brief 任务配置结构体
  */
typedef struct {
    void (*p_func)(void);       /*!< 任务函数指针 */
    uint32_t interval_ms;       /*!< 执行周期(ms)。设为0表示每轮循环都执行(最高优先级) */
    uint32_t last_run_tick;     /*!< 上次运行的时间戳(内部维护，初始化填0) */
} soft_task_t;

/**
  * @brief 非阻塞微型定时器结构体 (用于任务内部的小延时)
  */
typedef struct {
    uint32_t start_tick;        /*!< 启动时刻 */
    uint32_t delay_ms;          /*!< 设定的延时时长 */
    uint8_t  is_running;        /*!< 运行状态标记 */
} soft_timer_t;


/* ========================================================================= */
/*                               函数接口声明                                */
/* ========================================================================= */

/* 调度器控制函数 */
void Soft_Sched_Init(void);      // 初始化
void Soft_Sched_Run(void);       // 主循环调用
void Soft_Sched_Tick_Inc(void);  // SysTick中断调用

/* 系统时间获取 */
uint32_t Get_System_Tick(void);

/* 微型定时器工具 (Helper Functions) */
void    Soft_Timer_Start(soft_timer_t *timer, uint32_t ms);   // 启动/重置定时器
uint8_t Soft_Timer_Is_Expired(soft_timer_t *timer);           // 检查是否到期(一次性)
uint8_t Soft_Timer_Is_Running(soft_timer_t *timer);           // 检查是否在运行中

#ifdef __cplusplus
}
#endif

#endif /* __APP_SOFT_SCHED_H__ */
