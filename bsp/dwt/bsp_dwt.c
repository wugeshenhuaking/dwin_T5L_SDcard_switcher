#include "bsp_dwt.h"


#define STEP_DELAY_MS                    50


/* delay variable */


/* 1. 初始化 DWT (在 main 开头调用一次) */
void dwt_delay_init(void)
{
    /* 使能 DWT 外设 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    /* 清空计数器 */
    DWT->CYCCNT = 0;
    /* 启动计数器 */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* 2. 微秒延时 (非阻塞硬件轮询) */
void dwt_delay_us(uint32_t us)
{
    uint32_t start_tick = DWT->CYCCNT;
    /* 
     * SystemCoreClock 是系统主频 (例如 144000000)
     * delay_ticks = us * (144000000 / 1000000) = us * 144
     */
    uint32_t delay_ticks = us * (SystemCoreClock / 1000000);

    /* 死循环等待，直到时间走完 */
    /* 利用无符号减法，天然解决计数器溢出问题 */
    while ((DWT->CYCCNT - start_tick) < delay_ticks);
}


/**
  * @brief  毫秒级延时 (DWT实现)
  * @param  ms: 延时毫秒数
  */
void dwt_delay_ms(uint32_t ms)
{
    /* 
     * 1. 预先计算 1ms 需要的 Tick 数 
     *    例如 144MHz 主频，ticks_per_ms = 144000
     */
    uint32_t ticks_per_ms = SystemCoreClock / 1000;
    uint32_t start_tick;

    /* 
     * 2. 循环 ms 次，每次延时 1ms
     *    这样可以防止计算总 Ticks 时导致 uint32_t 溢出
     *    (例如在 144MHz 下，一次性延时超过 29秒 就会溢出)
     */
    while (ms--)
    {
        start_tick = DWT->CYCCNT;
        
        /* 等待 1ms 过去 */
        while ((DWT->CYCCNT - start_tick) < ticks_per_ms);
    }
}
