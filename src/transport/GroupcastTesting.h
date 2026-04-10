/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

namespace chip {
namespace Groupcast {

/**
 * @brief Class to store GroupcastTesting event data
 *
 * This class stores all fields from the GroupcastTesting event,
 * including IP addresses as byte arrays for proper storage.
 */
class Testing
{
public:
    /**
     * @brief Test result enum matching GroupcastTestResultEnum from the Groupcast cluster.
     * This cannot point to the raw code-generated type due to build deps, so it is copied for convenience.
     */
    enum Result : uint8_t
    {
        kSuccess        = 0x00,
        kGeneralError   = 0x01,
        kMessageReplay  = 0x02,
        kFailedAuth     = 0x03,
        kNoAvailableKey = 0x04,
        kSendFailure    = 0x05,
    };

    static constexpr size_t kIPv6AddressLength = 16; // IPv6 address is 16 bytes

    Testing()  = default;
    ~Testing() = default;

    // Getters for optional fields
    const chip::Optional<chip::GroupId> & GetGroupID() const { return mGroupID; }
    const chip::Optional<chip::EndpointId> & GetEndpointID() const { return mEndpointID; }
    const chip::Optional<chip::ClusterId> & GetClusterID() const { return mClusterID; }
    const chip::Optional<uint32_t> & GetElementID() const { return mElementID; }
    const chip::Optional<bool> & GetAccessAllowed() const { return mAccessAllowed; }

    // Setters for optional fields
    void SetGroupID(const chip::Optional<chip::GroupId> & groupID) { mGroupID = groupID; }
    void SetGroupID(chip::GroupId groupID) { mGroupID = MakeOptional(groupID); }
    void ClearGroupID() { mGroupID = chip::Optional<chip::GroupId>(); }

    void SetEndpointID(const chip::Optional<chip::EndpointId> & endpointID) { mEndpointID = endpointID; }
    void SetEndpointID(chip::EndpointId endpointID) { mEndpointID = MakeOptional(endpointID); }
    void ClearEndpointID() { mEndpointID = chip::Optional<chip::EndpointId>(); }

    void SetClusterID(const chip::Optional<chip::ClusterId> & clusterID) { mClusterID = clusterID; }
    void SetClusterID(chip::ClusterId clusterID) { mClusterID = MakeOptional(clusterID); }
    void ClearClusterID() { mClusterID = chip::Optional<chip::ClusterId>(); }

    void SetElementID(const chip::Optional<uint32_t> & elementID) { mElementID = elementID; }
    void SetElementID(uint32_t elementID) { mElementID = MakeOptional(elementID); }
    void ClearElementID() { mElementID = chip::Optional<uint32_t>(); }

    void SetAccessAllowed(const chip::Optional<bool> & accessAllowed) { mAccessAllowed = accessAllowed; }
    void SetAccessAllowed(bool accessAllowed) { mAccessAllowed = MakeOptional(accessAllowed); }
    void ClearAccessAllowed() { mAccessAllowed = chip::Optional<bool>(); }

    // Getters for required fields
    bool IsEnabled() const { return mEnabled; }
    uint8_t GetTestResult() const { return mTestResult; }
    Result GetTestResultEnum() const { return static_cast<Result>(mTestResult); }
    chip::FabricIndex GetFabricIndex() const { return mFabricIndex; }
    bool IsFabricUnderTest(chip::FabricIndex fabricIndex) { return mFabricIndex == fabricIndex; }

    // Setters for required fields
    void SetEnabled(bool enabled) { mEnabled = enabled; }
    void SetTestResult(uint8_t result) { mTestResult = result; }
    void SetTestResult(Result result) { mTestResult = static_cast<uint8_t>(result); }
    void SetFabricIndex(chip::FabricIndex fabricIndex) { mFabricIndex = fabricIndex; }

    // IP address getters
    chip::ByteSpan GetSourceIpAddress() const { return chip::ByteSpan(mSourceIpAddress, mSourceIpAddressLength); }
    chip::ByteSpan GetDestinationIpAddress() const { return chip::ByteSpan(mDestinationIpAddress, mDestinationIpAddressLength); }
    bool HasSourceIpAddress() const { return mSourceIpAddressLength > 0; }
    bool HasDestinationIpAddress() const { return mDestinationIpAddressLength > 0; }

