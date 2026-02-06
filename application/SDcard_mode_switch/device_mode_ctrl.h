#ifndef DEVICE_MODE_CTRL_H
#define DEVICE_MODE_CTRL_H

#include "at32_sdio.h"

typedef enum {
    MODE_USB_MSC = 0,
    MODE_EXT_PASS = 1
} device_mode_t;

// void device_mode_init(void);
// void device_mode_set(device_mode_t mode);
// device_mode_t device_mode_get(void);

void Task_Mode_Button(void);

#endif /* DEVICE_MODE_CTRL_H */
