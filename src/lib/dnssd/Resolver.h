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
#include <utility>

#include "lib/support/logging/CHIPLogging.h"
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <lib/dnssd/Constants.h>
#include <lib/support/BytesToHex.h>
#include <messaging/ReliableMessageProtocolConfig.h>

namespace chip {
namespace Dnssd {

/// Node resolution data common to both operational and commissionable discovery
struct CommonResolutionData
{
    static constexpr unsigned kMaxIPAddresses = CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES;

    Inet::InterfaceId interfaceId;

    size_t numIPs = 0; // number of valid IP addresses
    Inet::IPAddress ipAddress[kMaxIPAddresses];

    uint16_t port                         = 0;
    char hostName[kHostNameMaxLength + 1] = {};
    bool supportsTcp                      = false;
    Optional<System::Clock::Milliseconds32> mrpRetryIntervalIdle;
    Optional<System::Clock::Milliseconds32> mrpRetryIntervalActive;

    CommonResolutionData() { Reset(); }

    bool IsValid() const { return !IsHost("") && (numIPs > 0) && (ipAddress[0] != chip::Inet::IPAddress::Any); }

    ReliableMessageProtocolConfig GetRemoteMRPConfig() const
    {
        const ReliableMessageProtocolConfig defaultConfig = GetDefaultMRPConfig();
        return ReliableMessageProtocolConfig(GetMrpRetryIntervalIdle().ValueOr(defaultConfig.mIdleRetransTimeout),
                                             GetMrpRetryIntervalActive().ValueOr(defaultConfig.mActiveRetransTimeout));
    }
    Optional<System::Clock::Milliseconds32> GetMrpRetryIntervalIdle() const { return mrpRetryIntervalIdle; }
    Optional<System::Clock::Milliseconds32> GetMrpRetryIntervalActive() const { return mrpRetryIntervalActive; }

    bool IsDeviceTreatedAsSleepy(const ReliableMessageProtocolConfig * defaultMRPConfig) const
    {
        // If either sleepy interval (Idle - SII, Active - SAI) has a value and that value is greater
        // than the value passed to this function, then the peer device will be treated as if it is
        // a Sleepy End Device (SED)
        return (mrpRetryIntervalIdle.HasValue() && (mrpRetryIntervalIdle.Value() > defaultMRPConfig->mIdleRetransTimeout)) ||
            (mrpRetryIntervalActive.HasValue() && (mrpRetryIntervalActive.Value() > defaultMRPConfig->mActiveRetransTimeout));
    }

    bool IsHost(const char * host) const { return strcmp(host, hostName) == 0; }

    void Reset()
    {
        memset(hostName, 0, sizeof(hostName));
        mrpRetryIntervalIdle   = NullOptional;
        mrpRetryIntervalActive = NullOptional;
        numIPs                 = 0;
        port                   = 0;
        supportsTcp            = false;
        interfaceId            = Inet::InterfaceId::Null();
        for (auto & addr : ipAddress)
        {
            addr = chip::Inet::IPAddress::Any;
        }
    }

    void LogDetail() const
    {
        if (!IsHost(""))
        {
            ChipLogDetail(Discovery, "\tHostname: %s", hostName);
        }
#if CHIP_DETAIL_LOGGING
        for (unsigned j = 0; j < numIPs; j++)
        {
            char buf[Inet::IPAddress::kMaxStringLength];
            char * ipAddressOut = ipAddress[j].ToString(buf);
            ChipLogDetail(Discovery, "\tIP Address #%d: %s", j + 1, ipAddressOut);
        }
#endif // CHIP_DETAIL_LOGGING
        if (port > 0)
        {
            ChipLogDetail(Discovery, "\tPort: %u", port);
        }
        if (mrpRetryIntervalIdle.HasValue())
        {
            ChipLogDetail(Discovery, "\tMrp Interval idle: %" PRIu32 " ms", mrpRetryIntervalIdle.Value().count());
        }
        else
        {
            ChipLogDetail(Discovery, "\tMrp Interval idle: not present");
        }
        if (mrpRetryIntervalActive.HasValue())
        {
            ChipLogDetail(Discovery, "\tMrp Interval active: %" PRIu32 " ms", mrpRetryIntervalActive.Value().count());
        }
        else
        {
            ChipLogDetail(Discovery, "\tMrp Interval active: not present");
        }
        ChipLogDetail(Discovery, "\tTCP Supported: %d", supportsTcp);
    }
};

/// Data that is specific to Operational Discovery of nodes
struct OperationalNodeData
{
    PeerId peerId;

