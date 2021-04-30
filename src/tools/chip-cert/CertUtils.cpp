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
 *      This file implements utility functions for reading, writing,
 *      parsing, resigning, encoding, and decoding CHIP certificates.
 *
 */

#define __STDC_FORMAT_MACROS

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "chip-cert.h"

using namespace chip;
using namespace chip::Credentials;
using namespace chip::ASN1;

enum
{
    kMaxChipCertInflationFactor = 5 // Maximum ratio of the size of buffer needed to hold an X.509 certificate
                                    // relative to the size of buffer needed to hold its CHIP counterpart.
                                    // This value (5) is conservatively big given that certificates contain large
                                    // amounts of incompressible data.  In practice, the factor is going to be
                                    // much closer to 1.5.
};

bool ToolChipDN::SetCertSubjectDN(X509 * cert) const
{
    bool res         = true;
    uint8_t rdnCount = RDNCount();

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        int attrNID;
        switch (rdn[i].mAttrOID)
        {
        case kOID_AttributeType_CommonName:
            attrNID = NID_commonName;
            break;
        case kOID_AttributeType_ChipNodeId:
            attrNID = gNIDChipNodeId;
            break;
        case kOID_AttributeType_ChipFirmwareSigningId:
            attrNID = gNIDChipFirmwareSigningId;
            break;
        case kOID_AttributeType_ChipICAId:
            attrNID = gNIDChipICAId;
            break;
        case kOID_AttributeType_ChipRootId:
            attrNID = gNIDChipRootId;
            break;
        case kOID_AttributeType_ChipFabricId:
            attrNID = gNIDChipFabricId;
            break;
        case kOID_AttributeType_ChipAuthTag1:
            attrNID = gNIDChipAuthTag1;
            break;
        case kOID_AttributeType_ChipAuthTag2:
            attrNID = gNIDChipAuthTag2;
            break;
        default:
            ExitNow(res = false);
        }

        if (IsChipDNAttr(rdn[i].mAttrOID))
        {
            char chipAttrStr[17];
            int chipAttrLen;

            if (IsChip64bitDNAttr(rdn[i].mAttrOID))
            {
                snprintf(chipAttrStr, sizeof(chipAttrStr), "%016" PRIX64 "", rdn[i].mAttrValue.mChipVal);
                chipAttrLen = 16;
            }
            else
            {
                snprintf(chipAttrStr, sizeof(chipAttrStr), "%08" PRIX32 "", static_cast<uint32_t>(rdn[i].mAttrValue.mChipVal));
                chipAttrLen = 8;
            }

            if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(cert), attrNID, MBSTRING_UTF8, (unsigned char *) chipAttrStr,
                                            chipAttrLen, -1, 0))
            {
                ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
            }
        }
        else
        {
            if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(cert), attrNID, MBSTRING_UTF8,
                                            (unsigned char *) rdn[i].mAttrValue.mString.mValue,
                                            (int) rdn[i].mAttrValue.mString.mLen, -1, 0))
            {
                ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
            }
        }
    }

exit:
    return res;
}

bool ToolChipDN::HasAttr(chip::ASN1::OID oid) const
{
    uint8_t rdnCount = RDNCount();

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        if (oid == rdn[i].mAttrOID)
        {
            return true;
        }
    }

    return false;
}

void ToolChipDN::PrintDN(FILE * file, const char * name) const
{
    uint8_t rdnCount = RDNCount();
    char valueStr[128];
    const char * certDesc = nullptr;

    fprintf(file, "%s: ", name);

    for (uint8_t i = 0; i < rdnCount; i++)
    {
        if (IsChip64bitDNAttr(rdn[i].mAttrOID))
        {
            snprintf(valueStr, sizeof(valueStr), "%016" PRIX64, rdn[i].mAttrValue.mChipVal);
        }
        else if (IsChip32bitDNAttr(rdn[i].mAttrOID))
        {
            snprintf(valueStr, sizeof(valueStr), "%08" PRIX32, static_cast<uint32_t>(rdn[i].mAttrValue.mChipVal));
        }
        else
        {
            uint32_t len = rdn[i].mAttrValue.mString.mLen;
            if (len > sizeof(valueStr) - 1)
            {
                len = sizeof(valueStr) - 1;
            }
            memcpy(valueStr, rdn[i].mAttrValue.mString.mValue, len);
            valueStr[len] = 0;
        }

        fprintf(file, "%s=%s", chip::ASN1::GetOIDName(rdn[i].mAttrOID), valueStr);
        if (certDesc != nullptr)
        {
            fprintf(file, " (%s)", certDesc);
        }
    }
}

