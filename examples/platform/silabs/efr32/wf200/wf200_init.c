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

/* Includes */

#include "em_gpio.h"

#include "sl_wfx.h"
#include "sl_wfx_board.h"

// File specific to each platform, it must be created for custom boards
#include "sl_wfx_pds.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Firmware include */
#include "sl_wfx_wf200_C0.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "AppConfig.h"
#include "sl_wfx_host.h"
#include "sl_wfx_task.h"
#include "wfx_host_events.h"

#include "sl_spidrv_instances.h"
#include "spidrv.h"

#define SL_WFX_EVENT_MAX_SIZE 512
#define SL_WFX_EVENT_LIST_SIZE 1

StaticSemaphore_t xWfxWakeupSemaBuffer;
uint8_t sWfxEventQueueBuffer[SL_WFX_EVENT_LIST_SIZE * sizeof(uint8_t)];
StaticQueue_t sWfxEventQueueStruct;
QueueHandle_t wfx_event_Q        = NULL;
SemaphoreHandle_t wfx_wakeup_sem = NULL;
SemaphoreHandle_t wfx_mutex      = NULL;

StaticSemaphore_t xWfxMutexBuffer;

struct
{
    uint32_t wf200_firmware_download_progress;
    int wf200_initialized;
    uint8_t waited_event_id;
    uint8_t posted_event_id;
} host_context;

#ifdef SL_WFX_USE_SDIO
#ifdef SLEEP_ENABLED
sl_status_t sl_wfx_host_enable_sdio(void);
sl_status_t sl_wfx_host_disable_sdio(void);
#endif
#endif

#ifdef SL_WFX_USE_SPI
#ifdef SLEEP_ENABLED
sl_status_t sl_wfx_host_enable_spi(void);
sl_status_t sl_wfx_host_disable_spi(void);
#endif
#endif

