/*
 *
 *    Copyright (c) 2019 Google LLC.
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


#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>

#if WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <Weave/Profiles/WeaveProfiles.h>
#include <Weave/Profiles/common/CommonProfile.h>

#include <Weave/DeviceLayer/internal/GenericSoftwareUpdateManagerImpl_BDX.ipp>
#include <Weave/DeviceLayer/internal/GenericSoftwareUpdateManagerImpl.ipp>

namespace nl {
namespace Weave {
namespace DeviceLayer {

SoftwareUpdateManagerImpl SoftwareUpdateManagerImpl::sInstance;

WEAVE_ERROR SoftwareUpdateManagerImpl::_Init(void)
{
    Internal::GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>::DoInit();
    Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>::DoInit();

    return WEAVE_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER