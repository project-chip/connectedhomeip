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

#include "chip-cert.h"

using namespace chip;
using namespace chip::Credentials;
using namespace chip::ASN1;

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
                snprintf(chipAttrStr, sizeof(chipAttrStr), "%016" PRIX64 "", rdn[i].mChipVal);
                chipAttrLen = 16;
            }
            else
            {
                snprintf(chipAttrStr, sizeof(chipAttrStr), "%08" PRIX32 "", static_cast<uint32_t>(rdn[i].mChipVal));
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
                                            const_cast<uint8_t *>(rdn[i].mString.data()), static_cast<int>(rdn[i].mString.size()),
                                            -1, 0))
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
            snprintf(valueStr, sizeof(valueStr), "%016" PRIX64, rdn[i].mChipVal);
        }
        else if (IsChip32bitDNAttr(rdn[i].mAttrOID))
        {
            snprintf(valueStr, sizeof(valueStr), "%08" PRIX32, static_cast<uint32_t>(rdn[i].mChipVal));
        }
        else
        {
            size_t len = rdn[i].mString.size();
            if (len > sizeof(valueStr) - 1)
            {
                len = sizeof(valueStr) - 1;
            }
            memcpy(valueStr, rdn[i].mString.data(), len);
            valueStr[len] = 0;
        }

        fprintf(file, "%s=%s", chip::ASN1::GetOIDName(rdn[i].mAttrOID), valueStr);
        if (certDesc != nullptr)
        {
            fprintf(file, " (%s)", certDesc);
        }
    }
}

