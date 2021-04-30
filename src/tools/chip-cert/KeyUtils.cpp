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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "chip-cert.h"
#include <protocols/Protocols.h>
#include <support/Base64.h>
#include <support/BufferWriter.h>

using namespace chip;
using namespace chip::Protocols;
using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Crypto;

static KeyFormat DetectKeyFormat(const uint8_t * key, uint32_t keyLen)
{
    static uint32_t p256SerializedKeypairLen = kP256_PublicKey_Length + kP256_PrivateKey_Length;
    static const char * ecPEMMarker          = "-----BEGIN EC PRIVATE KEY-----";
    static const char * pkcs8PEMMarker       = "-----BEGIN PRIVATE KEY-----";
    static const char * ecPUBPEMMarker       = "-----BEGIN PUBLIC KEY-----";

    if (keyLen == p256SerializedKeypairLen)
    {
        return kKeyFormat_Chip_Raw;
    }

    if (keyLen == BASE64_ENCODED_LEN(p256SerializedKeypairLen))
    {
        return kKeyFormat_Chip_Base64;
    }

    if (ContainsPEMMarker(ecPEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_PEM;
    }

    if (ContainsPEMMarker(pkcs8PEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_PEM_PKCS8;
    }

    if (ContainsPEMMarker(ecPUBPEMMarker, key, keyLen))
    {
        return kKeyFormat_X509_PEM;
    }

    return kKeyFormat_X509_DER;
}

static bool SerializeKeyPair(EVP_PKEY * key, uint8_t *& chipKey, uint32_t & chipKeyLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t pubKey[kP256_PublicKey_Length];
    size_t pubKeyLen = 0;
    uint8_t privKey[kP256_PrivateKey_Length];
    int privKeyLen                  = 0;
    const BIGNUM * privKeyBN        = nullptr;
    const EC_GROUP * group          = nullptr;
    const EC_KEY * ecKey            = nullptr;
    const EC_POINT * ecPoint        = nullptr;
    P256SerializedKeypair * keyPair = nullptr;

    chipKey = nullptr;

    ecKey = EVP_PKEY_get1_EC_KEY(key);
    VerifyOrExit(ecKey != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    privKeyBN = EC_KEY_get0_private_key(ecKey);
    VerifyOrExit(privKeyBN != nullptr, err = CHIP_ERROR_INTERNAL);

    group = EC_KEY_get0_group(ecKey);
    VerifyOrExit(group != nullptr, err = CHIP_ERROR_INTERNAL);

    ecPoint = EC_KEY_get0_public_key(ecKey);
    VerifyOrExit(ecPoint != nullptr, err = CHIP_ERROR_INTERNAL);

    keyPair = reinterpret_cast<P256SerializedKeypair *>(malloc(sizeof(P256SerializedKeypair)));
    VerifyOrExit(keyPair != nullptr, err = CHIP_ERROR_NO_MEMORY);

    privKeyLen = BN_bn2binpad(privKeyBN, privKey, sizeof(privKey));
    VerifyOrExit(static_cast<size_t>(privKeyLen) == sizeof(privKey), err = CHIP_ERROR_INTERNAL);

    pubKeyLen = EC_POINT_point2oct(group, ecPoint, POINT_CONVERSION_UNCOMPRESSED, Uint8::to_uchar(pubKey), sizeof(pubKey), nullptr);
    VerifyOrExit(pubKeyLen == sizeof(pubKey), err = CHIP_ERROR_INTERNAL);

    {
        Encoding::LittleEndian::BufferWriter bbuf(reinterpret_cast<uint8_t *>(keyPair), keyPair->Capacity());
        bbuf.Put(pubKey, sizeof(pubKey));
        bbuf.Put(privKey, sizeof(privKey));
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
        keyPair->SetLength(bbuf.Needed());
    }

    chipKey    = reinterpret_cast<uint8_t *>(keyPair);
    chipKeyLen = static_cast<uint32_t>(keyPair->Length());

exit:
    ClearSecretData(privKey, sizeof(privKey));

    if (err != CHIP_NO_ERROR)
    {
        if (keyPair != nullptr)
        {
            ClearSecretData(reinterpret_cast<uint8_t *>(keyPair), static_cast<uint32_t>(keyPair->Length()));
            free(keyPair);
        }
        return false;
    }

    return true;
}

static bool DeserializeKeyPair(const uint8_t * keyPair, uint32_t keyPairLen, EVP_PKEY *& key)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    int res                 = 0;
    int nid                 = EC_curve_nist2nid("P-256");
    const uint8_t * pubKey  = keyPair;
    uint32_t pubKeyLen      = kP256_PublicKey_Length;
    const uint8_t * privKey = keyPair + pubKeyLen;
    uint32_t privKeyLen     = kP256_PrivateKey_Length;
    BIGNUM * privKeyBN      = nullptr;
    EC_GROUP * group        = nullptr;
    EC_POINT * ecPoint      = nullptr;
    EC_KEY * ecKey          = nullptr;

    ERR_clear_error();

    VerifyOrExit(keyPair != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(keyPairLen == privKeyLen + pubKeyLen, err = CHIP_ERROR_INVALID_ARGUMENT);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, err = CHIP_ERROR_INTERNAL);

    ecPoint = EC_POINT_new(group);
    VerifyOrExit(ecPoint != nullptr, err = CHIP_ERROR_INTERNAL);

    res = EC_POINT_oct2point(group, ecPoint, pubKey, pubKeyLen, nullptr);
    VerifyOrExit(res == 1, err = CHIP_ERROR_INTERNAL);

    ecKey = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ecKey != nullptr, err = CHIP_ERROR_INTERNAL);

    res = EC_KEY_set_public_key(ecKey, ecPoint);
    VerifyOrExit(res == 1, err = CHIP_ERROR_INTERNAL);

    privKeyBN = BN_bin2bn(privKey, static_cast<int>(privKeyLen), nullptr);
    VerifyOrExit(privKeyBN != nullptr, err = CHIP_ERROR_INTERNAL);

    res = EC_KEY_set_private_key(ecKey, privKeyBN);
    VerifyOrExit(res == 1, err = CHIP_ERROR_INTERNAL);

    key = EVP_PKEY_new();
    VerifyOrExit(key != nullptr, err = CHIP_ERROR_NO_MEMORY);

    EVP_PKEY_assign_EC_KEY(key, ecKey);

exit:
    if (privKeyBN != nullptr)
    {
        BN_clear_free(privKeyBN);
    }
    if (group != nullptr)
    {
        EC_GROUP_free(group);
    }
    if (ecPoint != nullptr)
    {
        EC_POINT_free(ecPoint);
    }

    return err;
}

bool ReadPrivateKey(const char * fileName, EVP_PKEY *& key)
{
    bool res            = true;
    uint8_t * keyData   = nullptr;
    uint32_t keyDataLen = 0;
    KeyFormat keyFormat = kKeyFormat_Unknown;
    BIO * keyBIO        = nullptr;

    key = nullptr;

    res = ReadFileIntoMem(fileName, keyData, keyDataLen);
    VerifyTrueOrExit(res);

    VerifyOrExit(keyDataLen <= MAX_KEY_SIZE, res = false);

    keyFormat = DetectKeyFormat(keyData, keyDataLen);

    if (keyFormat == kKeyFormat_Chip_Base64)
    {
        uint8_t * tmpKeyBuf = Base64Decode(keyData, keyDataLen, nullptr, 0, keyDataLen);
        if (tmpKeyBuf == nullptr)
        {
            fprintf(stderr, "Base64 decoding error\n");
            ExitNow(res = false);
        }

        free(keyData);
        keyData   = tmpKeyBuf;
        keyFormat = kKeyFormat_Chip_Raw;
    }

    if (keyFormat == kKeyFormat_Chip_Raw)
    {
        CHIP_ERROR err = DeserializeKeyPair(keyData, keyDataLen, key);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to decode Chip private key %s: %s\n", fileName, chip::ErrorStr(err));
            ExitNow(res = false);
        }
    }
    else
    {
        keyBIO = BIO_new_mem_buf(static_cast<const void *>(keyData), static_cast<int>(keyDataLen));
        if (keyBIO == nullptr)
        {
            fprintf(stderr, "Memory allocation error\n");
            ExitNow(res = false);
        }

        if (keyFormat == kKeyFormat_X509_PEM || keyFormat == kKeyFormat_X509_PEM_PKCS8)
        {
            if (PEM_read_bio_PrivateKey(keyBIO, &key, nullptr, nullptr) == nullptr)
            {
                fprintf(stderr, "Unable to read %s\n", fileName);
                ReportOpenSSLErrorAndExit("PEM_read_bio_PrivateKey", res = false);
            }
        }
        else
        {
            if (d2i_PrivateKey_bio(keyBIO, &key) == nullptr)
            {
                fprintf(stderr, "Unable to read %s\n", fileName);
                ReportOpenSSLErrorAndExit("d2i_PrivateKey_bio", res = false);
            }
        }
    }

exit:
    if (keyData != nullptr)
    {
        free(keyData);
    }
    if (keyBIO != nullptr)
    {
        BIO_free_all(keyBIO);
    }

    return res;
}