    /**
     * @brief Set source IP address from Inet::IPAddress.
     * @param ipAddress IPAddress containing the IPv6 address
     */
    void SetSourceIpAddress(const Inet::IPAddress & ipAddress)
    {
        if (ipAddress.IsIPv6())
        {
            uint8_t * p = mSourceIpAddress;
            ipAddress.WriteAddress(p);
            mSourceIpAddressLength = kIPv6AddressLength;
        }
    }

    /**
     * @brief Set destination IP address from Inet::IPAddress.
     * @param ipAddress IPAddress containing the IPv6 address
     */
    void SetDestinationIpAddress(const Inet::IPAddress & ipAddress)
    {
        if (ipAddress.IsIPv6())
        {
            uint8_t * p = mDestinationIpAddress;
            ipAddress.WriteAddress(p);
            mDestinationIpAddressLength = kIPv6AddressLength;
        }
    }

    /**
     * @brief Clear source IP address
     */
    void ClearSourceIpAddress() { mSourceIpAddressLength = 0; }

    /**
     * @brief Clear destination IP address
     */
    void ClearDestinationIpAddress() { mDestinationIpAddressLength = 0; }

    /**
     * @brief Template method to convert stored data to event type
     * @tparam EventType The event type structure (e.g., Groupcast::Events::GroupcastTesting::Type)
     * @param event Output event structure to populate
     */
    template <typename EventType>
    void ToEventType(EventType & event) const
    {
        event.groupcastTestResult = static_cast<decltype(event.groupcastTestResult)>(mTestResult);
        event.fabricIndex         = mFabricIndex;
        event.groupID             = mGroupID;
        event.endpointID          = mEndpointID;
        event.clusterID           = mClusterID;
        event.elementID           = mElementID;
        event.accessAllowed       = mAccessAllowed;

        if (mSourceIpAddressLength > 0)
        {
            event.sourceIpAddress = MakeOptional(chip::ByteSpan(mSourceIpAddress, mSourceIpAddressLength));
        }
        else
        {
            event.sourceIpAddress = chip::Optional<chip::ByteSpan>();
        }

        if (mDestinationIpAddressLength > 0)
        {
            event.destinationIpAddress = MakeOptional(chip::ByteSpan(mDestinationIpAddress, mDestinationIpAddressLength));
        }
        else
        {
            event.destinationIpAddress = chip::Optional<chip::ByteSpan>();
        }
    }

    /**
     * @brief Clear all stored data
     */
    void Clear()
    {
        mSourceIpAddressLength      = 0;
        mDestinationIpAddressLength = 0;
        mGroupID                    = chip::Optional<chip::GroupId>();
        mEndpointID                 = chip::Optional<chip::EndpointId>();
        mClusterID                  = chip::Optional<chip::ClusterId>();
        mElementID                  = chip::Optional<uint32_t>();
        mAccessAllowed              = chip::Optional<bool>();
        mTestResult                 = 0;
    }

private:
    bool mEnabled = false;
    // IP addresses stored as IPv6 address bytes (16 bytes) in network byte order (RFC 4291)
    uint8_t mSourceIpAddress[kIPv6AddressLength];
    uint8_t mDestinationIpAddress[kIPv6AddressLength];
    size_t mSourceIpAddressLength      = 0;
    size_t mDestinationIpAddressLength = 0;
    // Optional fields
    chip::Optional<chip::GroupId> mGroupID;
    chip::Optional<chip::EndpointId> mEndpointID;
    chip::Optional<chip::ClusterId> mClusterID;
    chip::Optional<uint32_t> mElementID;
    chip::Optional<bool> mAccessAllowed;
    // Required fields
    uint8_t mTestResult            = 0;
    chip::FabricIndex mFabricIndex = kUndefinedFabricIndex;
};

} // namespace Groupcast
} // namespace chip
