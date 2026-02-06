/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  * Copyright (c) 2025, Artery Technology, All rights reserved.
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f403a_407_wk_config.h"
#include "wk_acc.h"
#include "wk_debug.h"
#include "wk_sdio.h"
#include "wk_usart.h"
#include "wk_usbfs.h"
#include "wk_gpio.h"
#include "usb_app.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include <ctype.h> // 用于显示ASCII字符
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "at32_sdio.h"
#include "bsp_dwt.h"
#include "usbd_core.h"

//#include "wk_acc.h"
//#include "ffconf.h"
//#include "ff.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */
//FATFS *fs;
//FIL *file;
//UINT bw;
//FRESULT r;
//DISK_SIZE SD_size;
//FRESULT res;

//FRESULT Read_DiskSize(DISK_SIZE *disk_size, BYTE pdrv);
/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* 定义读取多少个块，这里测试连续读 2 个块 */
#define TEST_BLOCK_COUNT  2
#define TEST_BLOCK_SIZE   512
#define TEST_BUF_SIZE     (TEST_BLOCK_COUNT * TEST_BLOCK_SIZE)


#define SD_REINIT_INTERVAL_MS  500  // SD卡重初始化间隔，500ms一次（可根据需求调整）
#define SD_INIT_TIMEOUT_MS     2000 // SD卡单次初始化超时时间
/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
volatile uint32_t g_sd_reinit_tick = 0;  // 重初始化定时计数器（基于系统滴答定时器ms）
volatile uint32_t g_debug_counter = 0; // 定义一个计数器
volatile uint8_t g_sd_is_ready = 0; 
extern usbd_core_type usb_core_dev;
extern uint32_t wk_timebase_get(void);
/* 
 * 定义接收缓冲区 
 * 注意：使用 DMA 时，缓冲区必须 4 字节对齐！
 */
__attribute__((aligned(4))) uint8_t read_buffer[TEST_BUF_SIZE];

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */
//FRESULT Read_DiskSize(DISK_SIZE *disk_size, BYTE pdrv)
//{
//	FATFS *fs;
//	FATFS *pfs;
//	
//	DWORD fre_clust;
//	char Buff[3];
//	sprintf(Buff,"%d:", pdrv);
//	fs = (FATFS*)malloc(sizeof(FATFS));
//	res = f_mount(fs,Buff,0);
//	res = f_getfree(Buff, &fre_clust, &pfs);
//	if(!res)
//	{
//		disk_size->disk_totalsize = (float)(pfs->n_fatent - 2) * pfs->csize/2/1024;
//		disk_size->disk_freesize = (float)fre_clust * pfs->csize/2/1024;
//	}
//	f_mount(NULL, Buff, 0);
//	free(fs);
//	return res;
//}

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/**
  * @brief  安全的多块读取测试 (只读不写)
  */
