/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    CHIP_ERROR _ProvisionWiFiNetwork(const char * ssid, const char * passwd);
#else
    CHIP_ERROR _ProvisionWiFiNetwork(const char * ssid, const char * passwd) { return CHIP_ERROR_NOT_IMPLEMENTED; }
#endif
    CHIP_ERROR _ProvisionThreadNetwork(ByteSpan threadData);
};

} // namespace DeviceLayer
} // namespace chip
