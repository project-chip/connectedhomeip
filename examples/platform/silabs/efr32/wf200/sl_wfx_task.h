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

#include <stdbool.h>

#include "FreeRTOS.h"
#include "sl_wfx_constants.h"
#include "task.h"

typedef struct
{
    sl_wfx_send_frame_req_t * frame;
    uint32_t data_length;
    sl_wfx_interface_t interface;
    uint8_t priority;
} wfx_frame_q_item;

extern wfx_frame_q_item wfxtask_tx_frame;
extern TaskHandle_t wfx_bus_task_handle;

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * @fn void wfx_bus_start(void)
 * @brief
 * Start wfx bus communication task.
 *****************************************************************************/
void wfx_bus_start(void);

/****************************************************************************
 * @fn  bool wfx_bus_is_receive_processing(void)
 * @brief
 * Returns status of wfx receive frames.
 *****************************************************************************/
bool wfx_bus_is_receive_processing(void);

#ifdef __cplusplus
}
#endif