static bool SetCertSerialNumber(X509 * cert)
{
    bool res = true;
    uint64_t rnd;
    ASN1_INTEGER * snInt = X509_get_serialNumber(cert);

    // Generate a random value to be used as the serial number.
    if (!RAND_bytes(reinterpret_cast<uint8_t *>(&rnd), sizeof(rnd)))
    {
        ReportOpenSSLErrorAndExit("RAND_bytes", res = false);
    }

    // Avoid negative numbers.
    rnd &= 0x7FFFFFFFFFFFFFFF;

    // Store the serial number as an ASN1 integer value within the certificate.
    if (ASN1_INTEGER_set_uint64(snInt, rnd) == 0)
    {
        ReportOpenSSLErrorAndExit("ASN1_INTEGER_set_uint64", res = false);
    }

exit:
    return res;
}

static bool SetCertTimeField(ASN1_TIME * s, const struct tm & value)
{
    char timeStr[16];

    // Encode the time as a string in the form YYYYMMDDHHMMSSZ.
    snprintf(timeStr, sizeof(timeStr), "%04d%02d%02d%02d%02d%02dZ", value.tm_year + 1900, value.tm_mon + 1, value.tm_mday,
             value.tm_hour, value.tm_min, value.tm_sec);

    // X.509/RFC-5280 mandates that times before 2050 UTC must be encoded as ASN.1 UTCTime values, while
    // times equal or greater than 2050 must be encoded as GeneralizedTime values.  The only difference
    // between the two is the number of digits in the year -- 4 for GeneralizedTime, 2 for UTCTime.
    //
    // The OpenSSL ASN1_TIME_set_string() function DOES NOT handle picking the correct format based
    // on the given year.  Thus the caller MUST pass a correctly formatted string or the resultant
    // certificate will be malformed.

    bool useUTCTime = ((value.tm_year + 1900) < 2050);

    if (!ASN1_TIME_set_string(s, timeStr + (useUTCTime ? 2 : 0)))
    {
        fprintf(stderr, "OpenSSL ASN1_TIME_set_string() failed\n");
        return false;
    }

    return true;
}

static bool SetValidityTime(X509 * cert, const struct tm & validFrom, uint32_t validDays)
{
    bool res = true;
    struct tm validTo;
    time_t validToTime;

    // Compute the validity end date.
    // Note that this computation is done in local time, despite the fact that the certificate validity times are
    // UTC.  This is because the standard posix time functions do not make it easy to convert a struct tm containing
    // UTC to a time_t value without manipulating the TZ environment variable.
    validTo = validFrom;
    validTo.tm_mday += validDays;
    validTo.tm_sec -= 1; // Ensure validity period is exactly a multiple of a day.
    validTo.tm_isdst = -1;
    validToTime      = mktime(&validTo);
    if (validToTime == static_cast<time_t>(-1))
    {
        fprintf(stderr, "mktime() failed\n");
        ExitNow(res = false);
    }
    localtime_r(&validToTime, &validTo);

    // Set the certificate's notBefore date.
    res = SetCertTimeField(X509_get_notBefore(cert), validFrom);
    VerifyTrueOrExit(res);

    // Set the certificate's notAfter date.
    res = SetCertTimeField(X509_get_notAfter(cert), validTo);
    VerifyTrueOrExit(res);

exit:
    return true;
}

static bool AddExtension(X509 * cert, int extNID, const char * extStr)
{
    bool res = true;
    X509_EXTENSION * ex;

    ex = X509V3_EXT_nconf_nid(nullptr, nullptr, extNID, const_cast<char *>(extStr));
    if (ex == nullptr)
    {
        ReportOpenSSLErrorAndExit("X509V3_EXT_conf_nid", res = false);
    }

    if (!X509_add_ext(cert, ex, -1))
    {
        ReportOpenSSLErrorAndExit("X509_add_ext", res = false);
    }

exit:
    if (ex != nullptr)
    {
        X509_EXTENSION_free(ex);
    }

    return res;
}