void SD_Safe_Read_Test(void)
{
    sd_error_status_type status = SD_OK;
    
    printf("\r\n=== SDIO Safe Multi-Block Read Test ===\r\n");

    /* 1. 确保 SD 卡已初始化 */
    /* 如果 main 函数开头已经调用过 sd_init，这里可以注释掉，或者再调一次也无妨 */
    status = sd_init();
    if(status != SD_OK)
    {
        printf("SD Init Failed! Error: %d\r\n", status);
        return;
    }
    printf("SD Init OK.\r\n");

    /* 2. 配置总线宽度为 4-bit (速度更快) */
    /* 如果你的硬件 D1-D3 没接好，这里会报错，可以改成 D1 */
    status = sd_wide_bus_operation_config(SDIO_BUS_WIDTH_D4);
    if(status != SD_OK)
    {
        printf("Warning: 4-bit mode failed, trying 1-bit...\r\n");
        sd_wide_bus_operation_config(SDIO_BUS_WIDTH_D1);
    }

    /* 3. 预填充缓冲区 (用于验证 DMA 是否真的工作) */
    memset(read_buffer, 0xCC, TEST_BUF_SIZE);

    /* 4. 执行多块读取 */
    printf("Reading %d Blocks from Address 0...\r\n", TEST_BLOCK_COUNT);
    
    /* 
     * 调用官方库的多块读取函数
     * 参数1: 接收缓冲区
     * 参数2: 读取地址 (对于 AT32 标准库，通常传 字节地址，即 0)
     * 参数3: 块大小 (512)
     * 参数4: 块数量 (2)
     */
    status = sd_mult_blocks_read(read_buffer, 0, TEST_BLOCK_SIZE, TEST_BLOCK_COUNT);

    if(status == SD_OK)
    {
        printf("Read Success! Data Dump:\r\n");
        printf("=================================================================\r\n");
        printf("Offset | Hex Data                                      | ASCII   \r\n");
        printf("-------|-----------------------------------------------|---------\r\n");

        /* 打印数据内容 */
        for (int i = 0; i < TEST_BUF_SIZE; i += 16)
        {
            if(i % 512 == 0) printf("[Block %d Start]\r\n", i/512);
            
            printf("0x%04X | ", i);

            // 打印 Hex
            for (int j = 0; j < 16; j++)
            {
                if (i + j < TEST_BUF_SIZE)
                    printf("%02X ", read_buffer[i + j]);
                else
                    printf("   ");
            }
            printf("| ");

            // 打印 ASCII (方便查看 FAT32 标志)
            for (int j = 0; j < 16; j++)
            {
                if (i + j < TEST_BUF_SIZE)
                {
                    uint8_t c = read_buffer[i + j];
                    if (isprint(c)) printf("%c", c);
                    else printf(".");
                }
            }
            printf("\r\n");
        }
        printf("=================================================================\r\n");
        
        /* 检查 MBR 标志位 */
        if(read_buffer[510] == 0x55 && read_buffer[511] == 0xAA)
        {
            printf(">>> Valid Boot Sector Signature (55 AA) found! <<<\r\n");
        }
    }
    else
    {
        printf("Read Failed! Error Code: %d\r\n", status);
        /* 即使失败，也打印前16个字节看看是不是全是 0xCC (DMA没动) 还是 0x00 (线路断) */
        printf("Buffer Head: %02X %02X %02X %02X\r\n", 
               read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]);
    }
}

// 全局变量：存储SD卡第一个分区的总块数（512字节/块）
uint32_t g_partition_start_lba = 0; // 新增：分区起始LBA（关键，避免破坏MBR）
uint32_t g_partition_total_blocks = 0;
// 512MB分区的默认块数（解析失败时降级使用）
#define DEFAULT_512MB_BLOCKS  1048576


// MBR分区表项结构（16字节，偏移0x1BE开始共4个）
typedef struct {
    uint8_t  boot_flag;        // 0x00: 引导标志（0x80为活动分区）
    uint8_t  start_chs[3];     // 0x01: 分区起始CHS地址
    uint8_t  part_type;        // 0x04: 分区类型（如0x0B=FAT32）
    uint8_t  end_chs[3];       // 0x05: 分区结束CHS地址
    uint32_t start_lba;        // 0x08: 分区起始LBA块号（核心：分区起始地址）
    uint32_t total_sectors;    // 0x0C: 分区总扇区数（即总块数，核心要获取的值）
} __attribute__((packed)) MBR_Partition_t;  // 紧凑结构体，避免字节对齐问题

/**
 * @brief  解析SD卡MBR分区表，获取第一个主分区的总块数
 * @retval 0: 解析成功，-1: 解析失败（SD卡未初始化/读块失败/无有效分区）
 */
int32_t sdcard_parse_mbr_partition(void)
{
    uint8_t mbr_buf[512] = {0};
    MBR_Partition_t *p_part = NULL;

    // 修正：判断返回值为SD_OK（AT32驱动标准返回值）
    if (sd_block_read(mbr_buf, 0, 512) != SD_OK) {
        printf("MBR读取失败：sd_block_read错误\r\n");
        return -1;
    }
    if (mbr_buf[510] != 0x55 || mbr_buf[511] != 0xAA) {
        printf("MBR无效：无55AA签名\r\n");
        return -1;
    }
    p_part = (MBR_Partition_t *)(mbr_buf + 0x1BE);
    if (p_part->part_type == 0) {
        printf("无有效主分区：分区类型为0\r\n");
        return -1;
    }
    // 保存分区总块数 + 起始LBA（新增，后续读写偏移用）
    g_partition_total_blocks = p_part->total_sectors;
    g_partition_start_lba = p_part->start_lba;
    printf("分区解析成功：起始LBA=%d，总块数=%d\r\n", g_partition_start_lba, g_partition_total_blocks);
    return 0;
}



