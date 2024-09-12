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

/***************************************************************************
 * @fn  void wfx_bus_start()
 * @brief
 * Creates WFX bus communication task.
 * @param[in] None
 * @return None
 ******************************************************************************/
void wfx_bus_start()
{
    wfx_bus_task_handle =
        xTaskCreateStatic(wfx_bus_task, "wfxbus", BUS_TASK_STACK_SIZE, NULL, WFX_BUS_TASK_PRIORITY, busStack, &busTaskStruct);
    if (wfx_bus_task_handle == NULL)
    {
        SILABS_LOG("*ERR*WFX BusTask");
    }
}
