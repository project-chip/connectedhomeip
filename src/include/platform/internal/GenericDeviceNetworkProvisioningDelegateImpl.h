/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
    CHIP_ERROR ProvisionWiFi(const char * ssid, const char * passwd) override;
    CHIP_ERROR ProvisionThread(ByteSpan threadData) override;

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline CHIP_ERROR GenericDeviceNetworkProvisioningDelegateImpl<ImplClass>::ProvisionWiFi(const char * ssid, const char * passwd)
{
    return Impl()->_ProvisionWiFiNetwork(ssid, passwd);
}

template <class ImplClass>
inline CHIP_ERROR GenericDeviceNetworkProvisioningDelegateImpl<ImplClass>::ProvisionThread(ByteSpan threadData)
{
    return Impl()->_ProvisionThreadNetwork(threadData);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
