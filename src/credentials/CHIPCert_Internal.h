/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <credentials/CHIPCert.h>

namespace chip {
namespace Credentials {

// Constants for Network (Client) Identities as per section 11.24 (Wi-Fi
// Authentication with Per-Device Credentials) of the Matter spec.
inline constexpr CharSpan kNetworkIdentityCN           = "*"_span;
inline constexpr ByteSpan kNetworkIdentitySerialNumber = ByteSpan((uint8_t[1]){ 1 });

inline constexpr uint32_t kNetworkIdentityNotBeforeTime = 1;
inline constexpr uint32_t kNetworkIdentityNotAfterTime  = kNullCertTime;

inline constexpr auto kNetworkIdentityKeyUsage = BitFlags<KeyUsageFlags>(KeyUsageFlags::kDigitalSignature);
inline constexpr auto kNetworkIdentityKeyPurpose =
    BitFlags<KeyPurposeFlags>(KeyPurposeFlags::kClientAuth, KeyPurposeFlags::kServerAuth);

} // namespace Credentials
} // namespace chip
