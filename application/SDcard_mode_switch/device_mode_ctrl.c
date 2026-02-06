#include "device_mode_ctrl.h"
#include "usbd_core.h"
#include "wk_system.h"
#include "wk_sdio.h"
#include "bsp_dwt.h"
extern usbd_core_type usb_core_dev;
extern volatile uint8_t g_sd_is_ready; 
extern int32_t sdcard_parse_mbr_partition(void);
extern uint32_t wk_timebase_get(void);

static device_mode_t g_current_mode = MODE_USB_MSC;

/**
  * @brief  切换到外部透传模式 (给外部设备读卡)
  */
void Switch_To_Extension_Mode(void)
{
    printf("切换指令：进入外部透传模式...\r\n");

    /* 1. 软件断开 USB */
    /* 告诉电脑 U 盘已拔出，停止一切 USB 握手 */
    usbd_disconnect(&usb_core_dev); 
    
    /* 2. 关闭 SDIO 外设并释放引脚 */
    /* 这是最关键的一步：将 MCU 的 SDIO 引脚全部设为模拟输入(高阻态) */
    /* 防止 MCU 的引脚电平干扰外部设备的信号 */
//    sd_power_off(); // 关闭 SDIO 电源控制
    
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG; // 设为模拟高阻
    gpio_init_struct.gpio_pins = GPIO_PINS_4 | GPIO_PINS_5 | GPIO_PINS_6 | GPIO_PINS_7;
    gpio_init(GPIOA, &gpio_init_struct); // 释放 D0-D3
    
    gpio_init_struct.gpio_pins = GPIO_PINS_4 | GPIO_PINS_5;
    gpio_init(GPIOC, &gpio_init_struct); // 释放 CLK, CMD

    /* 3. 物理切换模拟开关 */
    /* PB11 拉高 -> 切向 B1 (延长线侧) */
    gpio_bits_set(GPIOB, GPIO_PINS_11);
    
    g_sd_is_ready = 0; // 标记 SD 卡不再由 MCU 管理
    g_current_mode = MODE_EXT_PASS;
//      dwt_delay_init();

    
    printf("模式切换成功：外部设备现在接管了SD卡。\r\n");
}

/**
  * @brief  切回 USB 读卡器模式 (由 MCU 读卡)
  */
void Switch_Back_To_UDisk_Mode(void)
{
    printf("切换指令：返回USB读卡器模式...\r\n");

    /* 1. 物理切换模拟开关 */
    /* PB11 拉低 -> 切向 B0 (MCU 侧) */
    gpio_bits_reset(GPIOB, GPIO_PINS_11);
    dwt_delay_ms(100);

    /* 2. 重新初始化 SDIO 引脚和外设 */
    wk_sdio2_init();
    if (sd_init() == SD_OK)
    {
        sdcard_parse_mbr_partition();
        g_sd_is_ready = 1;
        
        /* 3. 重新挂载 USB */
        usbd_connect(&usb_core_dev);
        g_current_mode = MODE_USB_MSC;
        printf("模式切换成功：MCU 已接管并重新挂载 U 盘。\r\n");
    }
    else
    {
        printf("警告：切回模式时 SD 卡初始化失败！\r\n");
    }
}

/* 按键状态定义 */
typedef enum {
    KEY_UP = 0,     // 按键抬起
    KEY_WAIT,       // 等待确认（消抖）
    KEY_DOWN        // 按键按下
} key_status_t;

/**
  * @brief  获取按键是否触发（单次触发逻辑）
  * @retval 1: 触发了按键逻辑, 0: 未触发
  */
uint8_t Get_Key_Trigger(void)
{
    static key_status_t key_state = KEY_UP;
    uint8_t current_level = gpio_input_data_bit_read(GPIOC, GPIO_PINS_7);
    uint8_t trigger = 0;

    switch (key_state) {
        case KEY_UP:
            if (current_level == RESET) { // 检测到低电平（按下）
                key_state = KEY_WAIT;
            }
            break;

        case KEY_WAIT:
            if (current_level == RESET) {
                key_state = KEY_DOWN;
                trigger = 1; // 【关键】在按下瞬间触发一次
            } else {
                key_state = KEY_UP;
            }
            break;

        case KEY_DOWN:
            if (current_level == SET) { // 检测到高电平（抬起）
                key_state = KEY_UP;
            }
            break;
    }
    return trigger;
}

void Task_Mode_Button(void)
{
    /* 假设你用一个按键（带去抖逻辑） */
    static uint32_t last_tick = 0;
    if(wk_timebase_get() - last_tick > 200)
    {

        if (Get_Key_Trigger()) 
        {
            if (g_current_mode == MODE_USB_MSC) 
            {
                Switch_To_Extension_Mode();
            } 
            else 
            {
                Switch_Back_To_UDisk_Mode();
            }
        }
        
        last_tick = wk_timebase_get(); // 更新最后一次触发时间

    }
    

}