namespace {

CertFormat DetectCertFormat(uint8_t * cert, uint32_t certLen)
{
    static const uint8_t chipRawPrefix[] = { 0x15, 0x30, 0x01 };
    static const char * chipB64Prefix    = "FTABC";
    static const size_t chipB64PrefixLen = strlen(chipB64Prefix);
    static const char * pemMarker        = "-----BEGIN CERTIFICATE-----";

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

bool SetCertSerialNumber(X509 * cert)
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

bool SetCertTimeField(ASN1_TIME * asn1Time, const struct tm & value)
{
    char timeStr[16];

    // Encode the time as a string in the form YYYYMMDDHHMMSSZ.
    snprintf(timeStr, sizeof(timeStr), "%04d%02d%02d%02d%02d%02dZ", static_cast<uint16_t>(value.tm_year + 1900) % 9999,
             static_cast<uint8_t>(value.tm_mon) % kMonthsPerYear + 1, static_cast<uint8_t>(value.tm_mday) % kMaxDaysPerMonth,
             static_cast<uint8_t>(value.tm_hour) % kHoursPerDay, static_cast<uint8_t>(value.tm_min) % kMinutesPerHour,
             static_cast<uint8_t>(value.tm_sec) % kSecondsPerMinute);

    // X.509/RFC-5280 mandates that times before 2050 UTC must be encoded as ASN.1 UTCTime values, while
    // times equal or greater than 2050 must be encoded as GeneralizedTime values.  The only difference
    // between the two is the number of digits in the year -- 4 for GeneralizedTime, 2 for UTCTime.
    //
    // The OpenSSL ASN1_TIME_set_string() function DOES NOT handle picking the correct format based
    // on the given year.  Thus the caller MUST pass a correctly formatted string or the resultant
    // certificate will be malformed.

    bool useUTCTime = ((value.tm_year + 1900) < 2050);

    if (!ASN1_TIME_set_string(asn1Time, timeStr + (useUTCTime ? 2 : 0)))
    {
        fprintf(stderr, "OpenSSL ASN1_TIME_set_string() failed\n");
        return false;
    }

    return true;
}

bool SetValidityTime(X509 * cert, const struct tm & validFrom, uint32_t validDays)
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

bool AddExtension(X509 * cert, int extNID, const char * extStr)
{
    bool res = true;
    std::unique_ptr<X509_EXTENSION, void (*)(X509_EXTENSION *)> ex(
        X509V3_EXT_nconf_nid(nullptr, nullptr, extNID, const_cast<char *>(extStr)), &X509_EXTENSION_free);

    if (!X509_add_ext(cert, ex.get(), -1))
    {
        ReportOpenSSLErrorAndExit("X509_add_ext", res = false);
    }

exit:
    return res;
}

/** The key identifier field is derived from the public key using method (1) per RFC5280 (section 4.2.1.2):
 *
 *      (1) The keyIdentifier is composed of the 160-bit SHA-1 hash of the
 *           value of the BIT STRING subjectPublicKey (excluding the tag,
 *           length, and number of unused bits).
 */
bool AddSubjectKeyId(X509 * cert)
{
    bool res             = true;
    ASN1_BIT_STRING * pk = X509_get0_pubkey_bitstr(cert);
    unsigned char pkHash[EVP_MAX_MD_SIZE];
    unsigned int pkHashLen;
    std::unique_ptr<ASN1_STRING, void (*)(ASN1_STRING *)> pkHashOS(ASN1_STRING_type_new(V_ASN1_OCTET_STRING), &ASN1_STRING_free);

    if (!EVP_Digest(pk->data, static_cast<size_t>(pk->length), pkHash, &pkHashLen, EVP_sha1(), nullptr))
    {
        ReportOpenSSLErrorAndExit("EVP_Digest", res = false);
    }

    if (pkHashLen != kKeyIdentifierLength)
    {
        fprintf(stderr, "Unexpected hash length returned from EVP_Digest()\n");
        ExitNow(res = false);
    }

    if (!ASN1_STRING_set(pkHashOS.get(), pkHash, static_cast<int>(pkHashLen)))
    {
        ReportOpenSSLErrorAndExit("ASN1_STRING_set", res = false);
    }

    if (!X509_add1_ext_i2d(cert, NID_subject_key_identifier, pkHashOS.get(), 0, X509V3_ADD_APPEND))
    {
        ReportOpenSSLErrorAndExit("X509_add1_ext_i2d", res = false);
    }

exit:
    return res;
}

bool AddAuthorityKeyId(X509 * cert, X509 * caCert)
{
    bool res = true;
    int isCritical;
    int index = 0;
    std::unique_ptr<AUTHORITY_KEYID, void (*)(AUTHORITY_KEYID *)> akid(AUTHORITY_KEYID_new(), &AUTHORITY_KEYID_free);

    akid.get()->keyid =
        reinterpret_cast<ASN1_OCTET_STRING *>(X509_get_ext_d2i(caCert, NID_subject_key_identifier, &isCritical, &index));
    if (akid.get()->keyid == nullptr)
    {
        ReportOpenSSLErrorAndExit("X509_get_ext_d2i", res = false);
    }

    if (!X509_add1_ext_i2d(cert, NID_authority_key_identifier, akid.get(), 0, X509V3_ADD_APPEND))
    {
        ReportOpenSSLErrorAndExit("X509_add1_ext_i2d", res = false);
    }

exit:
    return res;
}

bool ReadCertPEM(const char * fileName, X509 * cert)
{
    bool res    = true;
    FILE * file = nullptr;

    res = OpenFile(fileName, file);
    VerifyTrueOrExit(res);

    if (PEM_read_X509(file, &cert, nullptr, nullptr) == nullptr)
    {
        ReportOpenSSLErrorAndExit("PEM_read_X509", res = false);
    }

exit:
    CloseFile(file);
    return res;
}

} // namespace

bool ReadCert(const char * fileName, X509 * cert)
{
    CertFormat origCertFmt;
    return ReadCert(fileName, cert, origCertFmt);
}

bool ReadCert(const char * fileName, X509 * cert, CertFormat & certFmt)
{
    bool res          = true;
    CHIP_ERROR err    = CHIP_NO_ERROR;
    const uint8_t * p = nullptr;
    uint32_t certLen  = 0;
    std::unique_ptr<uint8_t[]> x509CertBuf(new uint8_t[kMaxDERCertLength]);
    std::unique_ptr<uint8_t[]> certBuf;

    res = ReadFileIntoMem(fileName, nullptr, certLen);
    VerifyTrueOrExit(res);

    certBuf = std::unique_ptr<uint8_t[]>(new uint8_t[certLen]);

    res = ReadFileIntoMem(fileName, certBuf.get(), certLen);
    VerifyTrueOrExit(res);

    certFmt = DetectCertFormat(certBuf.get(), certLen);

    if (certFmt == kCertFormat_X509_PEM)
    {
        res = ReadCertPEM(fileName, cert);
        VerifyTrueOrExit(res);
    }
    else
    {
        if (certFmt == kCertFormat_Chip_Base64)
        {
            res = Base64Decode(certBuf.get(), certLen, certBuf.get(), certLen, certLen);
            VerifyTrueOrExit(res);
        }

        if (certFmt == kCertFormat_Chip_Base64 || certFmt == kCertFormat_Chip_Raw)
        {
            err = ConvertChipCertToX509Cert(ByteSpan(certBuf.get(), certLen), x509CertBuf.get(), kMaxDERCertLength, certLen);
            if (err != CHIP_NO_ERROR)
            {
                fprintf(stderr, "Error converting certificate: %s\n", chip::ErrorStr(err));
                ExitNow(res = false);
            }

            p = x509CertBuf.get();
        }
        else
        {
            p = certBuf.get();
        }

        if (d2i_X509(&cert, &p, certLen) == nullptr)
        {
            ReportOpenSSLErrorAndExit("d2i_X509", res = false);
        }
    }

exit:
    return res;
}

bool X509ToChipCert(X509 * cert, uint8_t * certBuf, uint32_t certBufSize, uint32_t & certLen)
{
    bool res = true;
    CHIP_ERROR err;
    uint8_t * derCert = nullptr;
    int32_t derCertLen;

    derCertLen = i2d_X509(cert, &derCert);
    if (derCertLen < 0)
    {
        ReportOpenSSLErrorAndExit("i2d_X509", res = false);
    }

    VerifyOrReturnError(chip::CanCastTo<uint32_t>(derCertLen), false);

    err = ConvertX509CertToChipCert(ByteSpan(derCert, static_cast<uint32_t>(derCertLen)), certBuf, certBufSize, certLen);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "ConvertX509CertToChipCert() failed\n%s\n", chip::ErrorStr(err));
        ExitNow(res = false);
    }

