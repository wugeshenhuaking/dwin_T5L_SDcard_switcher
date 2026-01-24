/**
  ******************************************************************************
  * @file    soft_scheduler.c
  * @brief   轻量级裸机时间片调度器实现
  ******************************************************************************
  */

#include "soft_scheduler.h"

/* 
 * ---------------------------------------------------------------------------
 * [用户区域] 引用外部任务函数
 * 建议在对应的 led.h, key.h 中声明，这里引用头文件即可。
 * 这里为了演示方便，使用了 extern。
 * ---------------------------------------------------------------------------
 */
extern void Task_USB_MSC_Poll(void);    // USB大容量存储轮询 (需极快响应)
extern void Task_Key_Scan(void);        // 按键扫描
extern void Task_LED_Logic(void);       // LED逻辑 (内部含状态机)
extern void Task_Power_Ctrl(void);      // 电源管理

/* 
 * 全局系统节拍 (由SysTick驱动) 
 * 必须加 volatile 防止编译器过度优化
 */
static volatile uint32_t g_sched_tick = 0;


/* ========================================================================= */
/*                 [用户区域] 任务注册表 (核心配置)                          */
/* ========================================================================= */
static soft_task_t task_list[] = {
    /*  任务函数指针         执行周期(ms)    上次时间(固定填0)  */
    
    /* 1. USB任务：设置为0ms，代表不限速，在while(1)中全力运行 */
    {NULL,     0,              0},
    
    /* 2. 按键任务：20ms检测一次，适合消抖 */
    {NULL,         20,             0},
    
    /* 3. LED任务：10ms检测一次 (因为内部有短延时状态机，需要高频检查) */
    {NULL,        10,             0},
    
    /* 4. 电源控制：100ms维护一次即可 */
    {NULL,       100,            0},
};

/* 自动计算任务数量 */
#define TASK_COUNT  (sizeof(task_list) / sizeof(soft_task_t))


/* ========================================================================= */
/*                          调度器核心逻辑实现                               */
/* ========================================================================= */

/**
  * @brief  调度器初始化
  */
void Soft_Sched_Init(void)
{
    g_sched_tick = 0;
    // 如果有其他需要复位的调度器变量，写在这里
}

/**
  * @brief  提供给SysTick的中断服务函数调用
  * @note   每1ms调用一次
  */
void Soft_Sched_Tick_Inc(void)
{
    g_sched_tick++;
}

/**
  * @brief  获取当前系统Tick (原子读取建议，但简单场景直接读亦可)
  */
uint32_t Get_System_Tick(void)
{
    return g_sched_tick;
}

/**
  * @brief  调度器主运行函数 (放在 main 的 while(1) 中)
  */
void Soft_Sched_Run(void)
{
    uint8_t i;
    uint32_t current_tick;

    for (i = 0; i < TASK_COUNT; i++)
    {
        /* ---------------------------------------------------------
         * 特殊处理：如果周期为0，则视为"Idle Task"或"High Priority Task"
         * 每次循环无条件执行，用于USB、通信等需要最大带宽的任务
         * --------------------------------------------------------- */
        if (task_list[i].interval_ms == 0)
        {
            task_list[i].p_func();
            continue;
        }

        /* 获取当前时间快照 */
        current_tick = g_sched_tick;

        /* 
         * 核心时间片逻辑：
         * 使用 (Current - Last) >= Interval 的方式，天然解决计数器溢出问题。
         * 只要是 uint32_t，溢出回绕后的减法结果依然是正确的差值。
         */
        if ((current_tick - task_list[i].last_run_tick) >= task_list[i].interval_ms)
        {
            /* 
             * [防丢节拍策略]
             * 使用 += 累加方式，确保平均频率绝对精准。
             * 即使因为USB任务卡顿了5ms，下一次调度会立即补偿回来。
             */
            task_list[i].last_run_tick += task_list[i].interval_ms;

            /* 执行任务 */
            task_list[i].p_func();
        }
    }
}


/* ========================================================================= */
/*                      微型定时器工具实现 (Helper)                          */
/* ========================================================================= */

/**
  * @brief  启动或重置一个非阻塞定时器
  * @param  timer: 定时器句柄
  * @param  ms:    延时毫秒数
  */
void Soft_Timer_Start(soft_timer_t *timer, uint32_t ms)
{
    timer->start_tick = Get_System_Tick();
    timer->delay_ms   = ms;
    timer->is_running = 1;
}

/**
  * @brief  检查定时器是否到期
  * @retval 1: 到期 (并自动停止计时), 0: 未到期或未启动
  */
uint8_t Soft_Timer_Is_Expired(soft_timer_t *timer)
{
    if (timer->is_running == 0)
    {
        return 0;
    }

    if ((Get_System_Tick() - timer->start_tick) >= timer->delay_ms)
    {
        timer->is_running = 0; // 单次触发模式
        return 1;
    }
    return 0;
}

/**
  * @brief  检查定时器是否正在运行
  */
uint8_t Soft_Timer_Is_Running(soft_timer_t *timer)
{
    return timer->is_running;
}
