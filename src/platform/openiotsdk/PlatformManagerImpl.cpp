/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides the implementation of the Device Layer Platform Manager class
 *          for Open IOT SDK platform.
 */

#include "platform/internal/CHIPDeviceLayerInternal.h"
#include <platform/internal/testing/ConfigUnitTest.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.ipp>
#include <platform/openiotsdk/DiagnosticDataProviderImpl.h>
#include <platform/openiotsdk/SystemTimeSupport.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    // Members are initialized by the stack

    // ToDo initalize platform-specific members of stack

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    auto err = System::Clock::InitClock_RealTime();
    SuccessOrExit(err);

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

void PlatformManagerImpl::_LockChipStack()
{
    // ToDo
}

bool PlatformManagerImpl::_TryLockChipStack()
{
    // ToDo
    return true;
}

void PlatformManagerImpl::_UnlockChipStack()
{
    // ToDo
}

CHIP_ERROR PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * eventPtr)
{
    // ToDo

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::ProcessDeviceEvents()
{
    // ToDo
}

void PlatformManagerImpl::_RunEventLoop()
{
    // ToDo
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    // ToDo

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    // ToDo

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(System::Clock::Timeout duration)
{
    // Let LayerSocketsLoop::PrepareSelect() handle timers.
    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown()
{
    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip
