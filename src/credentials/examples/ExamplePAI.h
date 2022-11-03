/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <cstdint>

#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace DevelopmentCerts {

extern const uint8_t kDevelopmentPAI_Cert_FFF1[463];
extern const uint8_t kDevelopmentPAI_PublicKey_FFF1[65];
extern const uint8_t kDevelopmentPAI_PrivateKey_FFF1[32];
extern ByteSpan kPaiCert;
extern ByteSpan kPaiPublicKey;
extern ByteSpan kPaiPrivateKey;

} // namespace DevelopmentCerts
} // namespace chip
