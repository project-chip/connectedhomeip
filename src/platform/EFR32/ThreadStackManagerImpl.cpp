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
 *          EFR32 platforms using the Silicon Labs SDK and the OpenThread
 *          stack.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.cpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <openthread/platform/entropy.h>

#include <lib/support/CHIPPlatformMemory.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(otInst);
    SuccessOrExit(err);

exit:
    return err;
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return sInstance.mThreadStackLock != NULL;
}

} // namespace DeviceLayer
} // namespace chip

using namespace ::chip::DeviceLayer;

/**
 * Glue function called directly by the OpenThread stack when tasklet processing work
 * is pending.
 */
extern "C" void otTaskletsSignalPending(otInstance * p_instance)
{
    ThreadStackMgrImpl().SignalThreadActivityPending();
}

/**
 * Glue function called directly by the OpenThread stack when system event processing work
 * is pending.
 */
extern "C" void otSysEventSignalPending(void)
{
    BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
    portYIELD_FROM_ISR(yieldRequired);
}

extern "C" void * otPlatCAlloc(size_t aNum, size_t aSize)
{
    return CHIPPlatformMemoryCalloc(aNum, aSize);
}

extern "C" void otPlatFree(void * aPtr)
{
    CHIPPlatformMemoryFree(aPtr);
}

extern "C" __WEAK void sl_openthread_init(void)
{
    // Place holder for enabling Silabs specific features available only through Simplicity Studio
}

/**
 * @brief Openthread UART implementation for the CLI is conflicting
 *        with the UART implemented for Pigweed RPC as they use the same UART port
 *
 *        We now only build the uart as implemented in
 *        connectedhomeip/examples/platform/efr32/uart.c
 *        and remap OT functions to use our uart api.
 *
 *        For now OT CLI isn't usable when the examples are built with pw_rpc
 */

#ifndef PW_RPC_ENABLED
#include "uart.h"
#endif

extern "C" otError otPlatUartEnable(void)
{
#ifdef PW_RPC_ENABLED
    return OT_ERROR_NOT_IMPLEMENTED;
#else
    uartConsoleInit();
    return OT_ERROR_NONE;
#endif
}

#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI

extern "C" otError otPlatUartSend(const uint8_t * aBuf, uint16_t aBufLength)
{
#ifdef PW_RPC_ENABLED
    return OT_ERROR_NOT_IMPLEMENTED;
#else
    if (uartConsoleWrite((const char *) aBuf, aBufLength) > 0)
    {
        otPlatUartSendDone();
        return OT_ERROR_NONE;
    }
    return OT_ERROR_FAILED;
#endif
}

extern "C" void efr32UartProcess(void)
{
#if !defined(PW_RPC_ENABLED) && !defined(ENABLE_CHIP_SHELL)
    uint8_t tempBuf[128] = { 0 };
    // will read the data available up to 128bytes
    uint16_t count = uartConsoleRead((char *) tempBuf, 128);
    if (count > 0)
    {
        // ot process Received data for CLI cmds
        otPlatUartReceived(tempBuf, count);
    }
#endif
}

extern "C" __WEAK otError otPlatUartFlush(void)
{
    return OT_ERROR_NOT_IMPLEMENTED;
}

extern "C" __WEAK otError otPlatUartDisable(void)
{
    return OT_ERROR_NOT_IMPLEMENTED;
}

#endif // CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
