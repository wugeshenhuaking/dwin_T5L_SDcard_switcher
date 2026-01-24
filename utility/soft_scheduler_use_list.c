/**
  ******************************************************************************
  * @file    soft_scheduler_use_list.c
  ******************************************************************************
  */

#include "soft_scheduler_use_list.h"

/* 全局系统节拍 */
static volatile uint32_t g_list_tick = 0;

/* 链表头指针 (Head Pointer) */
static soft_task_t *g_task_head = NULL;

/**
  * @brief  初始化调度器
  */
void Sched_List_Init(void)
{
    g_list_tick = 0;
    g_task_head = NULL;
}

/**
  * @brief  注册任务 (采用头插法，O(1)复杂度，极快)
  * @param  task_node:   用户定义的节点变量指针 (必须是全局或静态变量)
  * @param  p_func:      任务函数名
  * @param  interval_ms: 运行周期
  */
void Sched_List_Register(soft_task_t *task_node, void (*p_func)(void), uint32_t interval_ms)
{
    if (task_node == NULL || p_func == NULL) return;

    /* 初始化节点参数 */
    task_node->p_func = p_func;
    task_node->interval_ms = interval_ms;
    task_node->last_run_tick = 0;

    /* 链表插入逻辑 (插入到头部) */
    task_node->next = g_task_head;
    g_task_head = task_node;
}

/**
  * @brief  SysTick中断调用
  */
void Sched_List_Tick_Inc(void)
{
    g_list_tick++;
}

/**
  * @brief  获取时间戳
  */
uint32_t Sched_List_Get_Tick(void)
{
    return g_list_tick;
}

/**
  * @brief  主循环调度器
  */
void Sched_List_Run(void)
{
    soft_task_t *curr = g_task_head;
    uint32_t current_tick;

    /* 遍历链表 */
    while (curr != NULL)
    {
        /* 如果周期为0，无条件运行 */
        if (curr->interval_ms == 0)
        {
            curr->p_func();
        }
        else
        {
            current_tick = g_list_tick;
            
            /* 时间差判断 (防溢出) */
            if ((current_tick - curr->last_run_tick) >= curr->interval_ms)
            {
                /* 累加法防丢节拍 */
                curr->last_run_tick += curr->interval_ms;
                
                /* 执行任务 */
                curr->p_func();
            }
        }

        /* 移动到下一个节点 */
        curr = curr->next;
    }
}
