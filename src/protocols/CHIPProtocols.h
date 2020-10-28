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

/**
 *   @namespace chip::Protocols
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for CHIP protocols.
 */

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

    kChipProtocol_SecurityChannel     = (kChipVendor_Common << 16) | 0x0001, // Security Channel Protocol
    kChipProtocol_Echo                = (kChipVendor_Common << 16) | 0x0002, // Echo Protocol
    kChipProtocol_BDX                 = (kChipVendor_Common << 16) | 0x0003, // Bulk Data Exchange Protocol
    kChipProtocol_NetworkProvisioning = (kChipVendor_Common << 16) | 0x0004, // Network Provisioning Protocol
    kChipProtocol_InteractionModel    = (kChipVendor_Common << 16) | 0x0005, // Interaction Model Protocol
    kChipProtocol_ServiceProvisioning = (kChipVendor_Common << 16) | 0x0006, // Service Provisioning Protocol

    // Protocols reserved for internal protocol use

    kChipProtocol_NotSpecified = (kChipVendor_NotSpecified << 16) | 0xFFFF, // The profile ID is either not specified or a wildcard
};

} // namespace Protocols
} // namespace chip
