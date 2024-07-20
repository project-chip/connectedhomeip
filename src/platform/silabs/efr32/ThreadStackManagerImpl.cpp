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

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/NetworkCommissioning.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <openthread/platform/entropy.h>

#include <lib/support/CHIPPlatformMemory.h>

#include <lib/support/CodeUtils.h>
#include <mbedtls/platform.h>

extern "C" {
#include "platform-efr32.h"
otInstance * otGetInstance(void);
#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
void otAppCliInit(otInstance * aInstance);
#endif // CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
}

namespace chip {
namespace DeviceLayer {
namespace {
otInstance * sOTInstance = NULL;

// Network commissioning
#ifndef _NO_GENERIC_THREAD_NETWORK_COMMISSIONING_DRIVER_
NetworkCommissioning::GenericThreadDriver sGenericThreadDriver;
app::Clusters::NetworkCommissioning::Instance sThreadNetworkCommissioningInstance(0 /* Endpoint Id */, &sGenericThreadDriver);
#endif

void initStaticNetworkCommissioningThreadDriver(void)
{
#ifndef _NO_GENERIC_THREAD_NETWORK_COMMISSIONING_DRIVER_
    sThreadNetworkCommissioningInstance.Init();
#endif
}

void shutdownStaticNetworkCommissioningThreadDriver(void)
{
#ifndef _NO_GENERIC_THREAD_NETWORK_COMMISSIONING_DRIVER_
    sThreadNetworkCommissioningInstance.Shutdown();
#endif
}
}; // namespace

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(sOTInstance);
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask(void)
{
    // Stubbed since our thread task is created in the InitThreadStack function and it will start once the scheduler starts.
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_LockThreadStack(void)
{
    sl_ot_rtos_acquire_stack_mutex();
}

bool ThreadStackManagerImpl::_TryLockThreadStack(void)
{
    // TODO: Implement a non-blocking version of the mutex lock
    sl_ot_rtos_acquire_stack_mutex();
    return true;
}

void ThreadStackManagerImpl::_UnlockThreadStack(void)
{
    sl_ot_rtos_release_stack_mutex();
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
void ThreadStackManagerImpl::_WaitOnSrpClearAllComplete()
{
    // Only 1 task can be blocked on a srpClearAll request
    if (mSrpClearAllRequester == NULL)
    {
        mSrpClearAllRequester = osThreadGetId();
        // Wait on OnSrpClientNotification which confirms the clearing is done.
        // It will notify this current task with NotifySrpClearAllComplete.
        // However, we won't wait more than 2s.
        osThreadFlagsWait(threadSrpClearAllFlags, osFlagsWaitAny, pdMS_TO_TICKS(2000));
        mSrpClearAllRequester = NULL;
    }
}

void ThreadStackManagerImpl::_NotifySrpClearAllComplete()
{
    if (mSrpClearAllRequester)
    {
        osThreadFlagsSet(mSrpClearAllRequester, threadSrpClearAllFlags);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::ConfigureThreadStack(otInst);
    initStaticNetworkCommissioningThreadDriver();
    return err;
}

void ThreadStackManagerImpl::FactoryResetThreadStack(void)
{
    VerifyOrReturn(sOTInstance != NULL);
    otInstanceFactoryReset(sOTInstance);
    shutdownStaticNetworkCommissioningThreadDriver();
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return otGetInstance() != NULL;
}

} // namespace DeviceLayer
} // namespace chip

using namespace ::chip::DeviceLayer;

#ifndef SL_COMPONENT_CATALOG_PRESENT
extern "C" __WEAK void sl_openthread_init(void)
{
#error "This shouldn't compile"
    // Place holder for enabling Silabs specific features available only through Simplicity Studio
}
#endif

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
    // Uart Init is handled in init_efrPlatform.cpp
    return OT_ERROR_NONE;
#endif
}

extern "C" otInstance * otGetInstance(void)
{
    return sOTInstance;
}

extern "C" void sl_ot_create_instance(void)
{
    VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);
    sOTInstance = otInstanceInitSingle();
}

extern "C" void sl_ot_cli_init(void)
{
#if !defined(PW_RPC_ENABLED) && CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
    VerifyOrDie(sOTInstance != NULL);
    otAppCliInit(sOTInstance);
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
    int16_t count = uartConsoleRead((char *) tempBuf, 128);
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