    void Reset() { peerId = PeerId(); }
};

constexpr size_t kMaxDeviceNameLen         = 32;
constexpr size_t kMaxRotatingIdLen         = 50;
constexpr size_t kMaxPairingInstructionLen = 128;

/// Data that is specific to commisionable/commissioning node discovery
struct CommissionNodeData
{
    char instanceName[Commission::kInstanceNameMaxLength + 1] = {};
    uint16_t longDiscriminator                                = 0;
    uint16_t vendorId                                         = 0;
    uint16_t productId                                        = 0;
    uint8_t commissioningMode                                 = 0;
    uint32_t deviceType                                       = 0;
    char deviceName[kMaxDeviceNameLen + 1]                    = {};
    uint8_t rotatingId[kMaxRotatingIdLen]                     = {};
    size_t rotatingIdLen                                      = 0;
    uint16_t pairingHint                                      = 0;
    char pairingInstruction[kMaxPairingInstructionLen + 1]    = {};

    CommissionNodeData() {}

    void Reset()
    {
        // Let constructor clear things as default
        this->~CommissionNodeData();
        new (this) CommissionNodeData();
    }

    bool IsInstanceName(const char * instance) const { return strcmp(instance, instanceName) == 0; }

    void LogDetail() const
    {
        if (rotatingIdLen > 0)
        {
            char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1] = "";
            Encoding::BytesToUppercaseHexString(rotatingId, rotatingIdLen, rotatingIdString, sizeof(rotatingIdString));
            ChipLogDetail(Discovery, "\tRotating ID: %s", rotatingIdString);
        }
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
            ChipLogDetail(Discovery, "\tDevice Type: %" PRIu32, deviceType);
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
        if (!IsInstanceName(""))
        {
            ChipLogDetail(Discovery, "\tInstance Name: %s", instanceName);
        }
        ChipLogDetail(Discovery, "\tCommissioning Mode: %u", commissioningMode);
    }
};

struct ResolvedNodeData
{
    CommonResolutionData resolutionData;
    OperationalNodeData operationalData;

    void LogNodeIdResolved() const
    {
#if CHIP_PROGRESS_LOGGING
        // Would be nice to log the interface id, but sorting out how to do so
        // across our different InterfaceId implementations is a pain.
        ChipLogProgress(Discovery, "Node ID resolved for " ChipLogFormatX64 ":" ChipLogFormatX64,
                        ChipLogValueX64(operationalData.peerId.GetCompressedFabricId()),
                        ChipLogValueX64(operationalData.peerId.GetNodeId()));
        resolutionData.LogDetail();
#endif // CHIP_PROGRESS_LOGGING
    }
};

struct DiscoveredNodeData
{
    CommonResolutionData resolutionData;
    CommissionNodeData commissionData;

    void Reset()
    {
        resolutionData.Reset();
        commissionData.Reset();
    }
    DiscoveredNodeData() { Reset(); }

    void LogDetail() const
    {
        ChipLogDetail(Discovery, "Discovered node:");
        resolutionData.LogDetail();
        commissionData.LogDetail();
    }
};

enum class DiscoveryFilterType : uint8_t
{
    kNone,
    kShortDiscriminator,
    kLongDiscriminator,
    kVendorId,
    kDeviceType,
    kCommissioningMode,
    kInstanceName,
    kCommissioner,
    kCompressedFabricId,
};
struct DiscoveryFilter
{
    DiscoveryFilterType type;
    uint64_t code             = 0;
    const char * instanceName = nullptr;
    DiscoveryFilter() : type(DiscoveryFilterType::kNone), code(0) {}
    DiscoveryFilter(const DiscoveryFilterType newType) : type(newType) {}
    DiscoveryFilter(const DiscoveryFilterType newType, uint64_t newCode) : type(newType), code(newCode) {}
    DiscoveryFilter(const DiscoveryFilterType newType, const char * newInstanceName) : type(newType), instanceName(newInstanceName)
    {}
    bool operator==(const DiscoveryFilter & other) const
    {
        if (type != other.type)
        {
            return false;
        }
        if (type == DiscoveryFilterType::kInstanceName)
        {
            return (instanceName != nullptr) && (other.instanceName != nullptr) && (strcmp(instanceName, other.instanceName) == 0);
        }

        return code == other.code;
    }
};
enum class DiscoveryType
{
    kUnknown,
    kOperational,
    kCommissionableNode,
    kCommissionerNode
};

/// Callbacks for resolving operational node resolution
class OperationalResolveDelegate
{
public:
    virtual ~OperationalResolveDelegate() = default;

    /// Called within the CHIP event loop after a successful node resolution.
    ///
    /// May be called multiple times: implementations may call this once per
    /// received packet and MDNS packets may arrive over different interfaces
    /// which will make nodeData have different content.
    virtual void OnOperationalNodeResolved(const ResolvedNodeData & nodeData) = 0;

