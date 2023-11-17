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

// The decode buffer is used to reconstruct TBS section of X.509 certificate, which doesn't include signature.
inline constexpr size_t kMaxCHIPCertDecodeBufLength = kMaxDERCertLength - Crypto::kMax_ECDSA_Signature_Length_Der;

// The TBSCerticate of a Network (Client) Identity has a fixed (smaller) size.
inline constexpr size_t kNetworkIdentityTBSLength = 244;

// Constants for Network (Client) Identities as per section 11.24 (Wi-Fi
// Authentication with Per-Device Credentials) of the Matter spec.
inline constexpr CharSpan kNetworkIdentityCN                = "*"_span;
inline constexpr uint8_t kNetworkIdentitySerialNumber       = 1;
inline constexpr ByteSpan kNetworkIdentitySerialNumberBytes = ByteSpan((uint8_t[1]){ kNetworkIdentitySerialNumber });

inline constexpr uint32_t kNetworkIdentityNotBeforeTime = 1;
inline constexpr uint32_t kNetworkIdentityNotAfterTime  = kNullCertTime;

inline constexpr auto kNetworkIdentityKeyUsage = BitFlags<KeyUsageFlags>(KeyUsageFlags::kDigitalSignature);
inline constexpr auto kNetworkIdentityKeyPurpose =
    BitFlags<KeyPurposeFlags>(KeyPurposeFlags::kClientAuth, KeyPurposeFlags::kServerAuth);

// Initializes a ChipDN as CN=kNetworkIdentityCN
void InitNetworkIdentitySubject(ChipDN & name);

// Emits a X.509 TBSCertificate for a Network (Client) Identity based on the specified key.
CHIP_ERROR EncodeNetworkIdentityTBSCert(const Crypto::P256PublicKey & pubkey, ASN1::ASN1Writer & writer);

} // namespace Credentials
} // namespace chip
