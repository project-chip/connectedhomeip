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
#include <lib/support/TypeTraits.h>

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

    static constexpr uint32_t kVendorIdShift = 16;

    static Id FromFullyQualifiedSpecForm(uint32_t aSpecForm)
    {
        return Id(static_cast<VendorId>(aSpecForm >> kVendorIdShift),
                  static_cast<uint16_t>(aSpecForm & ((1 << kVendorIdShift) - 1)));
    }

private:
    constexpr uint32_t ToUint32() const { return (static_cast<uint32_t>(mVendorId) << kVendorIdShift) | mProtocolId; }

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
CHIP_STANDARD_PROTOCOL(InteractionModel, 0x0001)          // Interaction Model Protocol
CHIP_STANDARD_PROTOCOL(BDX, 0x0002)                       // Bulk Data Exchange Protocol
CHIP_STANDARD_PROTOCOL(UserDirectedCommissioning, 0x0003) // User Directed Commissioning Protocol
CHIP_STANDARD_PROTOCOL(Echo, 0x0004)                      // Echo Protocol.  To be removed or standardized.

#undef CHIP_STANDARD_PROTOCOL

// Protocols reserved for internal protocol use
static constexpr Id NotSpecified(VendorId::NotSpecified, 0xFFFF); // The profile ID is either not specified or a wildcard

//
// Pre-declaration of type traits that protocol headers are expected to define.
//
// A protocol header should first define an enumeration that lists the various message types as enum classes.
//
// It should then define MessageTypeTraits as a template specialization of that enumeration containing two methods:
//      1. static constexpr const Protocols::Id & ProtocolId() that returns the Protocol ID
//      2. static auto GetTypeToNameTable() that returns a pointer to std::array<MessageTypeNameLookup, N> where N = number of
//      messages in protocol.
//
template <typename T>
struct MessageTypeTraits;

//
// Encapsulates a tuple of message type ID and its associated name.
//
struct MessageTypeNameLookup
{
    //
    // Constructor that takes an enumeration value for a specific message ID and its associated name.
    //
    template <typename T>
    constexpr MessageTypeNameLookup(T id, const char * name) : mId(to_underlying(id)), mName(name)
    {}

    const uint8_t mId;
    const char * mName;
};

//
// Given a protocol ID and a message type ID, retrieve the logical name of that message.
//
// This will not return a nullptr.
//
const char * GetMessageTypeName(Id protocolId, uint8_t msgType);

//
// Given a protool ID, retrieve the logical name for that protocol.
//
// This will not return a nullptr.
//
const char * GetProtocolName(Id protocolId);

} // namespace Protocols
} // namespace chip
