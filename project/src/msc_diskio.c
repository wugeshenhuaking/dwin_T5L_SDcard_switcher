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
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

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
  if (lun == SD_LUN)
  {
      /*
       * 调用底层读函数
       * 参数1: 缓冲区
       * 参数2: 字节地址 = 块地址(addr) * 512
       * 参数3: 块大小 = 512
       * 参数4: 块数量 = len
       */
      if(sd_mult_blocks_read(read_buf, (long long)addr * 512, 512, len) == SD_OK)
      {
          /* 等待传输完成 (必须加！否则数据还没拷完USB就拿走了) */
          return USB_OK; /* 直接返回 */
      }
      return USB_FAIL;
  }
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
  if (lun == SD_LUN)
  {
      /*
       * 调用底层写函数 (sd_mult_blocks_write)
       * 参数逻辑同读取
       */
      if(sd_mult_blocks_write(buf, (long long)addr * 512, 512, len) == SD_OK)
      {
          return USB_OK; /* 驱动内部已处理忙检测，直接返回成功 */
      }
      return USB_FAIL;
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
  sd_card_info_struct_type card_info;
  if (lun == SD_LUN) // 确保是 SD 卡
  {
      /* 获取卡信息 */
      if(sd_card_info_get(&card_info) == SD_OK)
      {
          /* 
           * 计算块数量。
           * card_capacity 在库里通常是字节数 (Bytes)。
           * 所以 总块数 = 总字节 / 512 
           */
          *blk_nbr = card_info.card_capacity / 512;
          *blk_size = 512;
          return USB_OK; /* 【关键】直接返回，不走后面的 switch */
      }
      else
      {
          return USB_FAIL;
      }
  }
    g_debug_counter++; 
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
