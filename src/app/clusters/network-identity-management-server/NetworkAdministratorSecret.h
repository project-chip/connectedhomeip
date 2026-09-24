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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

namespace chip::Crypto {

// Length of the raw key material in a Network Administrator Shared Secret.
// See Matter Specification section "Network Administrator Shared Secret".
inline constexpr size_t kNetworkAdministratorRawSecretLength = 32;

using NetworkAdministratorRawSecret = SensitiveDataFixedBuffer<kNetworkAdministratorRawSecretLength>;

// Maximum size of a TLV-encoded Network Administrator Shared Secret.
inline constexpr size_t kNetworkAdministratorSecretDataMaxEncodedLength =
    TLV::EstimateStructOverhead(sizeof(uint8_t),                       // version
                                sizeof(uint32_t),                      // created
                                kNetworkAdministratorRawSecretLength); // raw-secret

/**
 * Decoded representation of the network-administrator-shared-secret TLV structure.
 * See Matter Specification section "Network Administrator Shared Secret".
 */
struct NetworkAdministratorSecretData
{
    System::Clock::Seconds32 created{};        // Matter Epoch Seconds (spec field: created [2])
    NetworkAdministratorRawSecret rawSecret{}; // spec field: raw-secret [3]
};

/**
 * Encodes a NetworkAdministratorSecretData to the spec-defined TLV format.
 */
CHIP_ERROR EncodeNetworkAdministratorSecret(const NetworkAdministratorSecretData & secretData, MutableByteSpan & outEncoded);

/**
 * Decodes a NetworkAdministratorSecretData from the spec-defined TLV format.
 */
CHIP_ERROR DecodeNetworkAdministratorSecret(const ByteSpan & encoded, NetworkAdministratorSecretData & outSecretData);

} // namespace chip::Crypto
