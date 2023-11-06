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

#ifdef SL_WFX_USE_SECURE_LINK
#include "secure_link/sl_wfx_secure_link.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Securelink Task Configurations
#define WFX_SECURELINK_TASK_PRIO 1u
#define WFX_SECURELINK_TASK_STK_SIZE 512u

TaskHandle_t secureLinkTaskHandle;
SemaphoreHandle_t s_xSLSemaphore;
StackType_t secureLinkStack[WFX_SECURELINK_TASK_STK_SIZE];
StaticTask_t secureLinkTaskStruct;

StaticSemaphore_t xSlMutexBuffer;

/*********************************************************************
 * @fn  static void prvSecureLinkTask(void *p_arg)
 * @brief
 * The task that implements the Secure Link renegotiation with WFX.
 * @param[in] p_arg:
 * @return None
 *************************************************************************/
static void prvSecureLinkTask(void * p_arg)
{
    sl_status_t result;
    (void) p_arg;

    /* Create a mutex used for making Secure Link renegotiations atomic */
    s_xSLSemaphore = xSemaphoreCreateMutexStatic(&xSlMutexBuffer);

    for (;;)
    {
        /* Wait for a key renegotiation request */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        result = sl_wfx_secure_link_renegotiate_session_key();
        if (result != SL_STATUS_OK)
        {
            printf("session key negotiation error %lu\n", result);
        }
    }
}

/****************************************************************************
 * @fn  void wfx_securelink_task_start(void)
 * @brief
 * Creates WFX securelink key renegotiation task.
 * @param[in] None
 * @return None
 ******************************************************************************/
void wfx_securelink_task_start(void)
{
    secureLinkTaskHandle = xTaskCreateStatic(prvSecureLinkTask, "secureLinkTask", WFX_SECURELINK_TASK_STK_SIZE, NULL,
                                             WFX_SECURELINK_TASK_PRIO, secureLinkStack, &secureLinkTaskStruct);
    if (secureLinkTaskHandle == NULL)
    {
        printf("Failed to create WFX secureLinkTask");
    }
}

#endif
