/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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
