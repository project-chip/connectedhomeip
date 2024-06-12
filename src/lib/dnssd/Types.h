/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <cstdint>
#include <cstring>
#include <optional>

#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/PeerId.h>
#include <lib/dnssd/Constants.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/Variant.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ReliableMessageProtocolConfig.h>

namespace chip {
namespace Dnssd {

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

struct CommonResolutionData
{
    static constexpr unsigned kMaxIPAddresses = CHIP_DEVICE_CONFIG_MAX_DISCOVERED_IP_ADDRESSES;

    Inet::InterfaceId interfaceId;

    size_t numIPs = 0; // number of valid IP addresses
    Inet::IPAddress ipAddress[kMaxIPAddresses];

    uint16_t port                         = 0;
    char hostName[kHostNameMaxLength + 1] = {};
    bool supportsTcpClient                = false;
    bool supportsTcpServer                = false;
    std::optional<bool> isICDOperatingAsLIT;
    std::optional<System::Clock::Milliseconds32> mrpRetryIntervalIdle;
    std::optional<System::Clock::Milliseconds32> mrpRetryIntervalActive;
    std::optional<System::Clock::Milliseconds16> mrpRetryActiveThreshold;

    CommonResolutionData() { Reset(); }

    bool IsValid() const { return !IsHost("") && (numIPs > 0) && (ipAddress[0] != chip::Inet::IPAddress::Any); }

    ReliableMessageProtocolConfig GetRemoteMRPConfig() const
    {
        const ReliableMessageProtocolConfig defaultConfig = GetDefaultMRPConfig();
        return ReliableMessageProtocolConfig(GetMrpRetryIntervalIdle().value_or(defaultConfig.mIdleRetransTimeout),
                                             GetMrpRetryIntervalActive().value_or(defaultConfig.mActiveRetransTimeout),
                                             GetMrpRetryActiveThreshold().value_or(defaultConfig.mActiveThresholdTime));
    }
    std::optional<System::Clock::Milliseconds32> GetMrpRetryIntervalIdle() const { return mrpRetryIntervalIdle; }
    std::optional<System::Clock::Milliseconds32> GetMrpRetryIntervalActive() const { return mrpRetryIntervalActive; }
    std::optional<System::Clock::Milliseconds16> GetMrpRetryActiveThreshold() const { return mrpRetryActiveThreshold; }

    bool IsDeviceTreatedAsSleepy(const ReliableMessageProtocolConfig * defaultMRPConfig) const
    {
        // If either session interval (Idle - SII, Active - SAI) has a value and that value is greater
        // than the value passed to this function, then the peer device will be treated as if it is
        // a Sleepy End Device (SED)
        return (mrpRetryIntervalIdle.has_value() && (*mrpRetryIntervalIdle > defaultMRPConfig->mIdleRetransTimeout)) ||
            (mrpRetryIntervalActive.has_value() && (*mrpRetryIntervalActive > defaultMRPConfig->mActiveRetransTimeout));
    }

    bool IsHost(const char * host) const { return strcmp(host, hostName) == 0; }

