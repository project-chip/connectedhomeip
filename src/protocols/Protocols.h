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

#include <core/CHIPVendorIdentifiers.hpp>

namespace chip {
namespace Protocols {

//
// CHIP Protocol Ids (32-bits max)
//

enum CHIPProtocolId
{
    // Common Protocols
    //
    // NOTE: Do not attempt to allocate these values yourself.

    kProtocol_Protocol_Common     = (kChipVendor_Common << 16) | 0x0000, // Common Protocol
    kProtocol_SecureChannel       = (kChipVendor_Common << 16) | 0x0001, // Secure Channel Protocol
    kProtocol_Echo                = (kChipVendor_Common << 16) | 0x0002, // Echo Protocol
    kProtocol_BDX                 = (kChipVendor_Common << 16) | 0x0003, // Bulk Data Exchange Protocol
    kProtocol_NetworkProvisioning = (kChipVendor_Common << 16) | 0x0004, // Network Provisioning Protocol
    kProtocol_InteractionModel    = (kChipVendor_Common << 16) | 0x0005, // Interaction Model Protocol
    kProtocol_FabricProvisioning  = (kChipVendor_Common << 16) | 0x0006, // Fabric Provisioning Protocol
    kProtocol_ServiceProvisioning = (kChipVendor_Common << 16) | 0x0007, // Service Provisioning Protocol
    kProtocol_OpCredentials       = (kChipVendor_Common << 16) | 0x0008, // Operational Credentials

    // Protocols reserved for internal protocol use

    kProtocol_NotSpecified = (kChipVendor_NotSpecified << 16) | 0xFFFF, // The profile ID is either not specified or a wildcard
};

// Pre-delare our MessageTypeTraits so message type headers know what they are
// specializing.
template <typename T>
struct MessageTypeTraits;

} // namespace Protocols
} // namespace chip
