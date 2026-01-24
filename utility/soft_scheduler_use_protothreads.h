/**
  ******************************************************************************
  * @file    soft_scheduler_use_protothreads.h
  * @brief   基于 Protothreads 的协程调度器核心宏定义
  *          Original idea by Adam Dunkels
  ******************************************************************************
  */

#ifndef __SOFT_SCHEDULER_USE_PT_H__
#define __SOFT_SCHEDULER_USE_PT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ================== 核心结构体 ================== */

/* 协程控制块 (只需保存行号) */
typedef struct {
    uint16_t lc; 
} pt_t;

/* 协程专用非阻塞定时器 */
typedef struct {
    uint32_t start;
    uint32_t interval;
} pt_timer_t;


/* ================== Protothreads 核心宏 (黑魔法区域) ================== */

/* 初始化协程 */
#define PT_INIT(pt)             (pt)->lc = 0

/* 协程开始：本质是一个 switch 跳转 */
#define PT_BEGIN(pt)            switch((pt)->lc) { case 0:

/* 等待条件满足 (如果不满足，return PT_WAITING 并保存行号) */
#define PT_WAIT_UNTIL(pt, cond) \
    do { \
        (pt)->lc = __LINE__; \
        case __LINE__: \
        if(!(cond)) return 0; \
    } while(0)

/* 协程结束 */
#define PT_END(pt)              } (pt)->lc = 0; return 1


/* ================== 辅助功能宏 ================== */

/* 非阻塞延时 (最常用的功能) 
 * 参数：pt(协程指针), ms(毫秒数), tmr(定时器变量指针)
 */
#define PT_DELAY_MS(pt, ms, tmr) \
    do { \
        (tmr)->start = PT_Get_Tick(); \
        (tmr)->interval = (ms); \
        PT_WAIT_UNTIL(pt, (PT_Get_Tick() - (tmr)->start) >= (tmr)->interval); \
    } while(0)

/* 协程让出 CPU (Yield) */
#define PT_YIELD(pt) \
    do { \
        (pt)->lc = __LINE__; \
        case __LINE__: \
        return 0; \
    } while(0)


/* ================== 外部接口 ================== */

void PT_Sched_Init(void);
void PT_Sched_Tick_Inc(void);
void PT_Sched_Run(void);
uint32_t PT_Get_Tick(void);

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_SCHEDULER_USE_PT_H__ */