/**
 * @brief  SD卡完整初始化（幂等设计，重复调用无副作用）
 * @retval 0-成功，1-失败
 */
static uint8_t sd_card_complete_init(void)
{
    uint8_t ret = 1;
    // 1. 底层SDIO初始化（调用AT32 SDIO驱动的初始化函数）
    if (sd_init() == SD_OK)
    {
        // 2. 获取SD卡信息（块大小、总物理块数等）
        sd_card_info_get(&sd_card_info);
        // 3. 解析MBR分区表，获取第一个分区的总块数（复用之前的分区解析逻辑）
        if (sdcard_parse_mbr_partition() == 0)
        {
            // 初始化成功：更新就绪标记+打印信息
            g_sd_is_ready = 1;
            ret = 0;
            printf("SD卡初始化成功，分区块数：%d，块大小：%d\r\n",
                   g_partition_total_blocks, sd_card_info.card_blk_size);
        }
        else
        {
            printf("SD卡初始化成功，但分区表解析失败，使用默认512MB容量\r\n");
            g_partition_total_blocks = 1048576; // 默认512MB块数
            g_sd_is_ready = 1;
            ret = 0;
        }
    }
    else
    {
        // 初始化失败：重置就绪标记
        g_sd_is_ready = 0;
        printf("SD卡初始化失败\r\n");
    }
    return ret;
}

/**
 * @brief  SD卡状态实时检测（轮询调用，检测拔卡/异常）
 */
static void sd_card_state_detect(void)
{
    sd_card_state_type sd_state;
    // 仅当SD卡就绪时，才检测状态（未就绪时无需检测，避免无效操作）
    if (g_sd_is_ready == 1)
    {
        sd_state = sd_state_get();
        // 检测到SD卡异常/断开，标记为未就绪
        if (sd_state == SD_CARD_ERROR || sd_state == SD_CARD_DISCONNECTED)
        {
            g_sd_is_ready = 0;
            printf("SD卡已拔出/异常，标记为未就绪\r\n");
            // 可选：重置SDIO底层状态（部分驱动拔卡后需要重置）
//            sd_io_reset();
        }
    }
}

/**
 * @brief  SD卡定时重初始化（在主循环调用，基于ms定时器）
 */
static void sd_card_timer_reinit(void)
{
    // 仅当SD卡未就绪时，才执行定时重初始化
    if (g_sd_is_ready == 0)
    {
        // 达到重初始化间隔（500ms）
        if (g_sd_reinit_tick >= SD_REINIT_INTERVAL_MS)
        {
            g_sd_reinit_tick = 0; // 重置定时器
            sd_card_complete_init(); // 尝试重初始化
        }
    }
    else
    {
        g_sd_reinit_tick = 0; // SD卡就绪时，重置定时器
    }
}


