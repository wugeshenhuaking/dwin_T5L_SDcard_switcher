#include "bsp_delay.h"

/* 
 * 核心变量：每微秒需要的循环次数 
 * static 限制作用域，防止外部乱改
 */
static uint32_t s_fac_us = 0;

/*
 * 经验值：Cortex-M4 内核在 Flash 运行代码时，
 * 一个简单的 while(i--) 循环大约消耗 4~5 个时钟周期。
 * 考虑到流水线和 Flash 等待周期，我们取 5 作为分母比较稳妥。
 */
#define CYCLES_PER_LOOP  5 

/**
  * @brief  初始化软件延时 (在 main 开头，时钟配置完成后调用)
  */
void soft_delay_init(void)
{
    /* 
     * 计算公式：
     * 1us 的时钟周期数 = SystemCoreClock / 1000000
     * 需要循环的次数 = 周期数 / 每次循环消耗的周期数
     */
    if (system_core_clock > 0) 
    {
        s_fac_us = (system_core_clock / 1000000) / CYCLES_PER_LOOP;
    }
    else
    {
        /* 防止未初始化导致除零，给个保底值 (假设 144MHz) */
        s_fac_us = 144 / CYCLES_PER_LOOP; 
    }
}

/**
  * @brief  微秒级延时 (非中断模式，纯软件死循环)
  * @param  us: 延时微秒数
  * @note   此函数即使在关中断状态下也能运行，不会破坏 SysTick
  */
// 防止编译器把这个函数优化没了，强制 O0 优化 (针对 Keil/ARMCC)
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#pragma O0
#elif defined(__GNUC__)
__attribute__((optimize("O0")))
#endif
void soft_delay_us(uint32_t us)
{
    /* 
     * 使用 volatile 告诉编译器：
     * 这个变量很重要，不要去预测它，老老实实去读写内存。
     */
    volatile uint32_t i;

    /* 计算总循环次数 */
    i = us * s_fac_us;

    /* 死循环等待 */
    while(i--);
}

/**
  * @brief  毫秒级延时
  */
void soft_delay_ms(uint32_t ms)
{
    while(ms--)
    {
        soft_delay_us(1000);
    }
}

