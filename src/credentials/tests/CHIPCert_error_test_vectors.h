/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/Span.h>

namespace chip {
namespace TestCerts {

extern const uint8_t kTestErrorCert_NOC_0001_InvCATVerZero_Cert_Array[275];
extern const uint8_t kTestErrorCert_NOC_0001_InvCATVerZero_PublicKey_Array[65];
extern const uint8_t kTestErrorCert_NOC_0001_InvCATVerZero_PrivateKey_Array[32];

extern ByteSpan kTestErrorCert_NOC_0001_InvCATVerZero_Cert;
extern ByteSpan kTestErrorCert_NOC_0001_InvCATVerZero_PrivateKey;
extern ByteSpan kTestErrorCert_NOC_0001_InvCATVerZero_PublicKey;

extern const uint8_t kTestErrorCert_NOC_0002_InvCATMulVers_Cert_Array[287];
extern const uint8_t kTestErrorCert_NOC_0002_InvCATMulVers_PublicKey_Array[65];
extern const uint8_t kTestErrorCert_NOC_0002_InvCATMulVers_PrivateKey_Array[32];

extern ByteSpan kTestErrorCert_NOC_0002_InvCATMulVers_Cert;
extern ByteSpan kTestErrorCert_NOC_0002_InvCATMulVers_PrivateKey;
extern ByteSpan kTestErrorCert_NOC_0002_InvCATMulVers_PublicKey;

} // namespace TestCerts
} // namespace chip
