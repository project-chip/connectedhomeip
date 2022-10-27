/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *    @file
 *      This file implements utility functions for reading, parsing,
 *      encoding, and decoding CHIP key material.
 *
 */

#include "chip-cert.h"
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <protocols/Protocols.h>

using namespace chip;
using namespace chip::Encoding;
using namespace chip::Protocols;
using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::Encoding;

namespace {

KeyFormat DetectKeyFormat(const uint8_t * key, uint32_t keyLen)
{
    static uint32_t p256SerializedKeypairLen = kP256_PublicKey_Length + kP256_PrivateKey_Length;
    static const uint8_t chipRawPrefix[]     = { 0x04 };
    static const char * chipHexPrefix        = "04";
    static const char * chipB64Prefix        = "B";
    static const uint8_t derRawPrefix[]      = { 0x30, 0x77, 0x02, 0x01, 0x01, 0x04 };
    static const char * derHexPrefix         = "307702010104";
    static const char * ecPEMMarker          = "-----BEGIN EC PRIVATE KEY-----";
    static const char * pkcs8PEMMarker       = "-----BEGIN PRIVATE KEY-----";
    static const char * ecPUBPEMMarker       = "-----BEGIN PUBLIC KEY-----";

    VerifyOrReturnError(key != nullptr, kKeyFormat_Unknown);

    if ((keyLen == p256SerializedKeypairLen) && (memcmp(key, chipRawPrefix, sizeof(chipRawPrefix)) == 0))
    {
        return kKeyFormat_Chip_Raw;
    }
    if ((keyLen == HEX_ENCODED_LENGTH(p256SerializedKeypairLen)) && (memcmp(key, chipHexPrefix, strlen(chipHexPrefix)) == 0))
    {
        return kKeyFormat_Chip_Hex;
    }
    if ((keyLen == BASE64_ENCODED_LEN(p256SerializedKeypairLen)) && (memcmp(key, chipB64Prefix, strlen(chipB64Prefix)) == 0))
    {
        return kKeyFormat_Chip_Base64;
    }
    if ((keyLen == kP256_PublicKey_Length) && (memcmp(key, chipRawPrefix, sizeof(chipRawPrefix)) == 0))
    {
        return kKeyFormat_Chip_Pubkey_Raw;
    }
    if ((keyLen == (2 * kP256_PublicKey_Length)) && (memcmp(key, chipHexPrefix, strlen(chipHexPrefix)) == 0))
    {
        return kKeyFormat_Chip_Pubkey_Hex;
    }
    if ((keyLen == BASE64_ENCODED_LEN(kP256_PublicKey_Length)) && (memcmp(key, chipB64Prefix, strlen(chipB64Prefix)) == 0))
    {
        return kKeyFormat_Chip_Pubkey_Base64;
    }
    if ((keyLen > sizeof(derRawPrefix)) && (memcmp(key, derRawPrefix, sizeof(derRawPrefix)) == 0))
    {
        return kKeyFormat_X509_DER;
    }
    if ((keyLen > strlen(derHexPrefix)) && (memcmp(key, derHexPrefix, strlen(derHexPrefix)) == 0))
    {
        return kKeyFormat_X509_Hex;
    }
    if (ContainsPEMMarker(ecPEMMarker, key, keyLen) || ContainsPEMMarker(pkcs8PEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_PEM;
    }
    if (ContainsPEMMarker(ecPUBPEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_Pubkey_PEM;
    }

    return kKeyFormat_Unknown;
}

bool SetPublicKey(const uint8_t * pubkey, uint32_t pubkeyLen, EVP_PKEY * key)
{
    std::unique_ptr<EC_GROUP, void (*)(EC_GROUP *)> group(EC_GROUP_new_by_curve_name(gNIDChipCurveP256), &EC_GROUP_free);
    std::unique_ptr<EC_POINT, void (*)(EC_POINT *)> ecPoint(EC_POINT_new(group.get()), &EC_POINT_free);
    std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ecKey(EC_KEY_new_by_curve_name(gNIDChipCurveP256), &EC_KEY_free);

    VerifyOrReturnError(EC_POINT_oct2point(group.get(), ecPoint.get(), pubkey, pubkeyLen, nullptr) == 1, false);

    VerifyOrReturnError(EC_KEY_set_public_key(ecKey.get(), ecPoint.get()) == 1, false);

    VerifyOrReturnError(EVP_PKEY_set1_EC_KEY(key, ecKey.get()) == 1, false);

    return true;
}

bool ExtractPublicKey(EVP_PKEY * key, MutableByteSpan & pubKey)
{
    const EC_KEY * ecKey     = nullptr;
    const EC_GROUP * group   = nullptr;
    const EC_POINT * ecPoint = nullptr;

    VerifyOrReturnError(pubKey.size() >= kP256_PublicKey_Length, false);

    ecKey = EVP_PKEY_get1_EC_KEY(key);
    VerifyOrReturnError(ecKey != nullptr, false);

    group = EC_KEY_get0_group(ecKey);
    VerifyOrReturnError(group != nullptr, false);

    ecPoint = EC_KEY_get0_public_key(ecKey);
    VerifyOrReturnError(ecPoint != nullptr, false);

    VerifyOrReturnError(EC_POINT_point2oct(group, ecPoint, POINT_CONVERSION_UNCOMPRESSED, Uint8::to_uchar(pubKey.data()),
                                           kP256_PublicKey_Length, nullptr) == kP256_PublicKey_Length,
                        false);

    pubKey.reduce_size(static_cast<size_t>(kP256_PublicKey_Length));

    return true;
}

bool DeserializeKeyPair(const uint8_t * keyPair, uint32_t keyPairLen, EVP_PKEY * key)
{
    std::unique_ptr<BIGNUM, void (*)(BIGNUM *)> privKeyBN(BN_new(), &BN_clear_free);

    ERR_clear_error();

    VerifyOrReturnError(key != nullptr, false);
    VerifyOrReturnError(keyPair != nullptr, false);
    VerifyOrReturnError(keyPairLen == kP256_PublicKey_Length + kP256_PrivateKey_Length, false);

    VerifyOrReturnError(SetPublicKey(keyPair, kP256_PublicKey_Length, key), false);

    VerifyOrReturnError(BN_bin2bn(keyPair + kP256_PublicKey_Length, kP256_PrivateKey_Length, privKeyBN.get()) != nullptr, false);

    VerifyOrReturnError(EC_KEY_set_private_key(EVP_PKEY_get1_EC_KEY(key), privKeyBN.get()) == 1, false);

    return true;
}

} // namespace

bool SerializeKeyPair(EVP_PKEY * key, P256SerializedKeypair & serializedKeypair)
{
    const EC_KEY * ecKey     = nullptr;
    const BIGNUM * privKeyBN = nullptr;
    MutableByteSpan pubKey(serializedKeypair.Bytes(), kP256_PublicKey_Length);

    VerifyOrReturnError(ExtractPublicKey(key, pubKey), false);

    ecKey = EVP_PKEY_get1_EC_KEY(key);
    VerifyOrReturnError(ecKey != nullptr, false);

    privKeyBN = EC_KEY_get0_private_key(ecKey);
    VerifyOrReturnError(privKeyBN != nullptr, false);

    VerifyOrReturnError(BN_bn2binpad(privKeyBN, serializedKeypair.Bytes() + kP256_PublicKey_Length, kP256_PrivateKey_Length) ==
                            kP256_PrivateKey_Length,
                        false);

    serializedKeypair.SetLength(kP256_PublicKey_Length + kP256_PrivateKey_Length);

    return true;
}

bool ReadKey(const char * fileNameOrStr, std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> & key, bool ignorErrorIfUnsupportedCurve)
{
    bool res            = true;
    uint32_t keyDataLen = 0;
    KeyFormat keyFormat = kKeyFormat_Unknown;
    std::unique_ptr<uint8_t[]> keyData;

    // If fileNameOrStr is a file name
    if (access(fileNameOrStr, R_OK) == 0)
    {
        res = ReadFileIntoMem(fileNameOrStr, nullptr, keyDataLen);
        VerifyTrueOrExit(res);

        keyData = std::unique_ptr<uint8_t[]>(new uint8_t[keyDataLen]);

        res = ReadFileIntoMem(fileNameOrStr, keyData.get(), keyDataLen);
        VerifyTrueOrExit(res);

        keyFormat = DetectKeyFormat(keyData.get(), keyDataLen);
        if (keyFormat == kKeyFormat_Unknown)
        {
            fprintf(stderr, "Unrecognized Key Format in File: %s\n", fileNameOrStr);
            return false;
        }
    }
    // Otherwise, treat fileNameOrStr as a pointer to the key string
    else
    {
        keyDataLen = static_cast<uint32_t>(strlen(fileNameOrStr));

        keyFormat = DetectKeyFormat(reinterpret_cast<const uint8_t *>(fileNameOrStr), keyDataLen);
        if (keyFormat == kKeyFormat_Unknown)
        {
            fprintf(stderr, "Unrecognized Key Format in Input Argument: %s\n", fileNameOrStr);
            return false;
        }

        keyData = std::unique_ptr<uint8_t[]>(new uint8_t[keyDataLen]);
        memcpy(keyData.get(), fileNameOrStr, keyDataLen);
    }

    if ((keyFormat == kKeyFormat_X509_Hex) || (keyFormat == kKeyFormat_Chip_Hex) || (keyFormat == kKeyFormat_Chip_Pubkey_Hex))
    {
        size_t len = HexToBytes(Uint8::to_char(keyData.get()), keyDataLen, keyData.get(), keyDataLen);
        VerifyOrReturnError(CanCastTo<uint32_t>(2 * len), false);
        VerifyOrReturnError(2 * len == keyDataLen, false);
        keyDataLen = static_cast<uint32_t>(len);
    }
    else if ((keyFormat == kKeyFormat_Chip_Base64) || (keyFormat == kKeyFormat_Chip_Pubkey_Base64))
    {
        res = Base64Decode(keyData.get(), keyDataLen, keyData.get(), keyDataLen, keyDataLen);
        VerifyTrueOrExit(res);
    }
    else if (keyFormat == kKeyFormat_Chip_Hex)
    {
        const char * keyChars = reinterpret_cast<const char *>(keyData.get());

        keyDataLen = static_cast<uint32_t>(Encoding::HexToBytes(keyChars, keyDataLen, keyData.get(), keyDataLen));
        res        = (keyDataLen > 0);
        VerifyTrueOrExit(res);

        keyFormat = kKeyFormat_Chip_Raw;
    }

    if (IsChipPrivateKeyFormat(keyFormat))
    {
        res = DeserializeKeyPair(keyData.get(), keyDataLen, key.get());
        VerifyTrueOrExit(res);
    }
    else if (IsChipPublicKeyFormat(keyFormat))
    {
        res = SetPublicKey(keyData.get(), keyDataLen, key.get());
        VerifyTrueOrExit(res);
    }
    else
    {
        std::unique_ptr<BIO, void (*)(BIO *)> keyBIO(
            BIO_new_mem_buf(static_cast<const void *>(keyData.get()), static_cast<int>(keyDataLen)), &BIO_free_all);

        if (keyFormat == kKeyFormat_X509_Pubkey_PEM)
        {
            EC_KEY * ecKey = EC_KEY_new();

            if (PEM_read_bio_EC_PUBKEY(keyBIO.get(), &ecKey, nullptr, nullptr) == nullptr)
            {
                ReportOpenSSLErrorAndExit("PEM_read_bio_EC_PUBKEY", res = false);
            }

            if (EVP_PKEY_set1_EC_KEY(key.get(), ecKey) != 1)
            {
                ReportOpenSSLErrorAndExit("EVP_PKEY_set1_EC_KEY", res = false);
            }
        }
        else if (keyFormat == kKeyFormat_X509_PEM)
        {
            EVP_PKEY * tmpKeyPtr = nullptr;
            if (PEM_read_bio_PrivateKey(keyBIO.get(), &tmpKeyPtr, nullptr, nullptr) == nullptr)
            {
                ReportOpenSSLErrorAndExit("PEM_read_bio_PrivateKey", res = false);
            }
            key.reset(tmpKeyPtr);
        }
        else
        {
            EVP_PKEY * tmpKeyPtr = nullptr;
            if (d2i_PrivateKey_bio(keyBIO.get(), &tmpKeyPtr) == nullptr)
            {
                ReportOpenSSLErrorAndExit("d2i_PrivateKey_bio", res = false);
            }
            key.reset(tmpKeyPtr);
        }
    }

    if ((EC_GROUP_get_curve_name(EC_KEY_get0_group(EVP_PKEY_get1_EC_KEY(key.get()))) != gNIDChipCurveP256) &&
        !ignorErrorIfUnsupportedCurve)
    {
        fprintf(stderr, "Specified key uses unsupported Elliptic Curve\n");
        ExitNow(res = false);
    }

exit:
    return res;
}

bool GenerateKeyPair(EVP_PKEY * key)
{
    bool res = true;
    std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ecKey(EC_KEY_new_by_curve_name(gNIDChipCurveP256), &EC_KEY_free);

    VerifyOrExit(key != nullptr, res = false);

    if (!EC_KEY_generate_key(ecKey.get()))
    {
        ReportOpenSSLErrorAndExit("EC_KEY_generate_key", res = false);
    }

    if (!EVP_PKEY_set1_EC_KEY(key, ecKey.get()))
    {
        ReportOpenSSLErrorAndExit("EVP_PKEY_set1_EC_KEY", res = false);
    }

exit:
    return res;
}

bool GenerateKeyPair_Secp256k1(EVP_PKEY * key)
{
    bool res = true;
    std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ecKey(EC_KEY_new_by_curve_name(NID_secp256k1), &EC_KEY_free);

    VerifyOrExit(key != nullptr, res = false);

    if (!EC_KEY_generate_key(ecKey.get()))
    {
        ReportOpenSSLErrorAndExit("EC_KEY_generate_key", res = false);
    }

    if (!EVP_PKEY_set1_EC_KEY(key, ecKey.get()))
    {
        ReportOpenSSLErrorAndExit("EVP_PKEY_set1_EC_KEY", res = false);
    }

exit:
    return res;
}

bool WriteKey(const char * fileName, EVP_PKEY * key, KeyFormat keyFmt)
{
    bool res              = true;
    FILE * file           = nullptr;
    uint8_t * derKey      = nullptr;
    DataFormat dataFormat = kDataFormat_Unknown;

    VerifyOrExit(key != nullptr, res = false);

    if (keyFmt == kKeyFormat_X509_PEM || keyFmt == kKeyFormat_X509_Pubkey_PEM || keyFmt == kKeyFormat_X509_DER)
    {
        VerifyOrExit(OpenFile(fileName, file, true), res = false);
    }

    if (EVP_PKEY_type(EVP_PKEY_id(key)) != EVP_PKEY_EC)
    {
        fprintf(stderr, "Unsupported private key type\n");
        ExitNow(res = false);
    }

    switch (keyFmt)
    {
    case kKeyFormat_X509_PEM:
        if (PEM_write_ECPrivateKey(file, EVP_PKEY_get1_EC_KEY(key), nullptr, nullptr, 0, nullptr, nullptr) == 0)
        {
            ReportOpenSSLErrorAndExit("PEM_write_ECPrivateKey", res = false);
        }
        break;
    case kKeyFormat_X509_Pubkey_PEM:
        if (PEM_write_EC_PUBKEY(file, EVP_PKEY_get1_EC_KEY(key)) == 0)
        {
            ReportOpenSSLErrorAndExit("PEM_write_EC_PUBKEY", res = false);
        }
        break;
    case kKeyFormat_X509_DER:
        if (i2d_ECPrivateKey_fp(file, EVP_PKEY_get1_EC_KEY(key)) == 0)
        {
            ReportOpenSSLErrorAndExit("i2d_PrivateKey_fp", res = false);
        }
        break;
    case kKeyFormat_X509_Hex: {
        int derKeyLen = i2d_ECPrivateKey(EVP_PKEY_get1_EC_KEY(key), &derKey);
        if (derKeyLen < 0)
        {
            ReportOpenSSLErrorAndExit("i2d_X509", res = false);
        }
        VerifyOrExit(CanCastTo<size_t>(derKeyLen), res = false);
        VerifyOrExit(WriteDataIntoFile(fileName, derKey, static_cast<size_t>(derKeyLen), kDataFormat_Hex), res = false);
    }
    break;
    case kKeyFormat_Chip_Raw:
    case kKeyFormat_Chip_Hex:
    case kKeyFormat_Chip_Base64:
        if (keyFmt == kKeyFormat_Chip_Raw)
            dataFormat = kDataFormat_Raw;
        else if (keyFmt == kKeyFormat_Chip_Base64)
            dataFormat = kDataFormat_Base64;
        else
            dataFormat = kDataFormat_Hex;

        {
            P256SerializedKeypair serializedKeypair;
            VerifyOrExit(SerializeKeyPair(key, serializedKeypair), res = false);
            VerifyOrExit(WriteDataIntoFile(fileName, serializedKeypair.Bytes(), serializedKeypair.Length(), dataFormat),
                         res = false);
        }
        break;
    case kKeyFormat_Chip_Pubkey_Raw:
    case kKeyFormat_Chip_Pubkey_Base64:
    case kKeyFormat_Chip_Pubkey_Hex:
        if (keyFmt == kKeyFormat_Chip_Pubkey_Raw)
            dataFormat = kDataFormat_Raw;
        else if (keyFmt == kKeyFormat_Chip_Pubkey_Base64)
            dataFormat = kDataFormat_Base64;
        else
            dataFormat = kDataFormat_Hex;

        {
            uint8_t pubkey[kP256_PublicKey_Length] = { 0 };
            MutableByteSpan pubkeySpan(pubkey);
            VerifyOrExit(ExtractPublicKey(key, pubkeySpan), res = false);
            VerifyOrExit(WriteDataIntoFile(fileName, pubkeySpan.data(), pubkeySpan.size(), dataFormat), res = false);
        }
        break;
    default:
        fprintf(stderr, "Unsupported private key format\n");
        ExitNow(res = false);
    }

exit:
    CloseFile(file);
    OPENSSL_free(derKey);

    return res;
}
