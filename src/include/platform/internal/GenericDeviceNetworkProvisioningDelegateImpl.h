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

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DeviceNetworkProvisioning.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
class GenericDeviceNetworkProvisioningDelegateImpl : public DeviceNetworkProvisioningDelegate
{
public:
    void ProvisionWiFi(const char * ssid, const char * passwd) override;
    void ProvisionThread(DeviceLayer::Internal::DeviceNetworkInfo & threadData) override;

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline void GenericDeviceNetworkProvisioningDelegateImpl<ImplClass>::ProvisionWiFi(const char * ssid, const char * passwd)
{
    Impl()->_ProvisionWiFiNetwork(ssid, passwd);
}

template <class ImplClass>
inline void
GenericDeviceNetworkProvisioningDelegateImpl<ImplClass>::ProvisionThread(DeviceLayer::Internal::DeviceNetworkInfo & threadData)
{
    Impl()->_ProvisionThreadNetwork(threadData);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
