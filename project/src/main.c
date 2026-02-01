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

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
volatile uint32_t g_debug_counter = 0; // 定义一个计数器
volatile uint8_t g_sd_is_ready = 0; 
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

  /* 尝试初始化 SD 卡 */
  sd_error_status_type sd_status = sd_init();

  if(sd_status != SD_OK)
  {
      // SD卡初始化失败！
      // 可以在这里亮一个红灯，方便你排查硬件问题
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
    g_sd_is_ready = 1;
   /* 调用我给你的只读测试函数 */
//    SD_Safe_Read_Test();
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
  
      uint32_t poll_timer = 0;

  /* add user code end 2 */

  while(1)
  {
     wk_usb_app_task();

    /* add user code begin 3 */
//    printf("hello world \n");
    wk_delay_ms(1000);
    /* 
     * [高级技巧] 睡眠指令 (Wait For Interrupt)
     * 如果所有任务都处理完了，让 CPU 睡一会儿，等 SysTick 中断来了再醒。
     * 这可以极大地降低芯片功耗和发热。
     */
     // __WFI();  // 如果需要低功耗，取消这行的注释
      
    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */
