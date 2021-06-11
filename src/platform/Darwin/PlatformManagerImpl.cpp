/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object
 *          for Darwin platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
#include <platform/internal/GenericPlatformManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::PosixConfig::Init();
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    SystemLayer.SetDispatchQueue(GetWorkQueue());

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    // Call up to the base class _Shutdown() to perform the bulk of the shutdown.
    return System::MapErrorPOSIX(GenericPlatformManagerImpl<ImplClass>::_Shutdown());
}

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{
    const ChipDeviceEvent eventCopy = *event;
    dispatch_async(mWorkQueue, ^{
        Impl()->DispatchEvent(&eventCopy);
    });
}

} // namespace DeviceLayer
} // namespace chip