/* The key identifier field is derived from the public key using method (1) per RFC5280 (section 4.2.1.2):
 *
 *      (1) The keyIdentifier is composed of the 160-bit SHA-1 hash of the
 *           value of the BIT STRING subjectPublicKey (excluding the tag,
 *           length, and number of unused bits).
 */
static bool AddSubjectKeyId(X509 * cert)
{
    bool res               = true;
    ASN1_STRING * pkHashOS = nullptr;
    ASN1_BIT_STRING * pk   = X509_get0_pubkey_bitstr(cert);
    unsigned char pkHash[EVP_MAX_MD_SIZE];
    unsigned int pkHashLen;

    if (!EVP_Digest(pk->data, static_cast<size_t>(pk->length), pkHash, &pkHashLen, EVP_sha1(), nullptr))
    {
        ReportOpenSSLErrorAndExit("EVP_Digest", res = false);
    }

    if (pkHashLen != 20)
    {
        fprintf(stderr, "Unexpected hash length returned from EVP_Digest()\n");
        ExitNow(res = false);
    }

    pkHashOS = ASN1_STRING_type_new(V_ASN1_OCTET_STRING);
    if (pkHashOS == nullptr)
    {
        ReportOpenSSLErrorAndExit("ASN1_STRING_type_new", res = false);
    }

    if (!ASN1_STRING_set(pkHashOS, pkHash, static_cast<int>(pkHashLen)))
    {
        ReportOpenSSLErrorAndExit("ASN1_STRING_set", res = false);
    }

    if (!X509_add1_ext_i2d(cert, NID_subject_key_identifier, pkHashOS, 0, X509V3_ADD_APPEND))
    {
        ReportOpenSSLErrorAndExit("X509_add1_ext_i2d", res = false);
    }

exit:
    if (pkHashOS != nullptr)
    {
        ASN1_STRING_free(reinterpret_cast<ASN1_STRING *>(pkHashOS));
    }

    return res;
}

static bool AddAuthorityKeyId(X509 * cert, X509 * caCert)
{
    bool res = true;
    AUTHORITY_KEYID * akid;
    int isCritical, index = 0;

    akid = AUTHORITY_KEYID_new();
    if (akid == nullptr)
    {
        fprintf(stderr, "Memory allocation failure\n");
        ExitNow(res = false);
    }

    akid->keyid = (ASN1_OCTET_STRING *) X509_get_ext_d2i(caCert, NID_subject_key_identifier, &isCritical, &index);
    if (akid->keyid == nullptr)
    {
        ReportOpenSSLErrorAndExit("X509_get_ext_d2i", res = false);
    }

    if (!X509_add1_ext_i2d(cert, NID_authority_key_identifier, akid, 0, X509V3_ADD_APPEND))
    {
        ReportOpenSSLErrorAndExit("X509_add1_ext_i2d", res = false);
    }

exit:
    return res;
}

static bool X509PEMToDER(uint8_t * cert, uint32_t & certLen)
{
    bool res       = true;
    BIO * certBIO  = nullptr;
    char * name    = nullptr;
    char * header  = nullptr;
    uint8_t * data = nullptr;
    long dataLen;

    VerifyOrExit(certLen <= INT_MAX, res = false);

    certBIO = BIO_new_mem_buf(reinterpret_cast<void *>(cert), static_cast<int>(certLen));
    if (certBIO == nullptr)
    {
        fprintf(stderr, "Memory allocation error\n");
        ExitNow(res = false);
    }

    if (!PEM_read_bio(certBIO, &name, &header, &data, &dataLen))
    {
        ReportOpenSSLErrorAndExit("PEM_read_bio", res = false);
    }

    VerifyOrExit((dataLen >= 0) && (dataLen <= UINT32_MAX), res = false);

    memcpy(cert, data, static_cast<uint32_t>(dataLen));
    certLen = static_cast<uint32_t>(dataLen);

exit:
    if (certBIO != nullptr)
    {
        BIO_free(certBIO);
    }
    if (name != nullptr)
    {
        OPENSSL_free(name);
    }
    if (header != nullptr)
    {
        OPENSSL_free(header);
    }
    if (data != nullptr)
    {
        OPENSSL_free(data);
    }

    return res;
}

