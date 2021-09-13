/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <limits>

#include "lib/support/logging/CHIPLogging.h"
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <lib/support/BytesToHex.h>

namespace chip {
namespace Mdns {

// Largest host name is 64-bits in hex.
static constexpr int kMaxHostNameSize      = 16;
constexpr uint32_t kUndefinedRetryInterval = std::numeric_limits<uint32_t>::max();

struct ResolvedNodeData
{
    void LogNodeIdResolved()
    {
#if CHIP_PROGRESS_LOGGING
        char addrBuffer[Inet::kMaxIPAddressStringLength + 1];
        mAddress.ToString(addrBuffer);
        // Would be nice to log the interface id, but sorting out how to do so
        // across our differnet InterfaceId implementations is a pain.
        ChipLogProgress(Discovery, "Node ID resolved for 0x" ChipLogFormatX64 " to [%s]:%" PRIu16,
                        ChipLogValueX64(mPeerId.GetNodeId()), addrBuffer, mPort);
#endif // CHIP_PROGRESS_LOGGING
    }

    Optional<uint32_t> GetMrpRetryIntervalIdle() const
    {
        return mMrpRetryIntervalIdle != kUndefinedRetryInterval ? Optional<uint32_t>{ mMrpRetryIntervalIdle }
                                                                : Optional<uint32_t>{};
    }

    Optional<uint32_t> GetMrpRetryIntervalActive() const
    {
        return mMrpRetryIntervalActive != kUndefinedRetryInterval ? Optional<uint32_t>{ mMrpRetryIntervalActive }
                                                                  : Optional<uint32_t>{};
    }

    PeerId mPeerId;
    Inet::IPAddress mAddress             = Inet::IPAddress::Any;
    Inet::InterfaceId mInterfaceId       = INET_NULL_INTERFACEID;
    uint16_t mPort                       = 0;
    char mHostName[kMaxHostNameSize + 1] = {};
    bool mSupportsTcp                    = false;
    uint32_t mMrpRetryIntervalIdle       = kUndefinedRetryInterval;
    uint32_t mMrpRetryIntervalActive     = kUndefinedRetryInterval;
};

constexpr size_t kMaxDeviceNameLen         = 32;
constexpr size_t kMaxRotatingIdLen         = 50;
constexpr size_t kMaxPairingInstructionLen = 128;

static constexpr int kMaxInstanceNameSize = 16;
struct DiscoveredNodeData
{
    // TODO(cecille): is 4 OK? IPv6 LL, GUA, ULA, IPv4?
    static constexpr int kMaxIPAddresses = 5;
    char hostName[kMaxHostNameSize + 1];
    char instanceName[kMaxInstanceNameSize + 1];
    uint16_t longDiscriminator;
    uint16_t vendorId;
    uint16_t productId;
    uint8_t additionalPairing;
    uint8_t commissioningMode;
    // TODO: possibly 32-bit - see spec issue #3226
    uint16_t deviceType;
    char deviceName[kMaxDeviceNameLen + 1];
    uint8_t rotatingId[kMaxRotatingIdLen];
    size_t rotatingIdLen;
    uint16_t pairingHint;
    char pairingInstruction[kMaxPairingInstructionLen + 1];
    bool supportsTcp;
    uint32_t mrpRetryIntervalIdle;
    uint32_t mrpRetryIntervalActive;
    uint16_t port;
    int numIPs;
    Inet::InterfaceId interfaceId[kMaxIPAddresses];
    Inet::IPAddress ipAddress[kMaxIPAddresses];

    void Reset()
    {
        memset(hostName, 0, sizeof(hostName));
        memset(instanceName, 0, sizeof(instanceName));
        longDiscriminator = 0;
        vendorId          = 0;
        productId         = 0;
        additionalPairing = 0;
        commissioningMode = 0;
        deviceType        = 0;
        memset(deviceName, 0, sizeof(deviceName));
        memset(rotatingId, 0, sizeof(rotatingId));
        rotatingIdLen = 0;
        memset(pairingInstruction, 0, sizeof(pairingInstruction));
        pairingHint            = 0;
        supportsTcp            = false;
        mrpRetryIntervalIdle   = kUndefinedRetryInterval;
        mrpRetryIntervalActive = kUndefinedRetryInterval;
        numIPs                 = 0;
        for (int i = 0; i < kMaxIPAddresses; ++i)
        {
            ipAddress[i] = chip::Inet::IPAddress::Any;
        }
    }
    DiscoveredNodeData() { Reset(); }
    bool IsHost(const char * host) const { return strcmp(host, hostName) == 0; }
    bool IsValid() const { return !IsHost("") && ipAddress[0] != chip::Inet::IPAddress::Any; }

    Optional<uint32_t> GetMrpRetryIntervalIdle() const
    {
        return mrpRetryIntervalIdle != kUndefinedRetryInterval ? Optional<uint32_t>{ mrpRetryIntervalIdle } : Optional<uint32_t>{};
    }

