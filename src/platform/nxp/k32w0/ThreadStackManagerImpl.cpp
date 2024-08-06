/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
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
 *          K32W platforms using the NXP SDK and the OpenThread
 *          stack.
 *
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.hpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>

#include <lib/support/CHIPPlatformMemory.h>

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
extern "C" bool isThreadInitialized();
#endif

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

void ThreadStackManagerImpl::ProcessThreadActivity()
{
    /* reuse thread task for ble processing.
     * by doing this, we avoid allocating a new stack for short-lived
     * BLE processing (e.g.: only during Matter commissioning)
     */
    auto * bleManager = &chip::DeviceLayer::Internal::BLEMgrImpl();
    bleManager->DoBleProcessing();

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
    if (isThreadInitialized())
#endif
    {
        otTaskletsProcess(OTInstance());
        otSysProcessDrivers(OTInstance());
    }
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

extern "C" void * pvPortCallocRtos(size_t num, size_t size)
{
    size_t totalAllocSize = (size_t) (num * size);

    if (size && totalAllocSize / size != num)
        return nullptr;

    void * p = pvPortMalloc(totalAllocSize);

    if (p)
    {
        memset(p, 0, totalAllocSize);
    }

    return p;
}

extern "C" void * otPlatCAlloc(size_t aNum, size_t aSize)
{
    return CHIPPlatformMemoryCalloc(aNum, aSize);
}

extern "C" void otPlatFree(void * aPtr)
{
    return CHIPPlatformMemoryFree(aPtr);
}

extern "C" void * otPlatRealloc(void * p, size_t aSize)
{
    return CHIPPlatformMemoryRealloc(p, aSize);
}
