/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object
 *          for Tizen platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <platform/Tizen/DeviceInstanceInfoProviderImpl.h>
#include <platform/Tizen/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.ipp>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    ReturnErrorOnFailure(Internal::PosixConfig::Init());

    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack());

    // Now set up our device instance info provider.  We couldn't do that
    // earlier, because the generic implementation sets a generic one.
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