bool GenerateKeyPair(EVP_PKEY *& key)
{
    bool res           = true;
    EC_KEY * ecKey     = nullptr;
    EC_GROUP * ecGroup = nullptr;
    int curveNID;

    key = nullptr;

    curveNID = OBJ_sn2nid("prime256v1");
    if (curveNID == 0)
    {
        ReportOpenSSLErrorAndExit("OBJ_sn2nid", res = false);
    }

    ecGroup = EC_GROUP_new_by_curve_name(curveNID);
    if (ecGroup == nullptr)
    {
        ReportOpenSSLErrorAndExit("EC_GROUP_new_by_curve_name", res = false);
    }

    // Only include the curve name in the ASN.1 encoding of the public key.
    EC_GROUP_set_asn1_flag(ecGroup, OPENSSL_EC_NAMED_CURVE);

    ecKey = EC_KEY_new();
    if (ecKey == nullptr)
    {
        ReportOpenSSLErrorAndExit("EC_KEY_new", res = false);
    }

    if (EC_KEY_set_group(ecKey, ecGroup) == 0)
    {
        ReportOpenSSLErrorAndExit("EC_KEY_set_group", res = false);
    }

    if (!EC_KEY_generate_key(ecKey))
    {
        ReportOpenSSLErrorAndExit("EC_KEY_generate_key", res = false);
    }

    key = EVP_PKEY_new();
    if (key == nullptr)
    {
        ReportOpenSSLErrorAndExit("EVP_PKEY_new", res = false);
    }

    if (!EVP_PKEY_assign_EC_KEY(key, ecKey))
    {
        ReportOpenSSLErrorAndExit("EVP_PKEY_assign_EC_KEY", res = false);
    }

    ecKey = nullptr;

exit:
    if (ecKey != nullptr)
    {
        EC_KEY_free(ecKey);
    }
    if (key != nullptr && !res)
    {
        EVP_PKEY_free(key);
        key = nullptr;
    }
    return res;
}

