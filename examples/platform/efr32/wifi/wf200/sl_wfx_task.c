/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/***************************************************************************
 * @file
 * @brief WFX FMAC driver main bus communication task
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc.
 *www.silabs.com</b>
 *******************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in
 *compliance with the License. You may obtain a copy
 *of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in
 *writing, software distributed under the License is
 *distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *OR CONDITIONS OF ANY KIND, either express or
 *implied. See the License for the specific language
 *governing permissions and limitations under the
 *License.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_gpio.h"

#include "sl_wfx.h"
#include "sl_wfx_host.h"
#include "sl_wfx_board.h"
#include "sl_wfx_task.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "AppConfig.h"

#define BUS_TASK_STACK_SIZE 1024
static StackType_t busStack[BUS_TASK_STACK_SIZE];
StaticTask_t busTaskStruct;
TaskHandle_t wfx_bus_task_handle;

wfx_frame_q_item wfx_bus_tx_frame;
SemaphoreHandle_t wfxtask_tx_complete;
SemaphoreHandle_t wfxtask_mutex;

// Flag to indicate receive frames is currently running.
static bool wfx_bus_rx_in_process = false;

/***************************************************************************
 * Check receive frame status
 ******************************************************************************/
bool
wfx_bus_is_receive_processing(void)
{
    return wfx_bus_rx_in_process;
}

/*****************************************************************************
 * Receives frames from the WFX.
 ******************************************************************************/
static sl_status_t
receive_frames()
{
    sl_status_t result;
    uint16_t control_register = 0;
    wfx_bus_rx_in_process     = true;
    do {
        result = sl_wfx_receive_frame(&control_register);
        SL_WFX_ERROR_CHECK(result);
    } while ((control_register & SL_WFX_CONT_NEXT_LEN_MASK) != 0);

error_handler:
    wfx_bus_rx_in_process = false;
    return result;
}

/*
 * WFX bus communication task.
 * receives frames from the Bus interface
 */
static void
wfx_bus_task(void * p_arg)
{
    sl_wfx_host_start_platform_interrupt();
    EFR32_LOG ("SPI: Bus Task started");
    for (;;)  {
        /*Wait for an interrupt from WFX*/
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /*Disable the interrupt while treating frames received to avoid
         *the case where the interrupt is set but there is no frame left to treat.*/
        sl_wfx_host_disable_platform_interrupt();

        /*Receive the frame(s) pending in WFX*/
        receive_frames();

        /*Re-enable the interrupt*/
        sl_wfx_host_enable_platform_interrupt();
    }
}

/***************************************************************************
 * Creates WFX bus communication task.
 ******************************************************************************/
void
wfx_bus_start()
{
    wfx_bus_task_handle = xTaskCreateStatic(wfx_bus_task, "wfxbus",
                                            BUS_TASK_STACK_SIZE, NULL, 2,
                                            busStack, &busTaskStruct);
    if (wfx_bus_task_handle == NULL)
    {
        EFR32_LOG("*ERR*WFX BusTask");
    }
}
