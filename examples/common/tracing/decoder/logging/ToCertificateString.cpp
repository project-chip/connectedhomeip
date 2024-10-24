/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "ToCertificateString.h"

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>

namespace {
constexpr const uint8_t kChipRawPrefix[] = { 0x15, 0x30, 0x01 };

bool IsChipCertificate(const chip::ByteSpan & source)
{
    return (source.size() > sizeof(kChipRawPrefix)) && (memcmp(source.data(), kChipRawPrefix, sizeof(kChipRawPrefix)) == 0);
}

const char * ToCertificate(const chip::ByteSpan & source, chip::MutableCharSpan destination, const char * header = "",
                           const char * footer = "")
{
    // Reset the buffer
    memset(destination.data(), '\0', destination.size());
    int snprintf_len = 0;
    if (source.size() == 0)
    {
        return destination.data();
    }

    if (!chip::CanCastTo<uint16_t>(source.size()))
    {
        ChipLogError(DataManagement, "The certificate is too large to do base64 conversion on");
        return destination.data();
    }

    size_t base64DataLen = BASE64_ENCODED_LEN(source.size());
    size_t bufferLen     = base64DataLen + 1; // add one character for null-terminator
    if (bufferLen + strlen(header) + strlen(footer) > destination.size())
    {
        ChipLogError(DataManagement, "The certificate buffer is too small to hold the base64 encoded certificate");
        return destination.data();
    }

    chip::Platform::ScopedMemoryBuffer<char> str;
    str.Alloc(bufferLen);

    auto encodedLen       = chip::Base64Encode(source.data(), static_cast<uint16_t>(source.size()), str.Get());
    str.Get()[encodedLen] = '\0';

    if (IsChipCertificate(source))
    {
        // Wherever Matter Operational Certificate Encoding representation is used, all certificates SHALL NOT be longer than 400
        // bytes in their TLV form.
        if (source.size() > 400)
        {
            ChipLogError(DataManagement, "Certificate size is greater than 400 bytes");
        }

        snprintf_len = snprintf(destination.data(), destination.size(), "%s", str.Get());
        VerifyOrExit(snprintf_len >= 0, ChipLogError(DataManagement, "Failed to write certificate"););
    }
    else
    {
        // All certificates SHALL NOT be longer than 600 bytes in their uncompressed DER format.
        if (source.size() > 600)
        {
            ChipLogError(DataManagement, "Certificate size is greater than 600 bytes");
        }

        size_t inIndex  = 0;
        size_t outIndex = strlen(header) + 1;

        snprintf_len = snprintf(destination.data(), destination.size(), "%s\n", header);
        VerifyOrExit(snprintf_len >= 0, ChipLogError(DataManagement, "Failed to write header"););
        for (; inIndex < base64DataLen; inIndex += 64)
        {
            snprintf_len = snprintf(&destination.data()[outIndex], destination.size() - outIndex, "%.64s\n", &str[inIndex]);
            VerifyOrExit(snprintf_len >= 0, ChipLogError(DataManagement, "Failed to write certificate"););

            outIndex += static_cast<size_t>(snprintf_len);
        }
        snprintf_len = snprintf(&destination.data()[outIndex], destination.size() - outIndex, "%s", footer);
        VerifyOrExit(snprintf_len >= 0, ChipLogError(DataManagement, "Failed to write footer"););
    }
exit:
    if (snprintf_len < 0)
    {
        memset(destination.data(), '\0', destination.size());
    }
    return destination.data();
}

} // namespace

namespace chip {
namespace trace {
namespace logging {

const char * ToCertificateString(const ByteSpan & source, MutableCharSpan destination)
{
    static constexpr char kCertificateHeader[] = "-----BEGIN CERTIFICATE-----";
    static constexpr char kCertificateFooter[] = "-----END CERTIFICATE-----";

    return ToCertificate(source, destination, kCertificateHeader, kCertificateFooter);
}

const char * ToCertificateRequestString(const ByteSpan & source, MutableCharSpan destination)
{
    static constexpr char kCertificateHeader[] = "-----BEGIN CERTIFICATE REQUEST-----";
    static constexpr char kCertificateFooter[] = "-----END CERTIFICATE REQUEST-----";

    return ToCertificate(source, destination, kCertificateHeader, kCertificateFooter);
}

} // namespace logging
} // namespace trace
} // namespace chip
