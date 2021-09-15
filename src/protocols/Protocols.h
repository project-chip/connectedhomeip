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

/**
 *    @file
 *      This file defines constant enumerations for all public (or
 *      common) protocols.
 *
 */

#pragma once

#include <lib/core/CHIPVendorIdentifiers.hpp>

namespace chip {
namespace Protocols {

//
// CHIP Protocol Ids (32-bits max)
//
class Id
{
public:
    constexpr Id(VendorId aVendorId, uint16_t aProtocolId) : mVendorId(aVendorId), mProtocolId(aProtocolId) {}

    constexpr bool operator==(const Id & aOther) const
    {
        return mVendorId == aOther.mVendorId && mProtocolId == aOther.mProtocolId;
    }

    constexpr bool operator!=(const Id & aOther) const { return !(*this == aOther); }

    // Convert the Protocols::Id to a TLV profile id.
    // NOTE: We may want to change the TLV reader/writer to take Protocols::Id
    // directly later on and get rid of this method.
    constexpr uint32_t ToTLVProfileId() const { return ToUint32(); }

    // Convert the protocol id to a 32-bit unsigned integer "fully qualified"
    // form as defined in the spec.  This should only be used in the places
    // where the spec defines a 32-bit unsigned in as a wire representation of
    // protocol id.
    constexpr uint32_t ToFullyQualifiedSpecForm() const { return ToUint32(); }

    constexpr VendorId GetVendorId() const { return mVendorId; }
    constexpr uint16_t GetProtocolId() const { return mProtocolId; }

private:
    constexpr uint32_t ToUint32() const { return (static_cast<uint32_t>(mVendorId) << 16) | mProtocolId; }

    chip::VendorId mVendorId;
    uint16_t mProtocolId;
};

// Common Protocols
//
// NOTE: Do not attempt to allocate these values yourself.
#define CHIP_STANDARD_PROTOCOL(name, id)                                                                                           \
    namespace name {                                                                                                               \
    static constexpr Protocols::Id Id(VendorId::Common, id);                                                                       \
    } // namespace name.

CHIP_STANDARD_PROTOCOL(SecureChannel, 0x0000)             // Secure Channel Protocol
CHIP_STANDARD_PROTOCOL(Echo, 0x0001)                      // Echo Protocol
CHIP_STANDARD_PROTOCOL(BDX, 0x0002)                       // Bulk Data Exchange Protocol
CHIP_STANDARD_PROTOCOL(UserDirectedCommissioning, 0x0003) // User Directed Commissioning Protocol
CHIP_STANDARD_PROTOCOL(InteractionModel, 0x0005)          // Interaction Model Protocol
CHIP_STANDARD_PROTOCOL(ServiceProvisioning, 0x0007)       // Service Provisioning Protocol
CHIP_STANDARD_PROTOCOL(OpCredentials, 0x0008)             // Operational Credentials
CHIP_STANDARD_PROTOCOL(TempZCL, 0x0009) // For carrying ZCL-compatible messages, to be removed when we no longer use them

#undef CHIP_STANDARD_PROTOCOL

// Protocols reserved for internal protocol use
static constexpr Id NotSpecified(VendorId::NotSpecified, 0xFFFF); // The profile ID is either not specified or a wildcard

// Pre-delare our MessageTypeTraits so message type headers know what they are
// specializing.
template <typename T>
struct MessageTypeTraits;

} // namespace Protocols
} // namespace chip
