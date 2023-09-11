/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the network commissioning drivers
 *          for Open IOT SDK platform.
 */

#pragma once

#include <inet/IPAddress.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

class OpenIoTSDKEthernetDriver final : public EthernetDriver
{
public:
    struct EthernetNetworkIterator final : public NetworkIterator
    {
        EthernetNetworkIterator() = default;
        size_t Count() override { return interfaceNameLen > 0 ? 1 : 0; }
        bool Next(Network & item) override
        {
            if (exhausted)
            {
                return false;
            }
            exhausted = true;
            memcpy(item.networkID, interfaceName, interfaceNameLen);
            item.networkIDLen = interfaceNameLen;
            item.connected    = true;
            return true;
        }
        void Release() override { delete this; }
        ~EthernetNetworkIterator() override = default;

        // Public, but cannot be accessed via NetworkIterator interface.
        uint8_t interfaceName[kMaxNetworkIDLen];
        uint8_t interfaceNameLen = 0;
        bool exhausted           = false;
    };

    uint8_t GetMaxNetworks() override { return 1; };
    NetworkIterator * GetNetworks() override;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
