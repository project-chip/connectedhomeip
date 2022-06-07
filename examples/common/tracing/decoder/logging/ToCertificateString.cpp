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

    if (source.size() == 0)
    {
        return destination.data();
    }

    size_t base64DataLen = BASE64_ENCODED_LEN(source.size()) + 1;
    if (base64DataLen + strlen(header) + strlen(footer) > destination.size())
    {
        ChipLogError(DataManagement, "The certificate buffer is too small to hold the base64 encoded certificate");
        return destination.data();
    }

    chip::Platform::ScopedMemoryBuffer<char> str;
    str.Alloc(base64DataLen);

    auto encodedLen       = chip::Base64Encode(source.data(), source.size(), str.Get());
    str.Get()[encodedLen] = '\0';

    if (IsChipCertificate(source))
    {
        // Wherever Matter Operational Certificate Encoding representation is used, all certificates SHALL NOT be longer than 400
        // bytes in their TLV form.
        if (source.size() > 400)
        {
            ChipLogError(DataManagement, "Certificate size is greater than 400 bytes");
        }

        snprintf(destination.data(), destination.size(), "%s", str.Get());
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

        snprintf(destination.data(), destination.size(), "%s\n", header);
        for (; inIndex < base64DataLen; inIndex += 64)
        {
            outIndex += snprintf(&destination.data()[outIndex], destination.size() - outIndex, "%.64s\n", &str[inIndex]);
        }
        snprintf(&destination.data()[outIndex], destination.size() - outIndex, "%s", footer);
    }

    return destination.data();
}

} // namespace

namespace chip {
namespace trace {
namespace logging {

const char * ToCertificateString(const ByteSpan & source, MutableCharSpan destination)
{
    constexpr const char * kCertificateHeader = "-----BEGIN CERTIFICATE-----";
    constexpr const char * kCertificateFooter = "-----END CERTIFICATE-----";

    return ToCertificate(source, destination, kCertificateHeader, kCertificateFooter);
}

const char * ToCertificateRequestString(const ByteSpan & source, MutableCharSpan destination)
{
    constexpr const char * kCertificateHeader = "-----BEGIN CERTIFICATE REQUEST-----";
    constexpr const char * kCertificateFooter = "-----END CERTIFICATE REQUEST-----";

    return ToCertificate(source, destination, kCertificateHeader, kCertificateFooter);
}

} // namespace logging
} // namespace trace
} // namespace chip
