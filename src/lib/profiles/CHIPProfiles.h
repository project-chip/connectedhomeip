/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      common) profiles.
 *
 */

#ifndef CHIP_PROFILES_H_
#define CHIP_PROFILES_H_

#include <core/CHIPVendorIdentifiers.hpp>

/**
 *   @namespace chip::Profiles
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for CHIP profiles.
 */

namespace chip {
namespace Profiles {

//
// CHIP Profile Ids (32-bits max)
//

enum CHIPProfileId
{
    // Common Profiles
    //
    // NOTE: Do not attempt to allocate these values yourself.

    kChipProfile_Common              = (kChipVendor_Common << 16) | 0x0000, // Common Profile
    kChipProfile_Echo                = (kChipVendor_Common << 16) | 0x0001, // Echo Profile
    kChipProfile_NetworkProvisioning = (kChipVendor_Common << 16) | 0x0003, // Network Provisioning Profile
    kChipProfile_Security            = (kChipVendor_Common << 16) | 0x0004, // Network Security Profile
    kChipProfile_FabricProvisioning  = (kChipVendor_Common << 16) | 0x0005, // Fabric Provisioning Profile
    kChipProfile_ServiceProvisioning = (kChipVendor_Common << 16) | 0x000F, // Service Provisioning Profile

    // Profiles reserved for internal protocol use

    kChipProfile_NotSpecified = (kChipVendor_NotSpecified << 16) | 0xFFFF, // The profile ID is either not specified or a wildcard
};

} // namespace Profiles
} // namespace chip

#endif /* CHIP_PROFILES_H_ */