void SD_USB_HotPlug_Handler(void)
{
    static uint32_t last_check_tick = 0;
    uint32_t response = 0;
    
    // 每 500ms 检查一次卡状态，避免过度占用总线
    if (wk_timebase_get() - last_check_tick < 500) return;
    last_check_tick = wk_timebase_get();

    if (g_sd_is_ready) 
    {
        /* --- 场景 A：卡目前是在线状态，检查它是否被拔掉 --- */
        // sd_status_send 内部会发送 CMD13。如果卡拔掉了，会返回 SD_CMD_RSP_TIMEOUT
        if (sd_status_send(&response) != SD_OK) 
        {
            g_sd_is_ready = 0;
            printf("检测到卡已拔出！正在断开USB...\r\n");
            
            /* 关键：断开 USB 软连接，让电脑上的盘符消失 */
            usbd_disconnect(&usb_core_dev); 
        }
    }
    else 
    {
        /* --- 场景 B：卡目前不在线，尝试重新初始化 --- */
        if (sd_init() == SD_OK) 
        {
            printf("检测到新卡插入！正在初始化文件系统信息...\r\n");
            
            // 重新获取卡信息和分区表信息
            sd_card_info_get(&sd_card_info);
            if (sdcard_parse_mbr_partition() == 0) 
            {
                g_sd_is_ready = 1;
                printf("卡就绪，正在重新连接USB...\r\n");
                
                /* 关键：重新使能 USB，让电脑重新弹出 U 盘 */
                usbd_connect(&usb_core_dev); 
            }
        }
    }
}
/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */
  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* init debug function. */
  wk_debug_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config. */
  wk_timebase_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init usart1 function. */
  wk_usart1_init();

  /* init sdio2 function. */
  wk_sdio2_init();

  /* init acc function. */
  wk_acc_init();

  /* init usbfs function. */
  wk_usbfs_init();

  /* init usb app function. */
  wk_usb_app_init();

  /* add user code begin 2 */
  dwt_delay_init();
  /* 建议组合使用 */
//  debug_periph_mode_set(DEBUG_SLEEP, TRUE);     // 防止睡眠断开调试
//  debug_periph_mode_set(DEBUG_WDT_PAUSE, TRUE); // 当调试器断点停住 CPU 时，自动暂停看门狗计时，防止断点导致意外复位
  /* 尝试初始化 SD 卡 */
  printf("开始初始化SD卡...\r\n");

  sd_error_status_type sd_status = sd_init();

  if(sd_status != SD_OK)
  {
      // SD卡初始化失败！
      // 可以在这里亮一个红灯，方便你排查硬件问题
    
    g_sd_is_ready =0;
    while(1)
    {
      printf("sd card init failure!!\n");
      printf("sd card sd_status = %d\n",sd_status);
      wk_delay_ms(1000);
    }
  }
  else 
  {
    printf("sd card init success!!\n");
    printf("sd card sd_status = %d\n",sd_status);
    printf("Start Test...\r\n");
   /* 调用我给你的只读测试函数 */
//    SD_Safe_Read_Test();
    g_sd_is_ready = 1;
    // 3. 解析SD卡MBR分区表（核心步骤：提前获取分区容量，存入全局变量）
    if (sdcard_parse_mbr_partition() == 0) {
        printf("分区表解析成功，分区总块数：%d（%d MB）\r\n",
               g_partition_total_blocks,
               (g_partition_total_blocks * 512) / 1024 / 1024);
    } else {
        printf("分区表解析失败，使用默认512MB容量\r\n");
    }
  }
  
  
   if(!g_sd_is_ready) 
  {
      usbd_disconnect(&usb_core_dev); // 如果开机没卡，先关掉 USB 连接
  }
  
  
//  fs = (FATFS*)malloc(sizeof(FATFS));
//  file = (FIL*)malloc(sizeof(FIL));
//  r = f_mount(fs,"0:",0);
//  r = f_open(file,"0:/at32.txt",FA_WRITE|FA_CREATE_ALWAYS);
//  r = f_write(file,"hello",5,&bw);
//  r = f_close(file);
//  r = f_mount(NULL,"0:",0);
//  free(fs);
//  free(file);
//  Read_DiskSize(&SD_size,0);
//    uint32_t poll_timer = 0;
//    uint32_t response = 0;
/* 允许在睡眠模式下依然保持调试连接 */
  /* add user code end 2 */

  while(1)
  {
     wk_usb_app_task();

    /* add user code begin 3 */
//    printf("hello world \n");
//    wk_delay_ms(1000);

     /* 处理 SD 卡热插拔逻辑 */
     SD_USB_HotPlug_Handler();

    /* 
     * [高级技巧] 睡眠指令 (Wait For Interrupt)
     * 如果所有任务都处理完了，让 CPU 睡一会儿，等 SysTick 中断来了再醒。
     * 这可以极大地降低芯片功耗和发热。
     */
//     __WFI();  // 如果需要低功耗，取消这行的注释
      
    /* add user code end 3 */
  }
  
  /* add user code begin 4 */

  /* add user code end 4 */
}