/****************************************************************************
 * @fn  sl_status_t wfx_soft_init(void)
 * @brief
 * WFX FMAC driver host interface initialization
 * @param[in] None
 * @returns Returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t wfx_soft_init(void)
{
    SILABS_LOG("WF200:Soft Init");
    if ((wfx_event_Q = xQueueCreateStatic(SL_WFX_EVENT_LIST_SIZE, sizeof(uint8_t), sWfxEventQueueBuffer, &sWfxEventQueueStruct)) ==
        NULL)
    {
        return SL_STATUS_FAIL;
    }

    if ((wfx_wakeup_sem = xSemaphoreCreateBinaryStatic(&xWfxWakeupSemaBuffer)) == NULL)
    {
        return SL_STATUS_FAIL;
    }

    if ((wfx_mutex = xSemaphoreCreateMutexStatic(&xWfxMutexBuffer)) == NULL)
    {
        return SL_STATUS_FAIL;
    }

    return SL_STATUS_OK;
}
/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_init(void)
 * @brief
 * Notify driver init function
 * @param[in] None
 * @returns Returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_init(void)
{
    SILABS_LOG("WFX: Host Init");
    host_context.wf200_firmware_download_progress = 0;
    host_context.wf200_initialized                = 0;
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_get_firmware_data(const uint8_t **data, uint32_t data_size)
 * @brief
 * Get firmware data
 * @param[in] data:
 * @param[in] data_size:
 * @returns Returns SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_get_firmware_data(const uint8_t ** data, uint32_t data_size)
{
    *data = &sl_wfx_firmware[host_context.wf200_firmware_download_progress];
    host_context.wf200_firmware_download_progress += data_size;
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_get_firmware_size(uint32_t *firmware_size)
 * @brief
 * Get firmware size
 * @param[in] firmware_size:
 * @returns Returns SL_STATUS_OK if successful,
 *         SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_get_firmware_size(uint32_t * firmware_size)
{
    *firmware_size = sizeof(sl_wfx_firmware);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_get_pds_data(const char **pds_data, uint16_t index)
 * @brief
 * Get PDS data
 * @param[in] pds_data:
 * @param[in] index:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_get_pds_data(const char ** pds_data, uint16_t index)
{
    *pds_data = sl_wfx_pds[index];
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_get_pds_size(uint16_t *pds_size)
 * @brief
 * Get PDS size
 * @param[in] pds_size:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_get_pds_size(uint16_t * pds_size)
{
    *pds_size = SL_WFX_ARRAY_COUNT(sl_wfx_pds);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_deinit(void)
 * @brief
 * Deinit host interface
 * @param[in] None
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_deinit(void)
{
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_allocate_buffer(void **buffer, sl_wfx_buffer_type_t type, uint32_t buffer_size)
 * @brief
 * Allocate buffer (Should allocate either Ethernet - from LWIP or Control) - TODO
 * @param[in] buffer:
 * @param[in] type:
 * @param[in] buffer_size:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_allocate_buffer(void ** buffer, sl_wfx_buffer_type_t type, uint32_t buffer_size)
{
    if ((*buffer = pvPortMalloc(buffer_size)) == (void *) 0)
    {
        return SL_STATUS_FAIL;
    }
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_free_buffer(void *buffer, sl_wfx_buffer_type_t type)
 * @brief
 * Free host buffer (CHECK LWIP buffer)
 * @param[in] buffer:
 * @param[in] type:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_free_buffer(void * buffer, sl_wfx_buffer_type_t type)
{
    vPortFree(buffer);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_hold_in_reset(void)
 * @brief
 * Set reset pin low
 * @param[in] None
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_hold_in_reset(void)
{
    GPIO_PinOutClear(SL_WFX_HOST_PINOUT_RESET_PORT, SL_WFX_HOST_PINOUT_RESET_PIN);
    host_context.wf200_initialized = 0;
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_set_wake_up_pin(uint8_t state)
 * @brief
 * Set wakeup pin status
 * @param[in] state:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_set_wake_up_pin(uint8_t state)
{
    CORE_DECLARE_IRQ_STATE;

    CORE_ENTER_ATOMIC();
    if (state > PINOUT_CLEAR_STATUS)
    {
#ifdef SLEEP_ENABLED
#ifdef SL_WFX_USE_SDIO
        sl_wfx_host_enable_sdio();
#endif
#ifdef SL_WFX_USE_SPI
        sl_wfx_host_enable_spi();
#endif
#endif
        GPIO_PinOutSet(SL_WFX_HOST_PINOUT_WUP_PORT, SL_WFX_HOST_PINOUT_WUP_PIN);
    }
    else
    {
        GPIO_PinOutClear(SL_WFX_HOST_PINOUT_WUP_PORT, SL_WFX_HOST_PINOUT_WUP_PIN);
#ifdef SLEEP_ENABLED
#ifdef SL_WFX_USE_SDIO
        sl_wfx_host_disable_sdio();
#endif
#ifdef SL_WFX_USE_SPI
        sl_wfx_host_disable_spi();
#endif
#endif
    }
    CORE_EXIT_ATOMIC();
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_reset_chip(void)
 * @brief
 * reset the host chip
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_reset_chip(void)
{
    // Pull it low for at least 1 ms to issue a reset sequence
    GPIO_PinOutClear(SL_WFX_HOST_PINOUT_RESET_PORT, SL_WFX_HOST_PINOUT_RESET_PIN);

    // Delay for 10ms
    vTaskDelay(pdMS_TO_TICKS(10));

    // Hold pin high to get chip out of reset
    GPIO_PinOutSet(SL_WFX_HOST_PINOUT_RESET_PORT, SL_WFX_HOST_PINOUT_RESET_PIN);

    // Delay for 3ms
    vTaskDelay(pdMS_TO_TICKS(3));

    host_context.wf200_initialized = 0;
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_wait_for_wake_up(void)
 * @brief
 * wait for the host wake up
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_wait_for_wake_up(void)
{
    xSemaphoreTake(wfx_wakeup_sem, pdMS_TO_TICKS(TICKS_TO_WAIT_0));
    xSemaphoreTake(wfx_wakeup_sem, pdMS_TO_TICKS(TICKS_TO_WAIT_3));

    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_wait(uint32_t wait_time)
 * @brief
 * wait for the host
 * @param[in]  wait_time:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/

sl_status_t sl_wfx_host_wait(uint32_t wait_time)
{
    uint32_t ticks = pdMS_TO_TICKS(wait_time);
    vTaskDelay(ticks ? ticks : 10);
    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn   sl_status_t sl_wfx_host_setup_waited_event(uint8_t event_id)
 * @brief
 * Called when the driver needs to setup the waited event
 * @param[in] event_id:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/

sl_status_t sl_wfx_host_setup_waited_event(uint8_t event_id)
{
    host_context.waited_event_id = event_id;
    host_context.posted_event_id = 0;

    return SL_STATUS_OK;
}

/****************************************************************************
 * @fn  uint8_t sl_wfx_host_get_waited_event(void)
 * @brief
 * Called when the driver get waited event
 * @returns returns host_context.waited_event_id
 *****************************************************************************/

uint8_t sl_wfx_host_get_waited_event(void)
{
    return host_context.waited_event_id;
}

/******************************************************************************
 * @fn  sl_status_t sl_wfx_host_wait_for_confirmation(uint8_t confirmation_id, uint32_t timeout, void **event_payload_out)
 * @brief
 * wait for the host confirmation
 * @param[in] confirmation_id:
 * @param[in] timeout:
 * @param[in] event_payload_out:
 * @returns Returns SL_STATUS_OK if successful,
 * Timeout, SL_STATUS_TIMEOUT otherwise
 *****************************************************************************/

