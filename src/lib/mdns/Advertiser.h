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
    uint16_t GetPort() const { return mPort; }

private:
    uint64_t mFabricId = 0;
    uint64_t mNodeId   = 0;
    uint16_t mPort     = CHIP_PORT;
};

class CommissionAdvertisingParameters
{
public:
    CommissionAdvertisingParameters & SetDiscriminator(uint16_t discriminator)
    {
        mDiscriminator = discriminator;
        return *this;
    }
    uint16_t GetDiscriminator() const { return mDiscriminator; }

    CommissionAdvertisingParameters & SetVendorId(uint16_t vendorId)
    {
        mVendorId = vendorId;
        return *this;
    }
    uint16_t GetVendorId() const { return mVendorId; }

    CommissionAdvertisingParameters & SetProductId(uint16_t productId)
    {
        mProductId = productId;
        return *this;
    }
    uint16_t GetProductId() const { return mProductId; }

    CommissionAdvertisingParameters & SetIdentifier(uint64_t identifier)
    {
        mIdentifier = identifier;
        return *this;
    }
    uint64_t GetIdentifier() const { return mIdentifier; }

    CommissionAdvertisingParameters & SetPort(uint16_t port)
    {
        mPort = port;
        return *this;
    }
    uint16_t GetPort() const { return mPort; }

private:
    uint16_t mDiscriminator;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint64_t mIdentifier;
    uint16_t mPort = CHIP_PORT;
};

/// Handles advertising of CHIP nodes
class ServiceAdvertiser
{
public:
    virtual ~ServiceAdvertiser() {}

    /// Starts the advertiser. Items 'Advertised' will become visible.
    /// May be called before OR after Advertise() calls.
    virtual CHIP_ERROR Start(chip::Inet::InetLayer * inetLayer, uint64_t macAddress, uint16_t port, bool enableIPv4) = 0;

    /// Advertises the CHIP node as an operational node
    virtual CHIP_ERROR AdvertiseOperational(const OperationalAdvertisingParameters & params) = 0;

    /// Advertises the CHIP node as a commission node
    virtual CHIP_ERROR AdvertiseCommission(const CommissionAdvertisingParameters & params) = 0;

    /// Provides the system-wide implementation of the service advertiser
    static ServiceAdvertiser & Instance();
};

} // namespace Mdns
} // namespace chip
