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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_gpio.h"

#include "sl_wfx.h"
#include "sl_wfx_board.h"
#include "sl_wfx_host.h"
#include "sl_wfx_task.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "AppConfig.h"

#define CHECK_VAL 0
#define WFX_BUS_TASK_PRIORITY 2
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
 * @fn  bool wfx_bus_is_receive_processing(void)
 * @brief
 * Check receive frame status
 * @param[in]  None
 * @return returns wfx_bus_rx_in_process
 ******************************************************************************/
bool wfx_bus_is_receive_processing(void)
{
    return wfx_bus_rx_in_process;
}

/*****************************************************************************
 * @fn  static sl_status_t receive_frames()
 * @brief
 * Receives frames from the WFX.
 * @param[in] None
 * @return returns result
 ******************************************************************************/
static sl_status_t receive_frames()
{
    sl_status_t result;
    uint16_t control_register = 0;
    wfx_bus_rx_in_process     = true;
    do
    {
        result = sl_wfx_receive_frame(&control_register);
        SL_WFX_ERROR_CHECK(result);
    } while ((control_register & SL_WFX_CONT_NEXT_LEN_MASK) != CHECK_VAL);

error_handler:
    wfx_bus_rx_in_process = false;
    return result;
}

/********************************************************************************
 * @fn  static void wfx_bus_task(void *p_arg)
 * @brief
 * WFX bus communication task.
 * receives frames from the Bus interface
 * @param[in] p_arg:
 * @return None
 */
static void wfx_bus_task(void * p_arg)
{
    SILABS_LOG("SPI: Bus Task started");
    sl_wfx_host_start_platform_interrupt();
    for (;;)
    {
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

sl_status_t wfx_bus_start(void)
{
    wfx_bus_task_handle =
        xTaskCreateStatic(wfx_bus_task, "wfxbus", BUS_TASK_STACK_SIZE, NULL, WFX_BUS_TASK_PRIORITY, busStack, &busTaskStruct);
    if (wfx_bus_task_handle == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }
    return SL_STATUS_OK;
}
