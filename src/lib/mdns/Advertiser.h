/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <cstdint>

#include <core/CHIPError.h>
#include <inet/InetLayer.h>

namespace chip {
namespace Mdns {

static constexpr uint16_t kMdnsPort = 5353;

/// Defines parameters required for advertising a CHIP node
/// over mDNS as an 'operationally ready' node.
class OperationalAdvertisingParameters
{
public:
    OperationalAdvertisingParameters & SetFabricId(uint64_t fabricId)
    {
        mFabricId = fabricId;
        return *this;
    }
    uint64_t GetFabricId() const { return mFabricId; }

    OperationalAdvertisingParameters & SetNodeId(uint64_t nodeId)
    {
        mNodeId = nodeId;
        return *this;
    }
    uint64_t GetNodeId() const { return mNodeId; }

    OperationalAdvertisingParameters & SetPort(uint16_t port)
    {
        mPort = port;
        return *this;
    }
    uint64_t GetPort() const { return mPort; }

    OperationalAdvertisingParameters & EnableIpV4(bool enable)
    {
        mEnableIPv4 = enable;
        return *this;
    }
    bool IsIPv4Enabled() const { return mEnableIPv4; }

private:
    uint64_t mFabricId = 0;
    uint64_t mNodeId   = 0;
    uint16_t mPort     = CHIP_PORT;
    bool mEnableIPv4   = true;
};

/// Handles advertising of CHIP nodes
class ServiceAdvertiser
{
public:
    virtual ~ServiceAdvertiser() {}

    /// Starts the advertiser. Items 'Advertised' will become visible.
    /// May be called before OR after Advertise() calls.
    virtual CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint16_t port) = 0;

    /// Advertises the CHIP node as an operational node
    virtual CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) = 0;

    /// Provides the system-wide implementation of the service advertiser
    static ServiceAdvertiser & Instance();
};

} // namespace Mdns
} // namespace chip
