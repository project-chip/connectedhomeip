/**
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon
 *Laboratories Inc. Your use of this software is
 *governed by the terms of Silicon Labs Master
 *Software License Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 *This software is distributed to you in Source Code
 *format and is governed by the sections of the MSLA
 *applicable to Source Code.
 *
 ******************************************************************************/
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "sl_wfx.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif
uint8_t sl_wfx_host_get_waited_event(void);
sl_status_t wfx_soft_init(void);

#ifdef SLEEP_ENABLED
sl_status_t sl_wfx_host_switch_to_wirq(void);
#endif
#ifdef __cplusplus
}
#endif

#define SL_WFX_MAX_STATIONS 8
#define SL_WFX_MAX_SCAN_RESULTS 50

typedef struct __attribute__((__packed__)) scan_result_list_s
{
    sl_wfx_ssid_def_t ssid_def;
    uint8_t mac[SL_WFX_MAC_ADDR_SIZE];
    uint16_t channel;
    sl_wfx_security_mode_bitmask_t security_mode;
    uint16_t rcpi;
} scan_result_list_t;

void sl_wfx_host_start_platform_interrupt(void);
extern SemaphoreHandle_t wfx_wakeup_sem;
