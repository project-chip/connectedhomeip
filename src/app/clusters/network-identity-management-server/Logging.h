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

#include <credentials/CHIPCert.h>
#include <lib/core/CHIPEncoding.h>

/**
 * @brief Logging format macro for CertificateKeyId.
 *
 * Usage:
 *   ChipLogProgress(Zcl, "Identifier: " ChipLogFormatKeyId, ChipLogValueKeyId(id));
 */
#define ChipLogFormatKeyId "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32

/**
 * @brief Logging value macro for CertificateKeyId.
 * Takes a CertificateKeyId (FixedByteSpan<20>) or a value implicitly convertible to it,
 * e.g. a uint8_t[20] or std::array<uint8_t, 20>.
 * NOTE: The argument to ChipLogValueKeyId may be evaluated multiple times.
 */
#define ChipLogValueKeyId(id)                                                                                                      \
    chip::Encoding::BigEndian::Get32(chip::Credentials::CertificateKeyId(id).data()),                                              \
        chip::Encoding::BigEndian::Get32(chip::Credentials::CertificateKeyId(id).data() + 4),                                      \
        chip::Encoding::BigEndian::Get32(chip::Credentials::CertificateKeyId(id).data() + 8),                                      \
        chip::Encoding::BigEndian::Get32(chip::Credentials::CertificateKeyId(id).data() + 12),                                     \
        chip::Encoding::BigEndian::Get32(chip::Credentials::CertificateKeyId(id).data() + 16)

static_assert(chip::Credentials::CertificateKeyId::size() == 20); // hard-coded in ChipLogValueKeyId
