/**
  ******************************************************************************
  * @file    soft_scheduler_use_protothreads.c
  ******************************************************************************
  */

#include "soft_scheduler_use_protothreads.h"

/* 
 * 在这里声明你的任务函数 
 * 注意：协程任务必须返回 int (0表示正在运行/等待，1表示结束)
 * 并且带一个 pt_t* 参数
 */
extern int Task_LED_Coroutine(pt_t *pt);
extern int Task_Power_Logic(pt_t *pt);

/* 
 * 定义协程控制变量 
 */
static pt_t pt_led;
static pt_t pt_power;

/* 全局 Tick */
static volatile uint32_t g_pt_tick = 0;

void PT_Sched_Init(void)
{
    g_pt_tick = 0;
    
    /* 初始化所有协程变量 */
    PT_INIT(&pt_led);
    PT_INIT(&pt_power);
}

void PT_Sched_Tick_Inc(void)
{
    g_pt_tick++;
}

uint32_t PT_Get_Tick(void)
{
    return g_pt_tick;
}

/**
  * @brief 协程调度器
  *        原理：轮询调用所有协程函数。
  *        协程函数内部通过 PT_WAIT_UNTIL 自动决定是继续跑还是立即返回。
  */
void PT_Sched_Run(void)
{
    /* 
     * 依次调用各个协程任务
     * 它们会根据上次退出的位置继续运行，绝不阻塞
     */
//    Task_LED_Coroutine(&pt_led);
//    
//    Task_Power_Logic(&pt_power);
}


///* led.c 使用协程写法 */
//#include "soft_scheduler_use_protothreads.h"

//static pt_timer_t tmr_led; // 定义一个定时器变量

///* 
// * 任务：闪烁两次，停顿1秒，循环
// * 看起来像死循环，其实是异步非阻塞的！
// */
//int Task_LED_Coroutine(pt_t *pt)
//{
//    PT_BEGIN(pt); // 协程开始

//    while(1) 
//    {
//        // 第一次亮
//        LED_ON();
//        PT_DELAY_MS(pt, 50, &tmr_led); // 延时50ms (此处会立即return，下次进来跳到下一行)

//        // 第一次灭
//        LED_OFF();
//        PT_DELAY_MS(pt, 100, &tmr_led);

//        // 第二次亮
//        LED_ON();
//        PT_DELAY_MS(pt, 50, &tmr_led);

//        // 第二次灭，长等待
//        LED_OFF();
//        PT_DELAY_MS(pt, 1000, &tmr_led);
//    }

//    PT_END(pt); // 协程结束
//}