    /// Notify a final failure for a node operational resolution.
    ///
    /// Called within the chip event loop if node resolution could not be performed.
    /// This may be due to internal errors or timeouts.
    ///
    /// This will be called only if 'OnOperationalNodeResolved' is never called.
    virtual void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) = 0;
};

/// Callbacks for discovering nodes advertising non-operational status:
///   - Commissioners
///   - Nodes in commissioning modes over IP (e.g. ethernet devices, devices already
///     connected to thread/wifi or devices with a commissioning window open)
class CommissioningResolveDelegate
{
public:
    virtual ~CommissioningResolveDelegate() = default;

    /// Called within the CHIP event loop once a node is discovered.
    ///
    /// May be called multiple times as more nodes send their answer to a
    /// multicast discovery query
    virtual void OnNodeDiscovered(const DiscoveredNodeData & nodeData) = 0;
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
    virtual CHIP_ERROR Init(Inet::EndPointManager<Inet::UDPEndPoint> * endPointManager) = 0;

    /**
     * Returns whether the resolver has completed the initialization.
     *
     * Returns true if the resolver is ready to take node resolution and discovery requests.
     */
    virtual bool IsInitialized() = 0;

    /**
     * Shuts down the resolver if it has been initialized before.
     */
    virtual void Shutdown() = 0;

    /**
     * If nullptr is passed, the previously registered delegate is unregistered.
     */
    virtual void SetOperationalDelegate(OperationalResolveDelegate * delegate) = 0;

    /**
     * If nullptr is passed, the previously registered delegate is unregistered.
     */
    virtual void SetCommissioningDelegate(CommissioningResolveDelegate * delegate) = 0;

    /**
     * Requests resolution of the given operational node service.
     *
     * This will trigger a DNSSD query.
     *
     * When the operation succeeds or fails, and a resolver delegate has been registered,
     * the result of the operation is passed to the delegate's `OnOperationalNodeResolved` or
     * `OnOperationalNodeResolutionFailed` method, respectively.
     *
     * Multiple calls to ResolveNodeId may be coalesced by the implementation
     * and lead to just one call to
     * OnOperationalNodeResolved/OnOperationalNodeResolutionFailed, as long as
     * the later calls cause the underlying querying mechanism to re-query as if
     * there were no coalescing.
     *
     * A single call to ResolveNodeId may lead to multiple calls to
     * OnOperationalNodeResolved with different IP addresses.
     *
     * @see NodeIdResolutionNoLongerNeeded.
     */
    virtual CHIP_ERROR ResolveNodeId(const PeerId & peerId) = 0;

    /*
     * Notify the resolver that one of the consumers that called ResolveNodeId
     * successfully no longer needs the resolution result (e.g. because it got
     * the result via OnOperationalNodeResolved, or got an via
     * OnOperationalNodeResolutionFailed, or no longer cares about future
     * updates).
     *
     * There must be a NodeIdResolutionNoLongerNeeded call that matches every
     * successful ResolveNodeId call.  In particular, implementations of
     * OnOperationalNodeResolved and OnOperationalNodeResolutionFailed must call
     * NodeIdResolutionNoLongerNeeded once for each prior successful call to
     * ResolveNodeId for the relevant PeerId that has not yet had a matching
     * NodeIdResolutionNoLongerNeeded call made.
     */
    virtual void NodeIdResolutionNoLongerNeeded(const PeerId & peerId) = 0;

    /**
     * Finds all commissionable nodes matching the given filter.
     *
     * Whenever a new matching node is found and a resolver delegate has been registered,
     * the node information is passed to the delegate's `OnNodeDiscoveryComplete` method.
     */
    virtual CHIP_ERROR DiscoverCommissionableNodes(DiscoveryFilter filter = DiscoveryFilter()) = 0;

    /**
     * Finds all commissioner nodes matching the given filter.
     *
     * Whenever a new matching node is found and a resolver delegate has been registered,
     * the node information is passed to the delegate's `OnNodeDiscoveryComplete` method.
     */
    virtual CHIP_ERROR DiscoverCommissioners(DiscoveryFilter filter = DiscoveryFilter()) = 0;

    /**
     * Stop discovery (of commissionable or commissioner nodes).
     *
     * Some back ends may not support stopping discovery, so consumers should
     * not assume they will stop getting callbacks after calling this.
     */
    virtual CHIP_ERROR StopDiscovery() = 0;

    /**
     * Verify the validity of an address that appears to be out of date (for example
     * because establishing a connection to it has failed).
     */
    virtual CHIP_ERROR ReconfirmRecord(const char * hostname, Inet::IPAddress address, Inet::InterfaceId interfaceId) = 0;

    /**
     * Provides the system-wide implementation of the service resolver
     */
    static Resolver & Instance();
};

} // namespace Dnssd
} // namespace chip