static CertFormat DetectCertFormat(uint8_t * cert, uint32_t certLen)
{
    static const uint8_t chipRawPrefix[]   = { 0xD5, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00 };
    static const char * chipB64Prefix      = "1QAACAAB";
    static const uint32_t chipB64PrefixLen = sizeof(chipB64Prefix) - 1;
    static const char * pemMarker          = "-----BEGIN CERTIFICATE-----";

    if (certLen > sizeof(chipRawPrefix) && memcmp(cert, chipRawPrefix, sizeof(chipRawPrefix)) == 0)
    {
        return kCertFormat_Chip_Raw;
    }

    if (certLen > chipB64PrefixLen && memcmp(cert, chipB64Prefix, chipB64PrefixLen) == 0)
    {
        return kCertFormat_Chip_Base64;
    }

    if (ContainsPEMMarker(pemMarker, cert, certLen))
    {
        return kCertFormat_X509_PEM;
    }

    return kCertFormat_X509_DER;
}

bool ReadCert(const char * fileName, X509 *& cert)
{
    CertFormat origCertFmt;
    return ReadCert(fileName, cert, origCertFmt);
}

bool ReadCert(const char * fileName, X509 *& cert, CertFormat & origCertFmt)
{
    bool res = true;
    CHIP_ERROR err;
    uint8_t * certBuf = nullptr;
    const uint8_t * p;
    uint32_t certLen;
    CertFormat certFmt;

    cert = nullptr;

    res = ReadFileIntoMem(fileName, certBuf, certLen);
    VerifyTrueOrExit(res);

    VerifyOrExit(certLen <= MAX_CERT_SIZE, res = false);

    certFmt = origCertFmt = DetectCertFormat(certBuf, certLen);

    if (certFmt == kCertFormat_X509_PEM)
    {
        res = X509PEMToDER(certBuf, certLen);
        VerifyTrueOrExit(res);

        certFmt = kCertFormat_X509_DER;
    }
    else if (certFmt == kCertFormat_Chip_Base64)
    {
        if (!Base64Decode(certBuf, certLen, certBuf, certLen, certLen))
        {
            ExitNow(res = false);
        }
        certFmt = kCertFormat_Chip_Raw;
    }

    if (certFmt == kCertFormat_Chip_Raw)
    {
        uint32_t convertedCertLen  = certLen * kMaxChipCertInflationFactor;
        uint8_t * convertedCertBuf = reinterpret_cast<uint8_t *>(malloc(convertedCertLen));

        err = ConvertChipCertToX509Cert(certBuf, certLen, convertedCertBuf, convertedCertLen, convertedCertLen);
        if (err != CHIP_NO_ERROR)
        {
            free(convertedCertBuf);
            fprintf(stderr, "Error converting certificate: %s\n", chip::ErrorStr(err));
            ExitNow(res = false);
        }

        free(certBuf);
        certBuf = convertedCertBuf;
        certLen = convertedCertLen;

        certFmt = kCertFormat_X509_DER;
    }

    p    = certBuf;
    cert = d2i_X509(nullptr, &p, certLen);
    if (cert == nullptr)
    {
        ReportOpenSSLErrorAndExit("d2i_X509", res = false);
    }

exit:
    if (cert != nullptr && !res)
    {
        X509_free(cert);
        cert = nullptr;
    }
    if (certBuf != nullptr)
    {
        free(certBuf);
    }
    return res;
}

bool X509ToChipCert(X509 * cert, uint8_t *& chipCert, uint32_t & chipCertLen)
{
    bool res = true;
    CHIP_ERROR err;
    uint8_t * derEncodedCert = nullptr;
    int32_t derEncodedCertLen;

    chipCert = nullptr;

    derEncodedCertLen = i2d_X509(cert, &derEncodedCert);
    if (derEncodedCertLen < 0)
    {
        ReportOpenSSLErrorAndExit("i2d_X509", res = false);
    }

    chipCert = reinterpret_cast<uint8_t *>(malloc(static_cast<size_t>(derEncodedCertLen)));
    if (chipCert == nullptr)
    {
        fprintf(stderr, "Memory allocation error\n");
        ExitNow(res = false);
    }

    err = ConvertX509CertToChipCert(derEncodedCert, static_cast<uint32_t>(derEncodedCertLen), chipCert,
                                    static_cast<uint32_t>(derEncodedCertLen), chipCertLen);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Failed to create node certificate: ConvertX509CertToChipCert() failed\n%s\n", chip::ErrorStr(err));
        ExitNow(res = false);
    }