bool WritePrivateKey(const char * fileName, EVP_PKEY * key, KeyFormat keyFmt)
{
    bool res             = true;
    uint8_t * chipKey    = nullptr;
    uint32_t chipKeyLen  = 0;
    char * chipKeyBase64 = nullptr;
    FILE * file          = nullptr;

    VerifyOrExit(key != nullptr, res = false);

    res = OpenFileToWrite(fileName, file);
    VerifyTrueOrExit(res);

    if (EVP_PKEY_type(EVP_PKEY_id(key)) != EVP_PKEY_EC)
    {
        fprintf(stderr, "Unsupported private key type\n");
        ExitNow(res = false);
    }

    if (keyFmt == kKeyFormat_X509_PEM || keyFmt == kKeyFormat_X509_PEM_PKCS8)
    {
        if (PEM_write_PrivateKey(file, key, nullptr, nullptr, 0, nullptr, nullptr) == 0)
        {
            ReportOpenSSLErrorAndExit("PEM_write_PrivateKey", res = false);
        }
    }
    else if (keyFmt == kKeyFormat_X509_DER || keyFmt == kKeyFormat_X509_DER_PKCS8)
    {
        if (i2d_PrivateKey_fp(file, key) == 0)
        {
            ReportOpenSSLErrorAndExit("i2d_PrivateKey_fp", res = false);
        }
    }
    else if (keyFmt == kKeyFormat_Chip_Raw || keyFmt == kKeyFormat_Chip_Base64)
    {
        res = SerializeKeyPair(key, chipKey, chipKeyLen);
        VerifyTrueOrExit(res);

        if (keyFmt == kKeyFormat_Chip_Raw)
        {
            if (fwrite(chipKey, 1, chipKeyLen, file) != chipKeyLen)
            {
                fprintf(stderr, "Unable to write to %s\n%s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
                ExitNow(res = false);
            }
        }
        else
        {
            chipKeyBase64 = Base64Encode(chipKey, chipKeyLen);
            if (chipKeyBase64 == nullptr)
            {
                fprintf(stderr, "Memory allocation error\n");
                ExitNow(res = false);
            }

            if (fputs(chipKeyBase64, file) == EOF)
            {
                fprintf(stderr, "Unable to write to %s\n%s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
                ExitNow(res = false);
            }
        }
    }
    else
    {
        fprintf(stderr, "Unsupported private key format");
        ExitNow(res = false);
    }

exit:
    CloseFile(fileName, file);

    if (chipKey != nullptr)
    {
        free(chipKey);
    }
    if (chipKeyBase64 != nullptr)
    {
        free(chipKeyBase64);
    }

    return res;
}
