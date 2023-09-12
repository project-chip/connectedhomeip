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

#include <string.h>
#include <vector>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>

namespace matter {
namespace casting {
namespace core {

using namespace memory;
//using namespace endpoint;


enum ConnectionError
{
    NO_ERROR          = 0,
    FAILED_TO_CONNECT = 1
};

using ConnectCallback    = std::function<void(ConnectionError)>;
using DisconnectCallback = std::function<void(void)>;

const int kPortMaxLength = 5; //port is uint16_t
const int kIdMaxLength = chip::Dnssd::kHostNameMaxLength + kPortMaxLength;

class Attributes
{
public:
    char id[kIdMaxLength] = {};
    char name[chip::Dnssd::kMaxDeviceNameLen + 1] = {};
    char host_name[chip::Dnssd::kHostNameMaxLength + 1] = {};
    size_t num_IPs; // number of valid IP addresses
    chip::Inet::IPAddress ip_address[chip::Dnssd::CommonResolutionData::kMaxIPAddresses];
    uint16_t product_id;
    uint16_t vendor_id;
    uint32_t type;
};

class CastingPlayer : public std::enable_shared_from_this<CastingPlayer>
{
private:
    //std::vector<Strong<Endpoint>> endpoints;
    bool connected = false;
    Attributes attributes;

public:
    CastingPlayer(Attributes playerAttributes)
    {
        attributes = playerAttributes;
    }
    const char* GetId() const { return attributes.id; }

    const char* GetName() const { return attributes.name; }

    const char* GetHostName() const { return attributes.host_name; }

    size_t GetNumIPs() const { return attributes.num_IPs; }

    const chip::Inet::IPAddress& GetIPAddress(size_t idx) const { return attributes.ip_address[idx]; }

    uint16_t GetProductId() const { return attributes.product_id; }

    uint16_t GetVendorId() const { return attributes.vendor_id; }

    uint32_t GetType() const { return attributes.type; }

// public:
    // void RegisterEndpoint(const Strong<Endpoint> endpoint) { endpoints.push_back(endpoint); }

    // const std::vector<Strong<Endpoint>> GetEndpoints() const { return endpoints; }

public:
    bool IsConnected() const { return connected; }

    void Connect(const long timeout, ConnectCallback onCompleted);
    void Disconnect(const long timeout, DisconnectCallback onCompleted);

    void LogDetail() const
    {
        if (strlen(attributes.id) != 0)
        {
            ChipLogDetail(Discovery, "\tID: %s", attributes.id);
        }
        if (strlen(attributes.name) != 0)
        {
            ChipLogDetail(Discovery, "\tName: %s", attributes.name);
        }
        if (strlen(attributes.host_name) != 0)
        {
            ChipLogDetail(Discovery, "\tHost Name: %s", attributes.host_name);
        }
        if (attributes.num_IPs > 0)
        {
            ChipLogDetail(Discovery, "\tNumber of IPs: %zu", attributes.num_IPs);
        }
        char buf[chip::Inet::IPAddress::kMaxStringLength];
        if (strlen(attributes.ip_address[0].ToString(buf)) != 0 )
        {
            for (unsigned j = 0; j < attributes.num_IPs; j++)
            {
                char * ipAddressOut = attributes.ip_address[j].ToString(buf);
                ChipLogDetail(AppServer, "\tIP Address #%d: %s", j + 1, ipAddressOut);
            }
        }
        if (attributes.product_id > 0)
        {
            ChipLogDetail(Discovery, "\tProduct ID: %u", attributes.product_id);
        }
        if (attributes.vendor_id > 0)
        {
            ChipLogDetail(Discovery, "\tVendor ID: %u", attributes.vendor_id);
        }
        if (attributes.type > 0)
        {
            ChipLogDetail(Discovery, "\tType: %" PRIu32, attributes.type);
        }
    }
};

}; // namespace core
}; // namespace casting
}; // namespace matter