exit:
    if (chipCert != nullptr && !res)
    {
        free(chipCert);
    }

    return res;
}

bool LoadChipCert(const char * fileName, bool isTrused, ChipCertificateSet & certSet, uint8_t *& certBuf)
{
    bool res = true;
    CHIP_ERROR err;
    uint32_t certLen;
    BitFlags<CertDecodeFlags> decodeFlags;
    X509 * cert = nullptr;

    res = ReadCert(fileName, cert);
    VerifyTrueOrExit(res);

    res = X509ToChipCert(cert, certBuf, certLen);
    VerifyTrueOrExit(res);

    if (isTrused)
    {
        decodeFlags.Set(CertDecodeFlags::kIsTrustAnchor);
    }
    else
    {
        decodeFlags.Set(CertDecodeFlags::kGenerateTBSHash);
    }

    err = certSet.LoadCert(certBuf, certLen, decodeFlags);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Error reading %s\n%s\n", fileName, chip::ErrorStr(err));
        ExitNow(res = false);
    }

exit:
    if (cert != nullptr)
    {
        X509_free(cert);
    }
    if (!res && certBuf != nullptr)
    {
        free(certBuf);
    }

    return res;
}

bool WriteCert(const char * fileName, X509 * cert, CertFormat certFmt)
{
    bool res              = true;
    FILE * file           = nullptr;
    uint8_t * chipCert    = nullptr;
    uint32_t chipCertLen  = 0;
    char * chipCertBase64 = nullptr;

    VerifyOrExit(cert != nullptr, res = false);

    res = OpenFileToWrite(fileName, file);
    VerifyTrueOrExit(res);

    if (certFmt == kCertFormat_X509_PEM)
    {
        if (PEM_write_X509(file, cert) == 0)
        {
            ReportOpenSSLErrorAndExit("PEM_write_X509", res = false);
        }
    }
    else if (certFmt == kCertFormat_X509_DER)
    {
        if (i2d_X509_fp(file, cert) == 0)
        {
            ReportOpenSSLErrorAndExit("i2d_X509_fp", res = false);
        }
    }
    else if (certFmt == kCertFormat_Chip_Raw || certFmt == kCertFormat_Chip_Base64)
    {
        res = X509ToChipCert(cert, chipCert, chipCertLen);
        VerifyTrueOrExit(res);

        if (certFmt == kCertFormat_Chip_Raw)
        {
            if (fwrite(chipCert, 1, chipCertLen, file) != chipCertLen)
            {
                fprintf(stderr, "Unable to write to %s\n%s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
                ExitNow(res = false);
            }
        }
        else
        {
            chipCertBase64 = Base64Encode(chipCert, chipCertLen);
            if (chipCertBase64 == nullptr)
            {
                fprintf(stderr, "Memory allocation error\n");
                ExitNow(res = false);
            }

            if (fputs(chipCertBase64, file) == EOF)
            {
                fprintf(stderr, "Unable to write to %s\n%s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
                ExitNow(res = false);
            }
        }
    }

exit:
    CloseFile(fileName, file);

    if (chipCert != nullptr)
    {
        free(chipCert);
    }
    if (chipCertBase64 != nullptr)
    {
        free(chipCertBase64);
    }

    return res;
}

bool MakeCert(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom,
              uint32_t validDays, X509 *& newCert, EVP_PKEY *& newKey, const char * extSub, const char * extInfo)
{
    bool res         = true;
    bool keySupplied = (newKey != nullptr);

    newCert = nullptr;

    if (!keySupplied && !GenerateKeyPair(newKey))
    {
        ExitNow(res = false);
    }

    // Create new certificate object.
    newCert = X509_new();
    if (newCert == nullptr)
    {
        ReportOpenSSLErrorAndExit("X509_new", res = false);
    }

    // If a signing CA certificate was not provided, then arrange to generate a self-signed
    // certificate.
    if (caCert == nullptr)
    {
        caCert = newCert;
        caKey  = newKey;
    }

    // Set the certificate version (must be 2, a.k.a. v3).
    if (!X509_set_version(newCert, 2))
    {
        ReportOpenSSLErrorAndExit("X509_set_version", res = false);
    }

    // Generate a serial number for the cert.
    res = SetCertSerialNumber(newCert);
    VerifyTrueOrExit(res);

    // Set the certificate validity time.
    res = SetValidityTime(newCert, validFrom, validDays);
    VerifyTrueOrExit(res);

    // Set the certificate's public key.
    if (!X509_set_pubkey(newCert, newKey))
    {
        ReportOpenSSLErrorAndExit("X509_set_pubkey", res = false);
    }

    // Set certificate subject DN.
    res = subjectDN->SetCertSubjectDN(newCert);
    VerifyTrueOrExit(res);

    // Set the issuer name for the certificate. In the case of a self-signed cert, this will be
    // the new cert's subject name.
    if (!X509_set_issuer_name(newCert, X509_get_subject_name(caCert)))
    {
        ReportOpenSSLErrorAndExit("X509_set_issuer_name", res = false);
    }

    // Add the appropriate certificate extensions.
    if (certType == kCertType_Node)
    {
        res = AddExtension(newCert, NID_basic_constraints, "critical,CA:FALSE") &&
            AddExtension(newCert, NID_key_usage, "critical,digitalSignature,keyEncipherment") &&
            AddExtension(newCert, NID_ext_key_usage, "critical,clientAuth,serverAuth");
    }
    else if (certType == kCertType_FirmwareSigning)
    {
        res = AddExtension(newCert, NID_basic_constraints, "critical,CA:FALSE") &&
            AddExtension(newCert, NID_key_usage, "critical,digitalSignature") &&
            AddExtension(newCert, NID_ext_key_usage, "critical,codeSigning");
    }
    else if (certType == kCertType_ICA || certType == kCertType_Root)
    {
        res = AddExtension(newCert, NID_basic_constraints, "critical,CA:TRUE") &&
            AddExtension(newCert, NID_key_usage, "critical,keyCertSign,cRLSign");
    }
    VerifyTrueOrExit(res);

    // Add a subject key id extension for the certificate.
    res = AddSubjectKeyId(newCert);
    VerifyTrueOrExit(res);

    // Add the authority key id extension from the signing certificate. For self-signed cert's this will
    // be the same as new cert's subject key id extension.
    res = AddAuthorityKeyId(newCert, caCert);
    VerifyTrueOrExit(res);

    if (extSub != nullptr)
    {
        res = AddExtension(newCert, NID_subject_alt_name, extSub);
        VerifyTrueOrExit(res);
    }

    if (extInfo != nullptr)
    {
        res = AddExtension(newCert, NID_info_access, extInfo);
        VerifyTrueOrExit(res);
    }

    // Sign the new certificate.
    if (!X509_sign(newCert, caKey, EVP_sha256()))
    {
        ReportOpenSSLErrorAndExit("X509_sign", res = false);
    }

exit:
    if (newCert != nullptr && !res)
    {
        X509_free(newCert);
    }
    if (!keySupplied && newKey != nullptr && !res)
    {
        EVP_PKEY_free(newKey);
    }

    return res;
}

bool ResignCert(X509 * cert, X509 * caCert, EVP_PKEY * caKey)
{
    bool res            = true;
    int authKeyIdExtLoc = -1;

    res = SetCertSerialNumber(cert);
    VerifyTrueOrExit(res);

    if (!X509_set_issuer_name(cert, X509_get_subject_name(caCert)))
    {
        ReportOpenSSLErrorAndExit("X509_set_issuer_name", res = false);
    }

    // Remove any existing authority key id
    authKeyIdExtLoc = X509_get_ext_by_NID(cert, NID_authority_key_identifier, -1);
    if (authKeyIdExtLoc != -1)
    {
        if (X509_delete_ext(cert, authKeyIdExtLoc) == nullptr)
        {
            ReportOpenSSLErrorAndExit("X509_delete_ext", res = false);
        }
    }

    res = AddAuthorityKeyId(cert, caCert);
    VerifyTrueOrExit(res);

    if (!X509_sign(cert, caKey, EVP_sha256()))
    {
        ReportOpenSSLErrorAndExit("X509_sign", res = false);
    }

exit:
    return res;
}
