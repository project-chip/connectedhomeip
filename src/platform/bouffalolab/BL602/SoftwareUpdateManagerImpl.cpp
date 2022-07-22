/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/internal/GenericSoftwareUpdateManagerImpl.cpp>
#include <platform/internal/GenericSoftwareUpdateManagerImpl_BDX.cpp>

#include <platform/Profiles/CHIPProfiles.h>
#include <platform/Profiles/common/CommonProfile.h>

namespace chip {
namespace DeviceLayer {

SoftwareUpdateManagerImpl SoftwareUpdateManagerImpl::sInstance;

CHIP_ERROR SoftwareUpdateManagerImpl::_Init(void)
{
    Internal::GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>::DoInit();
    Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>::DoInit();

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
