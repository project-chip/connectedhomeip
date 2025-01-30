/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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

/**
 * @brief Start WFX bus communication task.
 *
 * @return sl_status_t SL_STATUS_OK, if the initialization succeeded
 *                     SL_STATUS_ALLOCATION_FAILED, if there are a memory allocation failure.
 */
sl_status_t wfx_bus_start(void);

/****************************************************************************
 * @fn  bool wfx_bus_is_receive_processing(void)
 * @brief
 * Returns status of wfx receive frames.
 *****************************************************************************/
bool wfx_bus_is_receive_processing(void);

#ifdef __cplusplus
}
#endif