sl_status_t sl_wfx_host_wait_for_confirmation(uint8_t confirmation_id, uint32_t timeout, void ** event_payload_out)
{
    uint8_t posted_event_id;
    for (uint32_t i = 0; i < timeout; i++)
    {
        /* Wait for an event posted by the function sl_wfx_host_post_event() */
        if (xQueueReceive(wfx_event_Q, &posted_event_id, TICKS_TO_WAIT_1) == pdTRUE)
        {
            /* Once a message is received, check if it is the expected ID */
            if (confirmation_id == posted_event_id)
            {
                /* Pass the confirmation reply and return*/
                if (event_payload_out != NULL)
                {
                    *event_payload_out = sl_wfx_context->event_payload_buffer;
                }
                return SL_STATUS_OK;
            }
        }
    }
    /* The wait for the confirmation timed out, return */
    return SL_STATUS_TIMEOUT;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_lock(void)
 * @brief
 * Called when the driver needs to lock its access
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_TIMEOUT otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_lock(void)
{

    sl_status_t status = SL_STATUS_OK;

    if (xSemaphoreTake(wfx_mutex, pdMS_TO_TICKS(TICKS_TO_WAIT_500)) != pdTRUE)
    {
        SILABS_LOG("*ERR*Wi-Fi driver mutex timo");
        status = SL_STATUS_TIMEOUT;
    }

    return status;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_unlock(void)
 * @brief
 * Called when the driver needs to unlock its access
 * @returns Returns SL_STATUS_OK
 *****************************************************************************/
sl_status_t sl_wfx_host_unlock(void)
{
    xSemaphoreGive(wfx_mutex);

    return SL_STATUS_OK;
}

/******************************************************************************
 * @fn  sl_status_t sl_wfx_host_post_event(sl_wfx_generic_message_t *event_payload)
 * @brief
 * Called when the driver needs to post an event
 * @param[in]  event_payload:
 * @returns Returns status
 *****************************************************************************/
sl_status_t sl_wfx_host_post_event(sl_wfx_generic_message_t * event_payload)
{
    sl_status_t status;

    /* Forward the message to the application */
    status = sl_wfx_host_process_event(event_payload);

    if (host_context.waited_event_id == event_payload->header.id)
    {
        if (event_payload->header.length < SL_WFX_EVENT_MAX_SIZE)
        {
            /* Post the event in the queue */
            memcpy(sl_wfx_context->event_payload_buffer, (void *) event_payload, event_payload->header.length);
            host_context.posted_event_id = event_payload->header.id;
            xQueueOverwrite(wfx_event_Q, (void *) &event_payload->header.id);
        }
    }

    return status;
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_transmit_frame(void *frame, uint32_t frame_len)
 * @brief
 * Called when the driver needs to transmit a frame
 * @param[in] frame:
 * @param[in] frame_len:
 * @returns returns sl_wfx_data_write(frame, frame_len)
 *****************************************************************************/
sl_status_t sl_wfx_host_transmit_frame(void * frame, uint32_t frame_len)
{
    return sl_wfx_data_write(frame, frame_len);
}

/****************************************************************************
 * @fn  sl_status_t sl_wfx_host_sleep_grant(sl_wfx_host_bus_transfer_type_t type,
                                    sl_wfx_register_address_t address,
                                    uint32_t length)
 * @brief
 * Called when the driver is considering putting the
 * WFx in sleep mode
 * @param[in] type:
 * @param[in] address:
 * @param[in] length:
 * @returns SL_WIFI_SLEEP_GRANTED to let the WFx go to
 *sleep, SL_WIFI_SLEEP_NOT_GRANTED otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_sleep_grant(sl_wfx_host_bus_transfer_type_t type, sl_wfx_register_address_t address, uint32_t length)
{
    (void) (type);
    (void) (address);
    (void) (length);

    return SL_STATUS_WIFI_SLEEP_GRANTED;
}

#if SL_WFX_DEBUG_MASK
/****************************************************************************
 * @fn  void sl_wfx_host_log(const char *str, ...)
 * @brief
 * Host debug output
 * @param[in] str: string
 * @return None
 *****************************************************************************/
void sl_wfx_host_log(const char * str, ...)
{
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
}
#endif
#ifndef PW_RPC_ENABLED
/* Place holder - This is just to handle UART interrupts
 * The "otThread tasks handles it. WiFi does not need it yet
 * I don't care for it. I should really have the thread
 * shut it off
 */
#if !CHIP_ENABLE_OPENTHREAD
/****************************************************************************
 * @fn  void otSysEventSignalPending(void)
 * @brief
 * system event signal pending
 * @param[in] None
 * @return None
 *****************************************************************************/
void otSysEventSignalPending(void)
{
    // BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
    SILABS_LOG("*ERR*UART intr - NOT Handled");
    portYIELD_FROM_ISR(pdFALSE);
}
#endif /* !CHIP_ENABLE_OPENTHREAD */
#endif /* PW_RPC_ENABLED */
