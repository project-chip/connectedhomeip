/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/logging/CHIPLogging.h>

#include "DeviceNetworkProvisioningDelegateImpl.h"
#include "NetworkCommissioningDriver.h"

using namespace ::chip::DeviceLayer::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    MutableCharSpan emptyBufferForDebugText;
    uint8_t outNetworkIndex;
    auto err = WiFiDriverImpl::GetInstance().AddOrUpdateNetwork(ByteSpan(Uint8::from_const_char(ssid), strlen(ssid)),
                                                                ByteSpan(Uint8::from_const_char(key), strlen(key)),
                                                                emptyBufferForDebugText, outNetworkIndex);
    if (err != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to add WiFi network: 0x%x", int(err));
        return CHIP_ERROR_INTERNAL;
    }
    NetworkCommissioning::WiFiDriverImpl::GetInstance().ConnectNetwork(ByteSpan(Uint8::from_const_char(ssid), strlen(ssid)),
                                                                       nullptr);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionThreadNetwork(ByteSpan threadData)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
