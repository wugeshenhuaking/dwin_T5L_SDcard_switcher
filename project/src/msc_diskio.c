/* add user code begin Header */
/**
  **************************************************************************
  * @file     msc_diskio.c
  * @brief    usb mass storage disk function
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

#include "msc_diskio.h"
#include "msc_bot_scsi.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "wk_sdio.h"            // 引入 WorkBench 生成的 SDIO 驱动
#include "at32f403a_407_sdio.h" // 引入底层驱动定义
#include "at32_sdio.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
extern volatile uint32_t g_debug_counter; // 引用它
/* 定义一个 4字节对齐的中间缓冲区，大小为一个扇区 512 字节 */
__attribute__((aligned(4))) uint8_t sd_dma_buffer[512]; 
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */
/* 
 * 辅助函数：等待 SD 卡回到传输状态 (空闲)
 * 写操作后必须调用，否则连续写入会报错
 */
static void wait_sd_ready(void)
{
    // 获取状态需要时间，循环检测直到状态变为 4 (Transfer Mode)
    // 防止死循环，加个计数器（虽然一般不会死）
    uint32_t timeout = 0xFFFFF;
    while(sd_state_get() != SD_CARD_TRANSFER && timeout--);
}
/* add user code end 0 */

uint8_t scsi_inquiry[MSC_SUPPORT_MAX_LUN][SCSI_INQUIRY_DATA_LENGTH] =
{
  /* lun = 0 */
  {
    0x00,         /* peripheral device type (direct-access device) */
    0x80,         /* removable media bit */
    0x00,         /* ansi version, ecma version, iso version */
    0x01,         /* respond data format */
    SCSI_INQUIRY_DATA_LENGTH - 5, /* additional length */
    0x00, 0x00, 0x00, /* reserved */
    'A', 'T', '3', '2', ' ', ' ', ' ', ' ', /* vendor information "AT32" */
    'D', 'i', 's', 'k', '0', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', /* Product identification "Disk" */
    '2', '.', '0', '0'  /* product revision level */
  }
};

/**
  * @brief  get disk inquiry
  * @param  lun: logical units number
  * @retval inquiry string
  */
uint8_t *get_inquiry(uint8_t lun)
{
  /* add user code begin get_inquiry 0 */

  /* add user code end get_inquiry 0 */

  if(lun < MSC_SUPPORT_MAX_LUN)
    return (uint8_t *)scsi_inquiry[lun];
  else
    return NULL;

  /* add user code begin get_inquiry 1 */

  /* add user code end get_inquiry 1 */
}

/**
  * @brief  disk read
  * @param  lun: logical units number
  * @param  addr: logical address
  * @param  read_buf: pointer to read buffer
  * @param  len: read length
  * @retval status of usb_sts_type
  */
usb_sts_type msc_disk_read(uint8_t lun, uint64_t addr, uint8_t *read_buf, uint32_t len)
{
  /* add user code begin msc_disk_read 0 */
    extern volatile uint32_t g_debug_counter;

  extern volatile uint8_t g_sd_is_ready; // 引用
  g_debug_counter++;

//  if (g_sd_is_ready == 0)
//  {
//      return USB_FAIL; // 卡都没了，直接拒单
//  }
  // 强制接管 LUN 0
  if (lun == 0 || lun == SD_LUN)
  {
      usb_sts_type res;
      res = (usb_sts_type)sd_read_disk(read_buf, addr/512, len/512);
      return res;

  }
  
  return USB_FAIL; // 其他情况返回 Fail
  /* add user code end msc_disk_read 0 */

  switch(lun)
  {
    case INTERNAL_FLASH_LUN:
      break;
    case SPI_FLASH_LUN:
      break;
    case SD_LUN:
      break;
    default:
      break;
  }

  /* add user code begin msc_disk_read 1 */

  /* add user code end msc_disk_read 1 */

  return USB_OK;
}

/**
  * @brief  disk write
  * @param  lun: logical units number
  * @param  addr: logical address
  * @param  buf: pointer to write buffer
  * @param  len: write length
  * @retval status of usb_sts_type
  */
usb_sts_type msc_disk_write(uint8_t lun, uint64_t addr, uint8_t *buf, uint32_t len)
{
  /* add user code begin msc_disk_write 0 */
  
  /* 在 msc_disk_read 和 msc_disk_write 的最开头加入 */
  extern volatile uint8_t g_sd_is_ready; // 引用

//  if (g_sd_is_ready == 0)
//  {
//      return USB_FAIL; // 卡都没了，直接拒单
//  }
  if (lun == 0 || lun == SD_LUN)
  {
    usb_sts_type res;
    res = (usb_sts_type)sd_write_disk(buf, addr/512, len/512);
    return res;

  }
  /* add user code end msc_disk_write 0 */

  switch(lun)
  {
    case INTERNAL_FLASH_LUN:
      break;
    case SPI_FLASH_LUN:
      break;
    case SD_LUN:
      break;
    default:
      break;;
  }

  /* add user code begin msc_disk_write 1 */

  /* add user code end msc_disk_write 1 */

  return USB_OK;
}

/**
  * @brief  disk capacity
  * @param  lun: logical units number
  * @param  blk_nbr: pointer to number of block
  * @param  blk_size: pointer to block size
  * @retval status of usb_sts_type
  */
usb_sts_type msc_disk_capacity(uint8_t lun, uint32_t *blk_nbr, uint32_t *blk_size)
{
  /* add user code begin msc_disk_capacity 0 */
 /* 调试计数器 */
  extern volatile uint32_t g_debug_counter;
  extern volatile uint8_t g_sd_is_ready; // 引用

  g_sd_is_ready =1 ;

  if (lun == 0 || lun == SD_LUN) 
  {
      *blk_nbr =sd_card_info.card_capacity/512;
      *blk_size = sd_card_info.card_blk_size;
      return USB_OK; // 此处返回OK，避免枚举失败

  }
  
  /* add user code end msc_disk_capacity 0 */

  switch(lun)
  {
    case INTERNAL_FLASH_LUN:
      break;
    case SPI_FLASH_LUN:
      break;
    case SD_LUN:
      break;
    default:
      break;
  }

  /* add user code begin msc_disk_capacity 1 */

  /* add user code end msc_disk_capacity 1 */

  return USB_OK;
}

/* add user code begin 1 */

/* add user code end 1 */
