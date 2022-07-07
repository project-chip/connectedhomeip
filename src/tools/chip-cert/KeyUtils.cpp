/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <protocols/Protocols.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Crypto;
using namespace chip::Encoding;

namespace {

KeyFormat DetectKeyFormat(const uint8_t * key, uint32_t keyLen)
{
    static uint32_t p256SerializedKeypairLen = kP256_PublicKey_Length + kP256_PrivateKey_Length;
    static const char * ecPEMMarker          = "-----BEGIN EC PRIVATE KEY-----";
    static const char * pkcs8PEMMarker       = "-----BEGIN PRIVATE KEY-----";
    static const char * ecPUBPEMMarker       = "-----BEGIN PUBLIC KEY-----";
    static const char * chipHexPrefix        = "04c2";
    static const size_t chipHexPrefixLen     = strlen(chipHexPrefix);
    static const size_t chipHexMinLen        = HEX_ENCODED_LENGTH(p256SerializedKeypairLen);

    if (keyLen == p256SerializedKeypairLen)
    {
        return kKeyFormat_Chip_Raw;
    }

    if (keyLen == BASE64_ENCODED_LEN(p256SerializedKeypairLen))
    {
        return kKeyFormat_Chip_Base64;
    }

    if (ContainsPEMMarker(ecPEMMarker, key, keyLen) || ContainsPEMMarker(pkcs8PEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_PEM;
    }

    if (ContainsPEMMarker(ecPUBPEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_PUBKEY_PEM;
    }

    if (keyLen >= chipHexMinLen && memcmp(key, chipHexPrefix, chipHexPrefixLen) == 0)
    {
        return kKeyFormat_Chip_Hex;
    }

    return kKeyFormat_X509_DER;
}

bool DeserializeKeyPair(const uint8_t * keyPair, uint32_t keyPairLen, EVP_PKEY * key)
{
    bool res                = true;
    int result              = 0;
    const uint8_t * pubKey  = keyPair;
    uint32_t pubKeyLen      = kP256_PublicKey_Length;
    const uint8_t * privKey = keyPair + pubKeyLen;
    uint32_t privKeyLen     = kP256_PrivateKey_Length;
    std::unique_ptr<EC_GROUP, void (*)(EC_GROUP *)> group(EC_GROUP_new_by_curve_name(gNIDChipCurveP256), &EC_GROUP_free);
    std::unique_ptr<EC_POINT, void (*)(EC_POINT *)> ecPoint(EC_POINT_new(group.get()), &EC_POINT_free);
    std::unique_ptr<EC_KEY, void (*)(EC_KEY *)> ecKey(EC_KEY_new_by_curve_name(gNIDChipCurveP256), &EC_KEY_free);
    std::unique_ptr<BIGNUM, void (*)(BIGNUM *)> privKeyBN(BN_new(), &BN_clear_free);

    ERR_clear_error();

    VerifyOrExit(key != nullptr, res = false);
    VerifyOrExit(keyPair != nullptr, res = false);
    VerifyOrExit(keyPairLen == privKeyLen + pubKeyLen, res = false);

    result = EC_POINT_oct2point(group.get(), ecPoint.get(), pubKey, pubKeyLen, nullptr);
    VerifyOrExit(result == 1, res = false);

    result = EC_KEY_set_public_key(ecKey.get(), ecPoint.get());
    VerifyOrExit(result == 1, res = false);

    VerifyOrExit(BN_bin2bn(privKey, static_cast<int>(privKeyLen), privKeyBN.get()) != nullptr, res = false);

    result = EC_KEY_set_private_key(ecKey.get(), privKeyBN.get());
    VerifyOrExit(result == 1, res = false);

    result = EVP_PKEY_set1_EC_KEY(key, ecKey.get());
    VerifyOrExit(result == 1, res = false);

exit:
    return res;
}

} // namespace

bool SerializeKeyPair(EVP_PKEY * key, P256SerializedKeypair & serializedKeypair)
{
    bool res                 = true;
    const BIGNUM * privKeyBN = nullptr;
    const EC_GROUP * group   = nullptr;
    const EC_KEY * ecKey     = nullptr;
    const EC_POINT * ecPoint = nullptr;
    uint8_t * pubKey         = serializedKeypair.Bytes();
    uint8_t * privKey        = pubKey + kP256_PublicKey_Length;
    size_t pubKeyLen         = 0;
    int privKeyLen           = 0;

    ecKey = EVP_PKEY_get1_EC_KEY(key);
    VerifyOrExit(ecKey != nullptr, res = false);

    privKeyBN = EC_KEY_get0_private_key(ecKey);
    VerifyOrExit(privKeyBN != nullptr, res = false);

    group = EC_KEY_get0_group(ecKey);
    VerifyOrExit(group != nullptr, res = false);

    ecPoint = EC_KEY_get0_public_key(ecKey);
    VerifyOrExit(ecPoint != nullptr, res = false);

    pubKeyLen =
        EC_POINT_point2oct(group, ecPoint, POINT_CONVERSION_UNCOMPRESSED, Uint8::to_uchar(pubKey), kP256_PublicKey_Length, nullptr);
    VerifyOrExit(pubKeyLen == kP256_PublicKey_Length, res = false);

    privKeyLen = BN_bn2binpad(privKeyBN, privKey, kP256_PrivateKey_Length);
    VerifyOrExit(privKeyLen == kP256_PrivateKey_Length, res = false);

    serializedKeypair.SetLength(kP256_PublicKey_Length + kP256_PrivateKey_Length);

exit:
    return res;
}

bool ReadKey(const char * fileName, EVP_PKEY * key, bool ignorErrorIfUnsupportedCurve)
{
    bool res            = true;
    uint32_t keyDataLen = 0;
    KeyFormat keyFormat = kKeyFormat_Unknown;
    std::unique_ptr<uint8_t[]> keyData;

    res = ReadFileIntoMem(fileName, nullptr, keyDataLen);
    VerifyTrueOrExit(res);

    keyData = std::unique_ptr<uint8_t[]>(new uint8_t[keyDataLen]);

    res = ReadFileIntoMem(fileName, keyData.get(), keyDataLen);
    VerifyTrueOrExit(res);

    keyFormat = DetectKeyFormat(keyData.get(), keyDataLen);

    if (keyFormat == kKeyFormat_Chip_Base64)
    {
        res = Base64Decode(keyData.get(), keyDataLen, keyData.get(), keyDataLen, keyDataLen);
        VerifyTrueOrExit(res);

        keyFormat = kKeyFormat_Chip_Raw;
    }
    else if (keyFormat == kKeyFormat_Chip_Hex)
    {
        const char * keyChars = reinterpret_cast<const char *>(keyData.get());

        keyDataLen = static_cast<uint32_t>(Encoding::HexToBytes(keyChars, keyDataLen, keyData.get(), keyDataLen));
        res        = (keyDataLen > 0);
        VerifyTrueOrExit(res);

        keyFormat = kKeyFormat_Chip_Raw;
    }

    if (keyFormat == kKeyFormat_Chip_Raw)
    {
        res = DeserializeKeyPair(keyData.get(), keyDataLen, key);
        VerifyTrueOrExit(res);
    }
    else
    {
        std::unique_ptr<BIO, void (*)(BIO *)> keyBIO(
            BIO_new_mem_buf(static_cast<const void *>(keyData.get()), static_cast<int>(keyDataLen)), &BIO_free_all);

        if (keyFormat == kKeyFormat_X509_PUBKEY_PEM)
        {
            EC_KEY * ecKey = EC_KEY_new();

            if (PEM_read_bio_EC_PUBKEY(keyBIO.get(), &ecKey, nullptr, nullptr) == nullptr)
            {
                ReportOpenSSLErrorAndExit("PEM_read_bio_EC_PUBKEY", res = false);
            }

            if (EVP_PKEY_set1_EC_KEY(key, ecKey) != 1)
            {
                ReportOpenSSLErrorAndExit("EVP_PKEY_set1_EC_KEY", res = false);
            }
        }
        else if (keyFormat == kKeyFormat_X509_PEM)
        {
            if (PEM_read_bio_PrivateKey(keyBIO.get(), &key, nullptr, nullptr) == nullptr)
            {
                ReportOpenSSLErrorAndExit("PEM_read_bio_PrivateKey", res = false);
            }
        }
        else
        {
            if (d2i_PrivateKey_bio(keyBIO.get(), &key) == nullptr)
            {
                ReportOpenSSLErrorAndExit("d2i_PrivateKey_bio", res = false);
            }
        }
    }

    if ((EC_GROUP_get_curve_name(EC_KEY_get0_group(EVP_PKEY_get1_EC_KEY(key))) != gNIDChipCurveP256) &&
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

bool WritePrivateKey(const char * fileName, EVP_PKEY * key, KeyFormat keyFmt)
{
    bool res               = true;
    FILE * file            = nullptr;
    uint8_t * keyToWrite   = nullptr;
    uint32_t keyToWriteLen = 0;
    P256SerializedKeypair serializedKeypair;
    uint32_t chipKeySize        = serializedKeypair.Capacity();
    uint32_t chipKeyDecodedSize = HEX_ENCODED_LENGTH(chipKeySize);
    std::unique_ptr<uint8_t[]> chipKey(new uint8_t[chipKeySize]);
    std::unique_ptr<uint8_t[]> chipKeyDecoded(new uint8_t[chipKeyDecodedSize]);

    VerifyOrExit(key != nullptr, res = false);

    res = OpenFile(fileName, file, true);
    VerifyTrueOrExit(res);

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
            ReportOpenSSLErrorAndExit("PEM_write_PrivateKey", res = false);
        }
        break;
    case kKeyFormat_X509_DER:
        if (i2d_ECPrivateKey_fp(file, EVP_PKEY_get1_EC_KEY(key)) == 0)
        {
            ReportOpenSSLErrorAndExit("i2d_PrivateKey_fp", res = false);
        }
        break;
    case kKeyFormat_Chip_Raw:
    case kKeyFormat_Chip_Hex:
    case kKeyFormat_Chip_Base64:
        res = SerializeKeyPair(key, serializedKeypair);
        VerifyTrueOrExit(res);

        if (keyFmt == kKeyFormat_Chip_Base64)
        {
            res = Base64Encode(serializedKeypair.Bytes(), static_cast<uint32_t>(serializedKeypair.Length()), chipKeyDecoded.get(),
                               chipKeyDecodedSize, chipKeyDecodedSize);
            VerifyTrueOrExit(res);

            keyToWrite    = chipKeyDecoded.get();
            keyToWriteLen = chipKeyDecodedSize;
        }
        else if (keyFmt == kKeyFormat_Chip_Hex)
        {
            char * keyHex = reinterpret_cast<char *>(chipKeyDecoded.get());

            SuccessOrExit(Encoding::BytesToLowercaseHexBuffer(
                serializedKeypair.Bytes(), static_cast<uint32_t>(serializedKeypair.Length()), keyHex, chipKeyDecodedSize));

            keyToWrite    = chipKeyDecoded.get();
            keyToWriteLen = chipKeyDecodedSize;
        }
        else
        {
            keyToWrite    = serializedKeypair.Bytes();
            keyToWriteLen = static_cast<uint32_t>(serializedKeypair.Length());
        }

        if (fwrite(keyToWrite, 1, keyToWriteLen, file) != keyToWriteLen)
        {
            fprintf(stderr, "Unable to write to %s\n%s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
            ExitNow(res = false);
        }
        break;
    default:
        fprintf(stderr, "Unsupported private key format");
        ExitNow(res = false);
    }

    printf("\r\n");

exit:
    CloseFile(file);

    return res;
}
