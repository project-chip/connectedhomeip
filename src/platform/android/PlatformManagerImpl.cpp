/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for Android platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <platform/android/DeviceInstanceInfoProviderImpl.h>
#include <platform/android/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.ipp>

#include <thread>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::AndroidConfig::Init();
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    // Now set up our device instance info provider.  We couldn't do that
    // earlier, because the generic implementation sets a generic one.
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
