/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

/**
 *  @file Implements message encryption and decryption
 */

#include "MessageSecurity.h"
#include <credentials/GroupDataProvider.h>
#include <lib/support/SafeInt.h>

using namespace chip::Credentials;

namespace chip {

namespace MessageSecurity {

constexpr size_t kAESCCMIVLen = 13;
constexpr size_t kMaxAADLen   = 128;

CHIP_ERROR GetIV(const chip::PacketHeader & header, uint8_t * iv, size_t len)
{
    VerifyOrReturnError(len == kAESCCMIVLen, CHIP_ERROR_INVALID_ARGUMENT);
    Encoding::LittleEndian::BufferWriter bbuf(iv, len);

    bbuf.Put8(header.GetSecurityFlags());
    bbuf.Put32(header.GetMessageCounter());
    bbuf.Put64(header.GetSourceNodeId().ValueOr(0));
    return bbuf.Fit() ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR GetAdditionalAuthData(const chip::PacketHeader & header, uint8_t * aad, uint16_t & len)
{
    VerifyOrReturnError(len >= header.EncodeSizeBytes(), CHIP_ERROR_INVALID_ARGUMENT);

    // Use unencrypted part of header as AAD. This will help
    // integrity protect the whole message
    uint16_t actualEncodedHeaderSize;

    ReturnErrorOnFailure(header.Encode(aad, len, &actualEncodedHeaderSize));
    VerifyOrReturnError(len >= actualEncodedHeaderSize, CHIP_ERROR_INVALID_ARGUMENT);

    len = actualEncodedHeaderSize;

    return CHIP_NO_ERROR;
}

CHIP_ERROR Encrypt(const ByteSpan & encryptionKey, chip::PayloadHeader & payloadHeader, chip::PacketHeader & packetHeader,
                   chip::System::PacketBufferHandle & msgBuf)
{
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!msgBuf->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrReturnError(msgBuf->TotalLength() <= chip::kMaxAppMessageLen, CHIP_ERROR_MESSAGE_TOO_LONG);

    static_assert(std::is_same<decltype(msgBuf->TotalLength()), uint16_t>::value,
                  "Addition to generate payloadLength might overflow");

    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(msgBuf));

    uint8_t * data    = msgBuf->Start();
    uint16_t totalLen = msgBuf->TotalLength();
    chip::MessageAuthenticationCode mac;

    {
        const uint8_t * input = data;
        size_t input_length   = totalLen;
        uint8_t * output      = data;
        PacketHeader & header = packetHeader;
        const size_t taglen   = packetHeader.MICTagLength();

        VerifyOrDie(taglen <= kMaxTagLen);
        VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        uint8_t AAD[kMaxAADLen];
        uint8_t IV[kAESCCMIVLen];
        uint16_t aadLen = sizeof(AAD);
        uint8_t tag[kMaxTagLen];

        ReturnErrorOnFailure(GetIV(header, IV, sizeof(IV)));
        ReturnErrorOnFailure(GetAdditionalAuthData(header, AAD, aadLen));
        ReturnErrorOnFailure(Crypto::AES_CCM_encrypt(input, input_length, AAD, aadLen, encryptionKey.data(),
                                                     Crypto::kAES_CCM128_Key_Length, IV, sizeof(IV), output, tag, taglen));

        mac.SetTag(&header, tag, taglen);
    }

    uint16_t taglen = 0;
    ReturnErrorOnFailure(mac.Encode(packetHeader, &data[totalLen], msgBuf->AvailableDataLength(), &taglen));
    VerifyOrReturnError(chip::CanCastTo<uint16_t>(totalLen + taglen), CHIP_ERROR_INTERNAL);
    msgBuf->SetDataLength(static_cast<uint16_t>(totalLen + taglen));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Decrypt(const ByteSpan & encryptionKey, chip::PayloadHeader & payloadHeader, const chip::PacketHeader & packetHeader,
                   chip::System::PacketBufferHandle & msg)
{
    ReturnErrorCodeIf(msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t * data = msg->Start();
    uint16_t len   = msg->DataLength();

    chip::System::PacketBufferHandle origMsg;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /* This is a workaround for the case where PacketBuffer payload is not
        allocated as an inline buffer to PacketBuffer structure */
    origMsg = std::move(msg);
    msg     = PacketBufferHandle::New(len);
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);
    msg->SetDataLength(len);
#endif

    uint16_t footerLen = packetHeader.MICTagLength();
    VerifyOrReturnError(footerLen <= len, CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    uint16_t taglen = 0;
    chip::MessageAuthenticationCode mac;
    ReturnErrorOnFailure(mac.Decode(packetHeader, &data[len - footerLen], footerLen, &taglen));
    VerifyOrReturnError(taglen == footerLen, CHIP_ERROR_INTERNAL);

    len = static_cast<uint16_t>(len - taglen);
    msg->SetDataLength(len);

    uint8_t * plainText = msg->Start();
    {
        const uint8_t * input       = data;
        size_t input_length         = len;
        uint8_t * output            = plainText;
        const PacketHeader & header = packetHeader;

        const size_t taglen2 = header.MICTagLength();
        const uint8_t * tag  = mac.GetTag();
        uint8_t IV[kAESCCMIVLen];
        uint8_t AAD[kMaxAADLen];
        uint16_t aadLen = sizeof(AAD);

        VerifyOrReturnError(input != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(input_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        ReturnErrorOnFailure(GetIV(header, IV, sizeof(IV)));
        ReturnErrorOnFailure(GetAdditionalAuthData(header, AAD, aadLen));

        ReturnErrorOnFailure(Crypto::AES_CCM_decrypt(input, input_length, AAD, aadLen, tag, taglen2, encryptionKey.data(),
                                                     Crypto::kAES_CCM128_Key_Length, IV, sizeof(IV), output));
    }

    ReturnErrorOnFailure(payloadHeader.DecodeAndConsume(msg));
    return CHIP_NO_ERROR;
}

namespace Groups {

/* Operational Group Key Group, Security Salt: "GroupKey v1.0" */
static const uint8_t kGroupSecuritySalt[]        = { 0x47, 0x72, 0x6f, 0x75, 0x70, 0x4b, 0x65, 0x79, 0x20, 0x76, 0x31, 0x2e, 0x30 };
static const uint8_t kOperationalGroupKeySalt[0] = {};

/* Group Key Derivation Function, Info: "GroupKeyHash" ” */
static const uint8_t kGroupKeyHashInfo[]  = { 0x47, 0x72, 0x6f, 0x75, 0x70, 0x4b, 0x65, 0x79, 0x48, 0x61, 0x73, 0x68 };
static const uint8_t kGroupKeyHashSalt[0] = {};

CHIP_ERROR DeriveOperationalKey(chip::FabricIndex fabric_index, chip::GroupId group_id, OperationKeyType out_key)
{
    auto * groups = GetGroupDataProvider();
    VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INTERNAL);

    auto * iter = groups->IterateGroupKeys(fabric_index);
    VerifyOrReturnError(nullptr != iter, CHIP_ERROR_NO_MEMORY);

    // Search for the keyset of the given group
    GroupDataProvider::GroupKey mapping;
    GroupDataProvider::KeySet keyset;
    ByteSpan epoch_key;

    while (epoch_key.empty() && iter->Next(mapping))
    {
        if (mapping.group_id == group_id)
        {
            ReturnErrorOnFailure(groups->GetKeySet(fabric_index, mapping.keyset_id, keyset));
            epoch_key = keyset.GetCurrentKey();
        }
    }
    iter->Release();
    VerifyOrReturnError(!epoch_key.empty(), CHIP_ERROR_NOT_FOUND);

    // Keyset found for the given fabric and group
    return DeriveOperationalKey(epoch_key, out_key);
}

CHIP_ERROR DeriveOperationalKey(const chip::ByteSpan & epoch_key, OperationKeyType out_key)
{
    VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == epoch_key.size(), CHIP_ERROR_INTERNAL);

    chip::Crypto::HKDF_sha crypto;
    return crypto.HKDF_SHA256(epoch_key.data(), epoch_key.size(), kOperationalGroupKeySalt, sizeof(kOperationalGroupKeySalt),
                              kGroupSecuritySalt, sizeof(kGroupSecuritySalt), out_key,
                              Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
}

CHIP_ERROR DeriveSessionId(const chip::ByteSpan & group_key, uint16_t & session_id)
{
    chip::Crypto::HKDF_sha context;
    uint8_t out_key[2];

    ReturnErrorOnFailure(context.HKDF_SHA256(group_key.data(), group_key.size(), kGroupKeyHashSalt, sizeof(kGroupKeyHashSalt),
                                             kGroupKeyHashInfo, sizeof(kGroupKeyHashInfo), out_key, sizeof(out_key)));
    session_id = static_cast<uint16_t>(out_key[0] | (out_key[1] << 8));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeriveSessionId(const OperationKeyType & group_key, uint16_t & session_id)
{
    return DeriveSessionId(ByteSpan(group_key, sizeof(group_key)), session_id);
}

} // namespace Groups

} // namespace MessageSecurity

} // namespace chip
