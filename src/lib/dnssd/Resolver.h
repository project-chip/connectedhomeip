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
#include <lib/dnssd/Constants.h>
#include <lib/support/BytesToHex.h>

namespace chip {
namespace Dnssd {

constexpr uint32_t kUndefinedRetryInterval = std::numeric_limits<uint32_t>::max();

struct ResolvedNodeData
{
    void LogNodeIdResolved()
    {
#if CHIP_PROGRESS_LOGGING
        char addrBuffer[Inet::IPAddress::kMaxStringLength];
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
    Inet::IPAddress mAddress               = Inet::IPAddress::Any;
    Inet::InterfaceId mInterfaceId         = Inet::InterfaceId::Null();
    uint16_t mPort                         = 0;
    char mHostName[kHostNameMaxLength + 1] = {};
    bool mSupportsTcp                      = false;
    uint32_t mMrpRetryIntervalIdle         = kUndefinedRetryInterval;
    uint32_t mMrpRetryIntervalActive       = kUndefinedRetryInterval;
};

constexpr size_t kMaxDeviceNameLen         = 32;
constexpr size_t kMaxRotatingIdLen         = 50;
constexpr size_t kMaxPairingInstructionLen = 128;

struct DiscoveredNodeData
{
    // TODO(cecille): is 4 OK? IPv6 LL, GUA, ULA, IPv4?
    static constexpr int kMaxIPAddresses = 5;
    char hostName[kHostNameMaxLength + 1];
    char instanceName[Commissionable::kInstanceNameMaxLength + 1];
    uint16_t longDiscriminator;
    uint16_t vendorId;
    uint16_t productId;
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
        port                   = 0;
        for (int i = 0; i < kMaxIPAddresses; ++i)
        {
            ipAddress[i] = chip::Inet::IPAddress::Any;
        }
    }
    DiscoveredNodeData() { Reset(); }
    bool IsHost(const char * host) const { return strcmp(host, hostName) == 0; }
    bool IsInstanceName(const char * instance) const { return strcmp(instance, instanceName) == 0; }
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
            char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(rotatingId, rotatingIdLen, rotatingIdString, sizeof(rotatingIdString));
            ChipLogDetail(Discovery, "Rotating ID: %s", rotatingIdString);
        }
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID
        if (strlen(deviceName) != 0)
        {
            ChipLogDetail(Discovery, "\tDevice Name: %s", deviceName);
        }
        if (vendorId > 0)
        {
            ChipLogDetail(Discovery, "\tVendor ID: %u", vendorId);
        }
        if (productId > 0)
        {
            ChipLogDetail(Discovery, "\tProduct ID: %u", productId);
        }
        if (deviceType > 0)
        {
            ChipLogDetail(Discovery, "\tDevice Type: %u", deviceType);
        }
        if (longDiscriminator > 0)
        {
            ChipLogDetail(Discovery, "\tLong Discriminator: %u", longDiscriminator);
        }
        if (strlen(pairingInstruction) != 0)
        {
            ChipLogDetail(Discovery, "\tPairing Instruction: %s", pairingInstruction);
        }
        if (pairingHint > 0)
        {
            ChipLogDetail(Discovery, "\tPairing Hint: %u", pairingHint);
        }
        if (!IsHost(""))
        {
            ChipLogDetail(Discovery, "\tHostname: %s", hostName);
        }
        if (!IsInstanceName(""))
        {
            ChipLogDetail(Discovery, "\tInstance Name: %s", instanceName);
        }
        for (int j = 0; j < numIPs; j++)
        {
#if CHIP_DETAIL_LOGGING
            char buf[Inet::IPAddress::kMaxStringLength];
            char * ipAddressOut = ipAddress[j].ToString(buf);
            ChipLogDetail(Discovery, "\tIP Address #%d: %s", j + 1, ipAddressOut);
            (void) ipAddressOut;
#endif // CHIP_DETAIL_LOGGING
        }
        if (port > 0)
        {
            ChipLogDetail(Discovery, "\tPort: %u", port);
        }
        ChipLogDetail(Discovery, "\tCommissioning Mode: %u", commissioningMode);
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
    kInstanceName,
    kCommissioner,
    kCompressedFabricId,
};
struct DiscoveryFilter
{
    DiscoveryFilterType type;
    uint64_t code;
    const char * instanceName;
    DiscoveryFilter() : type(DiscoveryFilterType::kNone), code(0) {}
    DiscoveryFilter(DiscoveryFilterType newType) : type(newType) {}
    DiscoveryFilter(DiscoveryFilterType newType, uint64_t newCode) : type(newType), code(newCode) {}
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

/**
 * Interface for resolving CHIP DNS-SD services
 */
class Resolver
{
public:
    virtual ~Resolver() {}

    /**
     * Initializes the resolver.
     *
     * The method must be called before other methods of this class.
     * If the resolver has already been initialized, the method exits immediately with no error.
     */
    virtual CHIP_ERROR Init(chip::Inet::InetLayer * inetLayer) = 0;

    /**
     * Shuts down the resolver if it has been initialized before.
     */
    virtual void Shutdown() = 0;

    /**
     * Registers a resolver delegate. If nullptr is passed, the previously registered delegate
     * is unregistered.
     */
    virtual void SetResolverDelegate(ResolverDelegate * delegate) = 0;

    /**
     * Requests resolution of the given operational node service.
     *
     * When the operation succeeds or fails, and a resolver delegate has been registered,
     * the result of the operation is passed to the delegate's `OnNodeIdResolved` or
     * `OnNodeIdResolutionFailed` method, respectively.
     */
    virtual CHIP_ERROR ResolveNodeId(const PeerId & peerId, Inet::IPAddressType type) = 0;

    /**
     * Finds all commissionable nodes matching the given filter.
     *
     * Whenever a new matching node is found and a resolver delegate has been registered,
     * the node information is passed to the delegate's `OnNodeDiscoveryComplete` method.
     */
    virtual CHIP_ERROR FindCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) = 0;

    /**
     * Finds all commissioner nodes matching the given filter.
     *
     * Whenever a new matching node is found and a resolver delegate has been registered,
     * the node information is passed to the delegate's `OnNodeDiscoveryComplete` method.
     */
    virtual CHIP_ERROR FindCommissioners(DiscoveryFilter filter = DiscoveryFilter()) = 0;

    /**
     * Provides the system-wide implementation of the service resolver
     */
    static Resolver & Instance();
};

} // namespace Dnssd
} // namespace chip
