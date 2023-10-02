/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "Types.h"
#include "lib/support/logging/CHIPLogging.h"

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <string.h>
#include <vector>

namespace matter {
namespace casting {
namespace core {

enum ConnectionError
{
    NO_CONNECTION_ERROR = 0,
    FAILED_TO_CONNECT   = 1
};

using ConnectCallback    = std::function<void(ConnectionError)>;
using DisconnectCallback = std::function<void(void)>;

const int kPortMaxLength = 5; // port is uint16_t
// +1 for the : between the hostname and the port.
const int kIdMaxLength = chip::Dnssd::kHostNameMaxLength + kPortMaxLength + 1;

class CastingPlayerAttributes
{
public:
    char id[kIdMaxLength + 1]                              = {};
    char deviceName[chip::Dnssd::kMaxDeviceNameLen + 1]    = {};
    char hostName[chip::Dnssd::kHostNameMaxLength + 1]     = {};
    char instanceName[chip::Dnssd::kHostNameMaxLength + 1] = {};
    unsigned int numIPs; // number of valid IP addresses
    chip::Inet::IPAddress ipAddresses[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];
    uint16_t port;
    uint16_t productId;
    uint16_t vendorId;
    uint32_t deviceType;
};

/**
 * @brief CastingPlayer represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 */
class CastingPlayer : public std::enable_shared_from_this<CastingPlayer>
{
private:
    // std::vector<memory::Strong<Endpoint>> endpoints;
    bool mConnected = false;
    CastingPlayerAttributes mAttributes;

public:
    CastingPlayer(CastingPlayerAttributes playerAttributes) { mAttributes = playerAttributes; }
    const char * GetId() const { return mAttributes.id; }

    const char * GetDeviceName() const { return mAttributes.deviceName; }

    const char * GetHostName() const { return mAttributes.hostName; }

    const char * GetInstanceName() const { return mAttributes.instanceName; }

    uint GetNumIPs() const { return mAttributes.numIPs; }

    chip::Inet::IPAddress * GetIPAddresses() { return mAttributes.ipAddresses; }

    uint16_t GetPort() { return mAttributes.port; }

    uint16_t GetProductId() const { return mAttributes.productId; }

    uint16_t GetVendorId() const { return mAttributes.vendorId; }

    uint32_t GetDeviceType() const { return mAttributes.deviceType; }

    // public:
    // void RegisterEndpoint(const memory::Strong<Endpoint> endpoint) { endpoints.push_back(endpoint); }

    // const std::vector<memory::Strong<Endpoint>> GetEndpoints() const { return endpoints; }

    /**
     * @brief Compares based on the Id
     */
    bool operator==(const CastingPlayer & other) const
    {
        int compareResult = strcmp(this->mAttributes.id, other.mAttributes.id);
        return (compareResult == 0) ? 1 : 0;
    }

public:
    /**
     * @return true if this CastingPlayer is connected to the CastingApp
     */
    bool IsConnected() const { return mConnected; }

    void Connect(const long timeout, ConnectCallback onCompleted);
    void Disconnect(const long timeout, DisconnectCallback onCompleted);

    void LogDetail() const
    {
        if (strlen(mAttributes.id) != 0)
        {
            ChipLogDetail(Discovery, "\tID: %s", mAttributes.id);
        }
        if (strlen(mAttributes.deviceName) != 0)
        {
            ChipLogDetail(Discovery, "\tName: %s", mAttributes.deviceName);
        }
        if (strlen(mAttributes.hostName) != 0)
        {
            ChipLogDetail(Discovery, "\tHost Name: %s", mAttributes.hostName);
        }
        if (strlen(mAttributes.instanceName) != 0)
        {
            ChipLogDetail(Discovery, "\tInstance Name: %s", mAttributes.instanceName);
        }
        if (mAttributes.numIPs > 0)
        {
            ChipLogDetail(Discovery, "\tNumber of IPs: %u", mAttributes.numIPs);
        }
        char buf[chip::Inet::IPAddress::kMaxStringLength];
        if (strlen(mAttributes.ipAddresses[0].ToString(buf)) != 0)
        {
            for (unsigned j = 0; j < mAttributes.numIPs; j++)
            {
                char * ipAddressOut = mAttributes.ipAddresses[j].ToString(buf);
                ChipLogDetail(AppServer, "\tIP Address #%d: %s", j + 1, ipAddressOut);
            }
        }
        if (mAttributes.port > 0)
        {
            ChipLogDetail(Discovery, "\tPort: %u", mAttributes.port);
        }
        if (mAttributes.productId > 0)
        {
            ChipLogDetail(Discovery, "\tProduct ID: %u", mAttributes.productId);
        }
        if (mAttributes.vendorId > 0)
        {
            ChipLogDetail(Discovery, "\tVendor ID: %u", mAttributes.vendorId);
        }
        if (mAttributes.deviceType > 0)
        {
            ChipLogDetail(Discovery, "\tDevice Type: %" PRIu32, mAttributes.deviceType);
        }
    }
};

}; // namespace core
}; // namespace casting
}; // namespace matter