exit:
    return res;
}

bool LoadChipCert(const char * fileName, bool isTrused, ChipCertificateSet & certSet, uint8_t * certBuf, uint32_t certBufSize)
{
    bool res = true;
    CHIP_ERROR err;
    uint32_t certLen;
    BitFlags<CertDecodeFlags> decodeFlags;
    std::unique_ptr<X509, void (*)(X509 *)> cert(X509_new(), &X509_free);

    res = ReadCert(fileName, cert.get());
    VerifyTrueOrExit(res);

    res = X509ToChipCert(cert.get(), certBuf, certBufSize, certLen);
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
    return res;
}

bool WriteCert(const char * fileName, X509 * cert, CertFormat certFmt)
{
    bool res    = true;
    FILE * file = nullptr;

    VerifyOrExit(cert != nullptr, res = false);

    res = OpenFile(fileName, file, true);
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
        uint8_t * certToWrite      = nullptr;
        uint32_t certToWriteLen    = 0;
        uint32_t chipCertLen       = kMaxCHIPCertLength;
        uint32_t chipCertBase64Len = BASE64_ENCODED_LEN(chipCertLen);
        std::unique_ptr<uint8_t[]> chipCert(new uint8_t[chipCertLen]);
        std::unique_ptr<uint8_t[]> chipCertBase64(new uint8_t[chipCertBase64Len]);

        res = X509ToChipCert(cert, chipCert.get(), chipCertLen, chipCertLen);
        VerifyTrueOrExit(res);

        if (certFmt == kCertFormat_Chip_Base64)
        {
            res = Base64Encode(chipCert.get(), chipCertLen, chipCertBase64.get(), chipCertBase64Len, chipCertBase64Len);
            VerifyTrueOrExit(res);

            certToWrite    = chipCertBase64.get();
            certToWriteLen = chipCertBase64Len;
        }
        else
        {
            certToWrite    = chipCert.get();
            certToWriteLen = chipCertLen;
        }

        if (fwrite(certToWrite, 1, certToWriteLen, file) != certToWriteLen)
        {
            fprintf(stderr, "Unable to write to %s: %s\n", fileName, strerror(ferror(file) ? errno : ENOSPC));
            ExitNow(res = false);
        }
    }

exit:
    CloseFile(file);
    return res;
}