    void Reset()
    {
        memset(hostName, 0, sizeof(hostName));
        mrpRetryIntervalIdle    = std::nullopt;
        mrpRetryIntervalActive  = std::nullopt;
        mrpRetryActiveThreshold = std::nullopt;
        isICDOperatingAsLIT     = std::nullopt;
        numIPs                  = 0;
        port                    = 0;
        supportsTcpClient       = false;
        supportsTcpServer       = false;
        interfaceId             = Inet::InterfaceId::Null();
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
        if (mrpRetryIntervalIdle.has_value())
        {
            ChipLogDetail(Discovery, "\tMrp Interval idle: %" PRIu32 " ms", mrpRetryIntervalIdle->count());
        }
        else
        {
            ChipLogDetail(Discovery, "\tMrp Interval idle: not present");
        }
        if (mrpRetryIntervalActive.has_value())
        {
            ChipLogDetail(Discovery, "\tMrp Interval active: %" PRIu32 " ms", mrpRetryIntervalActive->count());
        }
        else
        {
            ChipLogDetail(Discovery, "\tMrp Interval active: not present");
        }
        if (mrpRetryActiveThreshold.has_value())
        {
            ChipLogDetail(Discovery, "\tMrp Active Threshold: %u ms", mrpRetryActiveThreshold->count());
        }
        else
        {
            ChipLogDetail(Discovery, "\tMrp Active Threshold: not present");
        }
        ChipLogDetail(Discovery, "\tTCP Client Supported: %d", supportsTcpClient);
        ChipLogDetail(Discovery, "\tTCP Server Supported: %d", supportsTcpServer);
        if (isICDOperatingAsLIT.has_value())
        {
            ChipLogDetail(Discovery, "\tThe ICD operates in %s", *isICDOperatingAsLIT ? "LIT" : "SIT");
        }
        else
        {
            ChipLogDetail(Discovery, "\tICD: not present");
        }
    }
};

/// Data that is specific to Operational Discovery of nodes
struct OperationalNodeData
{
    PeerId peerId;
    bool hasZeroTTL;
    void Reset() { peerId = PeerId(); }
};

struct OperationalNodeBrowseData : public OperationalNodeData
{
    OperationalNodeBrowseData() { Reset(); };
    void LogDetail() const
    {
        ChipLogDetail(Discovery, "Discovered Operational node:\r\n");
        ChipLogDetail(Discovery, "\tNode Instance: " ChipLogFormatPeerId, ChipLogValuePeerId(peerId));
        ChipLogDetail(Discovery, "\thasZeroTTL: %s\r\n", hasZeroTTL ? "true" : "false");
    }
};

inline constexpr size_t kMaxDeviceNameLen         = 32;
inline constexpr size_t kMaxRotatingIdLen         = 50;
inline constexpr size_t kMaxPairingInstructionLen = 128;

/// Data that is specific to commisionable/commissioning node discovery
struct CommissionNodeData : public CommonResolutionData
{
    size_t rotatingIdLen                                      = 0;
    uint32_t deviceType                                       = 0;
    uint16_t longDiscriminator                                = 0;
    uint16_t vendorId                                         = 0;
    uint16_t productId                                        = 0;
    uint16_t pairingHint                                      = 0;
    uint8_t commissioningMode                                 = 0;
    bool supportsCommissionerGeneratedPasscode                = false;
    uint8_t rotatingId[kMaxRotatingIdLen]                     = {};
    char instanceName[Commission::kInstanceNameMaxLength + 1] = {};
    char deviceName[kMaxDeviceNameLen + 1]                    = {};
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
        ChipLogDetail(Discovery, "Discovered commissionable/commissioner node:");
        CommonResolutionData::LogDetail();

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
        ChipLogDetail(Discovery, "\tSupports Commissioner Generated Passcode: %s",
                      supportsCommissionerGeneratedPasscode ? "true" : "false");
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
        ChipLogProgress(Discovery, "Node ID resolved for " ChipLogFormatPeerId, ChipLogValuePeerId(operationalData.peerId));
        resolutionData.LogDetail();
#endif // CHIP_PROGRESS_LOGGING
    }
};

using DiscoveredNodeData = Variant<CommissionNodeData, OperationalNodeBrowseData>;

/// Callbacks for discovering nodes advertising both operational and non-operational status:
///   - Commissioners
///   - Nodes in commissioning modes over IP (e.g. ethernet devices, devices already
///     connected to thread/wifi or devices with a commissioning window open)
///   - Operational nodes
class DiscoverNodeDelegate
{
public:
    virtual ~DiscoverNodeDelegate() = default;

    /// Called within the CHIP event loop once a node is discovered.
    ///
    /// May be called multiple times as more nodes send their answer to a
    /// multicast discovery query
    virtual void OnNodeDiscovered(const DiscoveredNodeData & nodeData) = 0;
};

} // namespace Dnssd
} // namespace chip
