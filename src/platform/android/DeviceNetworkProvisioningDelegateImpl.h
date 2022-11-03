/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/internal/GenericDeviceNetworkProvisioningDelegateImpl.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

template <class ImplClass>
class GenericDeviceNetworkProvisioningDelegateImpl;

} // namespace Internal

class DeviceNetworkProvisioningDelegateImpl final
    : public Internal::GenericDeviceNetworkProvisioningDelegateImpl<DeviceNetworkProvisioningDelegateImpl>
{
    friend class GenericDeviceNetworkProvisioningDelegateImpl<DeviceNetworkProvisioningDelegateImpl>;

private:
    CHIP_ERROR _ProvisionWiFiNetwork(const char * ssid, const char * passwd);
    CHIP_ERROR _ProvisionThreadNetwork(ByteSpan threadData) { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

} // namespace DeviceLayer
} // namespace chip
