/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
