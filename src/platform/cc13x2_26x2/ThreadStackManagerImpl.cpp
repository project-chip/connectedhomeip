/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          CC13X2_26X2 platforms using the Texas Instruments SDK and the
 *          OpenThread stack.
 *
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.cpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp>

#include <mbedtls/platform.h>
#include <openthread/heap.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

// platform folder in the TI SDK example application
#include <platform/system.h>

// DMM Includes
#ifdef USE_DMM
#include "ti_dmm_application_policy.h"
#include <dmm/dmm_policy.h>
#include <dmm/dmm_priority_ble_thread.h>
#include <dmm/dmm_scheduler.h>
#endif

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE != 0
static void * ot_calloc(size_t n, size_t size)
{
    void * p_ptr = NULL;

    p_ptr = pvPortMalloc(n * size);

    memset(p_ptr, 0, n * size);

    return p_ptr;
}

static void ot_free(void * p_ptr)
{
    vPortFree(p_ptr);
}
#endif /* OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE != 0 */

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Create FreeRTOS queue for platform driver messages
    procQueue = xQueueCreate(16U, sizeof(ThreadStackManagerImpl::procQueueMsg));

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE != 0
    mbedtls_platform_set_calloc_free(ot_calloc, ot_free);
#endif /* OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE != 0 */

    // Initialize the OpenThread platform layer
    otSysInit(0, NULL);

#ifdef USE_DMM
    // DMM Init
    DMMSch_registerClient(xTaskGetCurrentTaskHandle(), DMMPolicy_StackRole_threadFtd);
    DMMPolicy_updateStackState(DMMPolicy_StackRole_threadFtd, DMMPOLICY_THREAD_IDLE);
#endif

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
    err = GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>::DoInit(otInst);
    SuccessOrExit(err);

exit:
    return err;
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return sInstance.mThreadStackLock != NULL;
}

void ThreadStackManagerImpl::_SendProcMessage(ThreadStackManagerImpl::procQueueMsg & procMsg)
{
    xQueueSendFromISR(procQueue, &procMsg, NULL);

    // signal processing loop
    SignalThreadActivityPendingFromISR();
}

void ThreadStackManagerImpl::_ProcMessage(otInstance * aInstance)
{
    procQueueMsg procMsg;
    while (pdTRUE == xQueueReceive(procQueue, &procMsg, 0U))
    {
        switch (procMsg.cmd)
        {
        case procQueueCmd_alarm: {
            platformAlarmProcess(aInstance);
            break;
        }

        case procQueueCmd_radio: {
            platformRadioProcess(aInstance, procMsg.arg);
            break;
        }

        case procQueueCmd_tasklets: {
            otTaskletsProcess(aInstance);
            break;
        }

        case procQueueCmd_random: {
            platformRandomProcess();
            break;
        }

        case procQueueCmd_alarmu: {
            platformAlarmMicroProcess(aInstance);
            break;
        }

        default: {
            break;
        }
        }
    }
}

void ThreadStackManagerImpl::GetExtAddress(otExtAddress & aExtAddr)
{
    const otExtAddress * extAddr;
    LockThreadStack();
    extAddr = otLinkGetExtendedAddress(OTInstance());
    UnlockThreadStack();

    memcpy(aExtAddr.m8, extAddr->m8, OT_EXT_ADDRESS_SIZE);
}

} // namespace DeviceLayer
} // namespace chip

using namespace ::chip::DeviceLayer;

/**
 * Glue function called by alarm processing layer to notify OpenThread the driver needs processing.
 */
extern "C" void platformAlarmSignal()
{
    ThreadStackManagerImpl::procQueueMsg msg;
    msg.cmd = ThreadStackManagerImpl::procQueueCmd_alarm;
    ThreadStackMgrImpl()._SendProcMessage(msg);
}

/**
 * Glue function called by alarm processing layer to notify OpenThread the driver needs processing.
 */
extern "C" void platformAlarmMicroSignal()
{
    ThreadStackManagerImpl::procQueueMsg msg;
    msg.cmd = ThreadStackManagerImpl::procQueueCmd_alarmu;
    ThreadStackMgrImpl()._SendProcMessage(msg);
}

/**
 * Glue function called by radio processing layer to notify OpenThread the driver needs processing.
 */
extern "C" void platformRadioSignal(uintptr_t arg)
{
    ThreadStackManagerImpl::procQueueMsg msg;
    msg.cmd = ThreadStackManagerImpl::procQueueCmd_radio;
    msg.arg = arg;
    ThreadStackMgrImpl()._SendProcMessage(msg);
}

/**
 * Glue function called directly by the OpenThread stack when tasklet processing work
 * is pending.
 */
extern "C" void otTaskletsSignalPending(otInstance * p_instance)
{
    ThreadStackMgrImpl().SignalThreadActivityPending();
}

/**
 * Process events from the drivers
 */
extern "C" void otSysProcessDrivers(otInstance * aInstance)
{
    ThreadStackMgrImpl()._ProcMessage(aInstance);
}

/**
 * Get a pointer to the OpenThread instance object.
 *
 * @return Pointer to the OpenThread instance object.
 */
extern "C" otInstance * OtInstance_get(void)
{
    return ThreadStackMgrImpl().OTInstance();
}