bool MakeCert(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom,
              uint32_t validDays, const FutureExtension * futureExts, uint8_t futureExtsCount, X509 * newCert, EVP_PKEY * newKey)
{
    bool res = true;

    VerifyOrExit(subjectDN != nullptr, res = false);
    VerifyOrExit(caCert != nullptr, res = false);
    VerifyOrExit(caKey != nullptr, res = false);
    VerifyOrExit(newCert != nullptr, res = false);
    VerifyOrExit(newKey != nullptr, res = false);

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
            AddExtension(newCert, NID_key_usage, "critical,digitalSignature") &&
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

    for (uint8_t i = 0; i < futureExtsCount; i++)
    {
        res = AddExtension(newCert, futureExts[i].nid, futureExts[i].info);
        VerifyTrueOrExit(res);
    }

    // Sign the new certificate.
    if (!X509_sign(newCert, caKey, EVP_sha256()))
    {
        ReportOpenSSLErrorAndExit("X509_sign", res = false);
    }

exit:
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

bool MakeAttCert(AttCertType attCertType, const char * subjectCN, uint16_t subjectVID, uint16_t subjectPID, X509 * caCert,
                 EVP_PKEY * caKey, const struct tm & validFrom, uint32_t validDays, X509 * newCert, EVP_PKEY * newKey)
{
    bool res = true;

    VerifyOrReturnError(subjectCN != nullptr, false);
    VerifyOrReturnError(caCert != nullptr, false);
    VerifyOrReturnError(caKey != nullptr, false);
    VerifyOrReturnError(newCert != nullptr, false);
    VerifyOrReturnError(newKey != nullptr, false);

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

    // Add common name attribute to the certificate subject DN.
    if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(newCert), NID_commonName, MBSTRING_UTF8,
                                    reinterpret_cast<unsigned char *>(const_cast<char *>(subjectCN)),
                                    static_cast<int>(strlen(subjectCN)), -1, 0))
    {
        ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
    }

    // Add VID attribute to the certificate subject DN.
    if (subjectVID != 0)
    {
        char chipAttrStr[5];

        snprintf(chipAttrStr, sizeof(chipAttrStr), "%04" PRIX16 "", subjectVID);

        if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(newCert), gNIDChipAttAttrVID, MBSTRING_UTF8,
                                        reinterpret_cast<unsigned char *>(chipAttrStr), 4, -1, 0))
        {
            ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
        }
    }

    // Add PID attribute to the certificate subject DN.
    if (subjectPID != 0)
    {
        char chipAttrStr[5];

        snprintf(chipAttrStr, sizeof(chipAttrStr), "%04" PRIX16 "", subjectPID);

        if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(newCert), gNIDChipAttAttrPID, MBSTRING_UTF8,
                                        reinterpret_cast<unsigned char *>(chipAttrStr), 4, -1, 0))
        {
            ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
        }
    }

    // Set the issuer name for the certificate. In the case of a self-signed cert, this will be
    // the new cert's subject name.
    if (!X509_set_issuer_name(newCert, X509_get_subject_name(caCert)))
    {
        ReportOpenSSLErrorAndExit("X509_set_issuer_name", res = false);
    }

    // Add the appropriate certificate extensions.
    if (attCertType == kAttCertType_DAC)
    {
        res = AddExtension(newCert, NID_basic_constraints, "critical,CA:FALSE") &&
            AddExtension(newCert, NID_key_usage, "critical,digitalSignature");
    }
    // otherwise, it is PAI or PAA
    else
    {
        res = AddExtension(newCert, NID_basic_constraints, "critical,CA:TRUE") &&
            AddExtension(newCert, NID_key_usage, "critical,keyCertSign,cRLSign");
    }
    VerifyTrueOrExit(res);

    // Add a subject key id extension for the certificate.
    res = AddSubjectKeyId(newCert);
    VerifyTrueOrExit(res);

    // Add the authority key id extension from the signing certificate.
    res = AddAuthorityKeyId(newCert, caCert);
    VerifyTrueOrExit(res);

    // Sign the new certificate.
    if (!X509_sign(newCert, caKey, EVP_sha256()))
    {
        ReportOpenSSLErrorAndExit("X509_sign", res = false);
    }

exit:
    return res;
}