    Optional<uint32_t> GetMrpRetryIntervalActive() const
    {
        return mrpRetryIntervalActive != kUndefinedRetryInterval ? Optional<uint32_t>{ mrpRetryIntervalActive }
                                                                 : Optional<uint32_t>{};
    }

    void LogDetail() const
    {
#if CHIP_ENABLE_ROTATING_DEVICE_ID
        if (rotatingIdLen > 0)
        {
            char rotatingIdString[chip::Mdns::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(rotatingId, rotatingIdLen, rotatingIdString, sizeof(rotatingIdString));
            ChipLogDetail(Discovery, "Rotating ID: %s", rotatingIdString);
        }
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
        if (strlen(deviceName) != 0)
        {
            ChipLogDetail(Discovery, "Device Name: %s", deviceName);
        }
        if (vendorId > 0)
        {
            ChipLogDetail(Discovery, "Vendor ID: %u", vendorId);
        }
        if (productId > 0)
        {
            ChipLogDetail(Discovery, "Product ID: %u", productId);
        }
        if (deviceType > 0)
        {
            ChipLogDetail(Discovery, "Device Type: %u", deviceType);
        }
        if (longDiscriminator > 0)
        {
            ChipLogDetail(Discovery, "Long Discriminator: %u", longDiscriminator);
        }
        if (additionalPairing > 0)
        {
            ChipLogDetail(Discovery, "Additional Pairing: %u", additionalPairing);
        }
        if (strlen(pairingInstruction) != 0)
        {
            ChipLogDetail(Discovery, "Pairing Instruction: %s", pairingInstruction);
        }
        if (pairingHint > 0)
        {
            ChipLogDetail(Discovery, "Pairing Hint: 0x%x", pairingHint);
        }
        if (!IsHost(""))
        {
            ChipLogDetail(Discovery, "Hostname: %s", hostName);
        }
        for (int j = 0; j < numIPs; j++)
        {
#if CHIP_DETAIL_LOGGING
            char buf[Inet::kMaxIPAddressStringLength];
            char * ipAddressOut = ipAddress[j].ToString(buf);
            ChipLogDetail(Discovery, "IP Address #%d: %s", j + 1, ipAddressOut);
            (void) ipAddressOut;
#endif // CHIP_DETAIL_LOGGING
        }
        if (port > 0)
        {
            ChipLogDetail(Discovery, "Port: %u", port);
        }
        ChipLogDetail(Discovery, "Commissioning Mode: %u", commissioningMode);
    }
};

enum class DiscoveryFilterType : uint8_t
{
    kNone,
    kShort,
    kLong,
    kVendor,
    kDeviceType,
    kCommissioningMode,
    kCommissioningModeFromCommand,
    kInstanceName,
    kCommissioner
};
struct DiscoveryFilter
{
    DiscoveryFilterType type;
    uint16_t code;
    const char * instanceName;
    DiscoveryFilter() : type(DiscoveryFilterType::kNone), code(0) {}
    DiscoveryFilter(DiscoveryFilterType newType, uint16_t newCode) : type(newType), code(newCode) {}
    DiscoveryFilter(DiscoveryFilterType newType, const char * newInstanceName) : type(newType), instanceName(newInstanceName) {}
};
enum class DiscoveryType
{
    kUnknown,
    kOperational,
    kCommissionableNode,
    kCommissionerNode
};
/// Groups callbacks for CHIP service resolution requests
class ResolverDelegate
{
public:
    virtual ~ResolverDelegate() = default;

    /// Called when a requested CHIP node ID has been successfully resolved
    virtual void OnNodeIdResolved(const ResolvedNodeData & nodeData) = 0;

    /// Called when a CHIP node ID resolution has failed
    virtual void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) = 0;

    // Called when a CHIP Node acting as Commissioner or in commissioning mode is found
    virtual void OnNodeDiscoveryComplete(const DiscoveredNodeData & nodeData) = 0;
};

/// Interface for resolving CHIP services
class Resolver
{
public:
    virtual ~Resolver() {}

    /// Ensures that the resolver is started.
    /// Must be called before any ResolveNodeId calls.
    ///
    /// Unsual name to allow base MDNS classes to implement both Advertiser and Resolver interfaces.
    virtual CHIP_ERROR StartResolver(chip::Inet::InetLayer * inetLayer, uint16_t port) = 0;

    /// Registers a resolver delegate if none has been registered before
    virtual CHIP_ERROR SetResolverDelegate(ResolverDelegate * delegate) = 0;

    /// Requests resolution of a node ID to its address
    virtual CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) = 0;

    // Finds all nodes with the given filter that are currently in commissioning mode.
    virtual CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) = 0;

    // Finds all nodes with the given filter that are currently acting as Commissioners.
    virtual CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) = 0;

    /// Provides the system-wide implementation of the service resolver
    static Resolver & Instance();
};

} // namespace Mdns
} // namespace chip
