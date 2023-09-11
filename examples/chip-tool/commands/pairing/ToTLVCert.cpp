/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "ToTLVCert.h"

#include <credentials/CHIPCert.h>
#include <lib/support/Base64.h>

constexpr const char kBase64Header[] = "base64:";
constexpr size_t kBase64HeaderLen    = ArraySize(kBase64Header) - 1;

CHIP_ERROR ToBase64(const chip::ByteSpan & input, std::string & outputAsPrefixedBase64)
{
    chip::Platform::ScopedMemoryBuffer<char> base64String;
    base64String.Alloc(kBase64HeaderLen + BASE64_ENCODED_LEN(input.size()) + 1);
    VerifyOrReturnError(base64String.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

    auto encodedLen = chip::Base64Encode(input.data(), static_cast<uint16_t>(input.size()), base64String.Get() + kBase64HeaderLen);
    if (encodedLen)
    {
        memcpy(base64String.Get(), kBase64Header, kBase64HeaderLen);
        encodedLen = static_cast<uint16_t>(encodedLen + kBase64HeaderLen);
    }
    base64String.Get()[encodedLen] = '\0';
    outputAsPrefixedBase64         = std::string(base64String.Get(), encodedLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ToTLVCert(const chip::ByteSpan & derEncodedCertificate, std::string & tlvCertAsPrefixedBase64)
{
    uint8_t chipCertBuffer[chip::Credentials::kMaxCHIPCertLength];
    chip::MutableByteSpan chipCertBytes(chipCertBuffer);
    ReturnErrorOnFailure(chip::Credentials::ConvertX509CertToChipCert(derEncodedCertificate, chipCertBytes));
    ReturnErrorOnFailure(ToBase64(chipCertBytes, tlvCertAsPrefixedBase64));
    return CHIP_NO_ERROR;
}
