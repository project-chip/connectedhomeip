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
 *      This file implements utility functions for reading, writing,
 *      parsing, resigning, encoding, and decoding CHIP certificates.
 *
 */

#define __STDC_FORMAT_MACROS

#include "chip-cert.h"
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BytesToHex.h>

#include <string>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::ASN1;
using namespace chip::TLV;
using namespace chip::Encoding;

bool ToolChipDN::SetCertName(X509_NAME * name) const
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
        case kOID_AttributeType_Surname:
            attrNID = NID_surname;
            break;
        case kOID_AttributeType_SerialNumber:
            attrNID = NID_serialNumber;
            break;
        case kOID_AttributeType_CountryName:
            attrNID = NID_countryName;
            break;
        case kOID_AttributeType_LocalityName:
            attrNID = NID_localityName;
            break;
        case kOID_AttributeType_StateOrProvinceName:
            attrNID = NID_stateOrProvinceName;
            break;
        case kOID_AttributeType_OrganizationName:
            attrNID = NID_organizationName;
            break;
        case kOID_AttributeType_OrganizationalUnitName:
            attrNID = NID_organizationalUnitName;
            break;
        case kOID_AttributeType_Title:
            attrNID = NID_title;
            break;
        case kOID_AttributeType_Name:
            attrNID = NID_name;
            break;
        case kOID_AttributeType_GivenName:
            attrNID = NID_givenName;
            break;
        case kOID_AttributeType_Initials:
            attrNID = NID_initials;
            break;
        case kOID_AttributeType_GenerationQualifier:
            attrNID = NID_generationQualifier;
            break;
        case kOID_AttributeType_DNQualifier:
            attrNID = NID_dnQualifier;
            break;
        case kOID_AttributeType_Pseudonym:
            attrNID = NID_pseudonym;
            break;
        case kOID_AttributeType_DomainComponent:
            attrNID = NID_domainComponent;
            break;
        case kOID_AttributeType_MatterNodeId:
            attrNID = gNIDChipNodeId;
            break;
        case kOID_AttributeType_MatterFirmwareSigningId:
            attrNID = gNIDChipFirmwareSigningId;
            break;
        case kOID_AttributeType_MatterICACId:
            attrNID = gNIDChipICAId;
            break;
        case kOID_AttributeType_MatterRCACId:
            attrNID = gNIDChipRootId;
            break;
        case kOID_AttributeType_MatterFabricId:
            attrNID = gNIDChipFabricId;
            break;
        case kOID_AttributeType_MatterCASEAuthTag:
            attrNID = gNIDChipCASEAuthenticatedTag;
            break;
        default:
            ExitNow(res = false);
        }

        char chipAttrStr[std::max(kChip64bitAttrUTF8Length, kChip32bitAttrUTF8Length)] = { 0 };
        int type                                                                       = V_ASN1_UTF8STRING;
        uint8_t * attrStr                                                              = reinterpret_cast<uint8_t *>(chipAttrStr);
        int attrLen                                                                    = 0;

        if (IsChip64bitDNAttr(rdn[i].mAttrOID))
        {
            VerifyOrReturnError(Encoding::Uint64ToHex(rdn[i].mChipVal, chipAttrStr, kChip64bitAttrUTF8Length,
                                                      Encoding::HexFlags::kUppercase) == CHIP_NO_ERROR,
                                false);
            attrLen = kChip64bitAttrUTF8Length;
        }
        else if (IsChip32bitDNAttr(rdn[i].mAttrOID))
        {
            VerifyOrReturnError(Encoding::Uint32ToHex(static_cast<uint32_t>(rdn[i].mChipVal), chipAttrStr, kChip32bitAttrUTF8Length,
                                                      Encoding::HexFlags::kUppercase) == CHIP_NO_ERROR,
                                false);
            attrLen = kChip32bitAttrUTF8Length;
        }
        else
        {
            if (rdn[i].mAttrOID == kOID_AttributeType_DomainComponent)
            {
                type = V_ASN1_IA5STRING;
            }
            else if (rdn[i].mAttrIsPrintableString)
            {
                type = V_ASN1_PRINTABLESTRING;
            }
            attrStr = reinterpret_cast<uint8_t *>(const_cast<char *>(rdn[i].mString.data()));
            attrLen = static_cast<int>(rdn[i].mString.size());
        }

        if (!X509_NAME_add_entry_by_NID(name, attrNID, type, attrStr, attrLen, -1, 0))
        {
            ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
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
            Encoding::Uint64ToHex(rdn[i].mChipVal, valueStr, sizeof(valueStr), Encoding::HexFlags::kUppercaseAndNullTerminate);
        }
        else if (IsChip32bitDNAttr(rdn[i].mAttrOID))
        {
            Encoding::Uint32ToHex(static_cast<uint32_t>(rdn[i].mChipVal), valueStr, sizeof(valueStr),
                                  Encoding::HexFlags::kUppercaseAndNullTerminate);
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

CertFormat DetectCertFormat(const uint8_t * cert, uint32_t certLen)
{
    static const uint8_t chipRawPrefix[] = { 0x15, 0x30, 0x01 };
    static const char * chipHexPrefix    = "153001";
    static const char * chipB64Prefix    = "FTAB";
    static const uint8_t derRawPrefix[]  = { 0x30, 0x82 };
    static const char * derHexPrefix     = "30820";
    static const char * pemMarker        = "-----BEGIN CERTIFICATE-----";

    VerifyOrReturnError(cert != nullptr, kCertFormat_Unknown);

    if ((certLen > sizeof(chipRawPrefix)) && (memcmp(cert, chipRawPrefix, sizeof(chipRawPrefix)) == 0))
    {
        return kCertFormat_Chip_Raw;
    }

    if ((certLen > strlen(chipHexPrefix)) && (memcmp(cert, chipHexPrefix, strlen(chipHexPrefix)) == 0))
    {
        return kCertFormat_Chip_Hex;
    }

    if ((certLen > strlen(chipB64Prefix)) && (memcmp(cert, chipB64Prefix, strlen(chipB64Prefix)) == 0))
    {
        return kCertFormat_Chip_Base64;
    }

    if ((certLen > sizeof(derRawPrefix)) && (memcmp(cert, derRawPrefix, sizeof(derRawPrefix)) == 0))
    {
        return kCertFormat_X509_DER;
    }

    if ((certLen > strlen(derHexPrefix)) && (memcmp(cert, derHexPrefix, strlen(derHexPrefix)) == 0))
    {
        return kCertFormat_X509_Hex;
    }

    if (ContainsPEMMarker(pemMarker, cert, certLen))
    {
        return kCertFormat_X509_PEM;
    }

    return kCertFormat_Unknown;
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
    char timeStr[ASN1UniversalTime::kASN1TimeStringMaxLength + 1];
    MutableCharSpan timeSpan(timeStr);
    ASN1UniversalTime val = { .Year   = static_cast<uint16_t>((value.tm_year == kX509NoWellDefinedExpirationDateYear)
                                                                ? kX509NoWellDefinedExpirationDateYear
                                                                : (value.tm_year + 1900)),
                              .Month  = static_cast<uint8_t>(value.tm_mon + 1),
                              .Day    = static_cast<uint8_t>(value.tm_mday),
                              .Hour   = static_cast<uint8_t>(value.tm_hour),
                              .Minute = static_cast<uint8_t>(value.tm_min),
                              .Second = static_cast<uint8_t>(value.tm_sec) };

    if (val.ExportTo_ASN1_TIME_string(timeSpan) != CHIP_NO_ERROR)
    {
        fprintf(stderr, "ExportTo_ASN1_TIME_string() failed\n");
        return false;
    }

    timeSpan.data()[timeSpan.size()] = '\0';

    if (!ASN1_TIME_set_string(asn1Time, timeStr))
    {
        fprintf(stderr, "OpenSSL ASN1_TIME_set_string() failed\n");
        return false;
    }

    return true;
}

bool SetValidityTime(X509 * cert, const struct tm & validFrom, uint32_t validDays, CertStructConfig & certConfig)
{
    bool res = true;
    struct tm validFromLocal;
    struct tm validTo;
    time_t validToTime;

    // Compute the validity end date.
    // Note that this computation is done in local time, despite the fact that the certificate validity times are
    // UTC.  This is because the standard posix time functions do not make it easy to convert a struct tm containing
    // UTC to a time_t value without manipulating the TZ environment variable.
    if (validDays == kCertValidDays_NoWellDefinedExpiration)
    {
        validTo.tm_year  = kX509NoWellDefinedExpirationDateYear;
        validTo.tm_mon   = kMonthsPerYear - 1;
        validTo.tm_mday  = kMaxDaysPerMonth;
        validTo.tm_hour  = kHoursPerDay - 1;
        validTo.tm_min   = kMinutesPerHour - 1;
        validTo.tm_sec   = kSecondsPerMinute - 1;
        validTo.tm_isdst = -1;
    }
    else
    {
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
    }

    if (certConfig.IsValidityCorrect())
    {
        validFromLocal = validFrom;
    }
    else
    {
        // Switch values if error flag is set.
        validFromLocal = validTo;
        validTo        = validFrom;
    }

    // Set the certificate's notBefore date.
    if (certConfig.IsValidityNotBeforePresent())
    {
        res = SetCertTimeField(X509_get_notBefore(cert), validFromLocal);
        VerifyTrueOrExit(res);
    }

    // Set the certificate's notAfter date.
    if (certConfig.IsValidityNotAfterPresent())
    {
        res = SetCertTimeField(X509_get_notAfter(cert), validTo);
        VerifyTrueOrExit(res);
    }

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

bool SetBasicConstraintsExtension(X509 * cert, bool isCA, int pathLen, CertStructConfig & certConfig)
{
    if (!certConfig.IsExtensionBasicPresent())
    {
        return true;
    }

    std::string basicConstraintsExt;

    if (certConfig.IsExtensionBasicCriticalPresent() && certConfig.IsExtensionBasicCritical())
    {
        basicConstraintsExt += "critical";
    }

    if (certConfig.IsExtensionBasicCAPresent())
    {
        if (!basicConstraintsExt.empty())
        {
            basicConstraintsExt += ",";
        }
        if ((certConfig.IsExtensionBasicCACorrect() && !isCA) || (!certConfig.IsExtensionBasicCACorrect() && isCA))
        {
            basicConstraintsExt += "CA:FALSE";
        }
        else
        {
            basicConstraintsExt += "CA:TRUE";
        }
    }

    if (pathLen != kPathLength_NotSpecified)
    {
        if (!basicConstraintsExt.empty())
        {
            basicConstraintsExt += ",";
        }
        basicConstraintsExt.append("pathlen:" + std::to_string(pathLen));
    }

    return AddExtension(cert, NID_basic_constraints, basicConstraintsExt.c_str());
}

bool SetKeyUsageExtension(X509 * cert, bool isCA, CertStructConfig & certConfig)
{
    if (!certConfig.IsExtensionKeyUsagePresent())
    {
        return true;
    }

    std::string keyUsageExt;

    if (certConfig.IsExtensionKeyUsageCriticalPresent() && certConfig.IsExtensionKeyUsageCritical())
    {
        keyUsageExt += "critical";
    }

    if ((certConfig.IsExtensionKeyUsageDigitalSigCorrect() && !isCA) ||
        (!certConfig.IsExtensionKeyUsageDigitalSigCorrect() && isCA))
    {
        if (!keyUsageExt.empty())
        {
            keyUsageExt += ",";
        }
        keyUsageExt += "digitalSignature";
    }

    if ((certConfig.IsExtensionKeyUsageKeyCertSignCorrect() && isCA) ||
        (!certConfig.IsExtensionKeyUsageKeyCertSignCorrect() && !isCA))
    {
        if (!keyUsageExt.empty())
        {
            keyUsageExt += ",";
        }
        keyUsageExt += "keyCertSign";
    }

    if ((certConfig.IsExtensionKeyUsageCRLSignCorrect() && isCA) || (!certConfig.IsExtensionKeyUsageCRLSignCorrect() && !isCA))
    {
        if (!keyUsageExt.empty())
        {
            keyUsageExt += ",";
        }
        keyUsageExt += "cRLSign";
    }

    // In test mode only: just add an extra extension flag to prevent empty extantion.
    if (certConfig.IsErrorTestCaseEnabled() && (keyUsageExt.empty() || (keyUsageExt.compare("critical") == 0)))
    {
        if (!keyUsageExt.empty())
        {
            keyUsageExt += ",";
        }
        keyUsageExt += "keyEncipherment";
    }

    return AddExtension(cert, NID_key_usage, keyUsageExt.c_str());
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

    akid->keyid = reinterpret_cast<ASN1_OCTET_STRING *>(X509_get_ext_d2i(caCert, NID_subject_key_identifier, &isCritical, &index));
    if (akid->keyid == nullptr)
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

} // namespace

bool ReadCert(const char * fileNameOrStr, X509 * cert)
{
    CertFormat origCertFmt;
    return ReadCert(fileNameOrStr, cert, origCertFmt);
}

bool ReadCert(const char * fileNameOrStr, X509 * cert, CertFormat & certFmt)
{
    bool res         = true;
    uint32_t certLen = 0;
    std::unique_ptr<uint8_t[]> certBuf;

    // If fileNameOrStr is a file name
    if (access(fileNameOrStr, R_OK) == 0)
    {
        res = ReadFileIntoMem(fileNameOrStr, nullptr, certLen);
        VerifyTrueOrExit(res);

        certBuf = std::unique_ptr<uint8_t[]>(new uint8_t[certLen]);

        res = ReadFileIntoMem(fileNameOrStr, certBuf.get(), certLen);
        VerifyTrueOrExit(res);

        certFmt = DetectCertFormat(certBuf.get(), certLen);
        if (certFmt == kCertFormat_Unknown)
        {
            fprintf(stderr, "Unrecognized Cert Format in File: %s\n", fileNameOrStr);
            return false;
        }
    }
    // Otherwise, treat fileNameOrStr as a pointer to the certificate string
    else
    {
        certLen = static_cast<uint32_t>(strlen(fileNameOrStr));

        certFmt = DetectCertFormat(reinterpret_cast<const uint8_t *>(fileNameOrStr), certLen);
        if (certFmt == kCertFormat_Unknown)
        {
            fprintf(stderr, "Unrecognized Cert Format in the Input Argument: %s\n", fileNameOrStr);
            return false;
        }

        certBuf = std::unique_ptr<uint8_t[]>(new uint8_t[certLen]);
        memcpy(certBuf.get(), fileNameOrStr, certLen);
    }

    if ((certFmt == kCertFormat_X509_Hex) || (certFmt == kCertFormat_Chip_Hex))
    {
        size_t len = chip::Encoding::HexToBytes(Uint8::to_char(certBuf.get()), certLen, certBuf.get(), certLen);
        VerifyOrReturnError(CanCastTo<uint32_t>(2 * len), false);
        VerifyOrReturnError(2 * len == certLen, false);
        certLen = static_cast<uint32_t>(len);
    }

    if (certFmt == kCertFormat_X509_PEM)
    {
        VerifyOrReturnError(chip::CanCastTo<int>(certLen), false);

        std::unique_ptr<BIO, void (*)(BIO *)> certBIO(
            BIO_new_mem_buf(static_cast<const void *>(certBuf.get()), static_cast<int>(certLen)), &BIO_free_all);

        if (PEM_read_bio_X509(certBIO.get(), &cert, nullptr, nullptr) == nullptr)
        {
            ReportOpenSSLErrorAndExit("PEM_read_bio_X509", res = false);
        }
    }
    else if ((certFmt == kCertFormat_X509_DER) || (certFmt == kCertFormat_X509_Hex))
    {
        VerifyOrReturnError(chip::CanCastTo<int>(certLen), false);

        const uint8_t * outCert = certBuf.get();

        if (d2i_X509(&cert, &outCert, static_cast<int>(certLen)) == nullptr)
        {
            ReportOpenSSLErrorAndExit("d2i_X509", res = false);
        }
    }
    // Otherwise, it is either CHIP TLV in raw, Base64, or hex encoded format.
    else
    {
        if (certFmt == kCertFormat_Chip_Base64)
        {
            res = Base64Decode(certBuf.get(), certLen, certBuf.get(), certLen, certLen);
            VerifyTrueOrExit(res);
        }

        std::unique_ptr<uint8_t[]> x509CertBuf(new uint8_t[kMaxDERCertLength]);
        MutableByteSpan x509Cert(x509CertBuf.get(), kMaxDERCertLength);

        CHIP_ERROR err = ConvertChipCertToX509Cert(ByteSpan(certBuf.get(), certLen), x509Cert);
        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Error converting certificate: %s\n", chip::ErrorStr(err));
            ExitNow(res = false);
        }

        const uint8_t * outCert = x509Cert.data();

        VerifyOrReturnError(chip::CanCastTo<int>(x509Cert.size()), false);

        if (d2i_X509(&cert, &outCert, static_cast<int>(x509Cert.size())) == nullptr)
        {
            ReportOpenSSLErrorAndExit("d2i_X509", res = false);
        }
    }

exit:
    return res;
}

bool ReadCertDER(const char * fileNameOrStr, MutableByteSpan & cert)
{
    bool res = true;
    std::unique_ptr<X509, void (*)(X509 *)> certX509(X509_new(), &X509_free);

    VerifyOrReturnError(ReadCert(fileNameOrStr, certX509.get()), false);

    uint8_t * certPtr = cert.data();
    int certLen       = i2d_X509(certX509.get(), &certPtr);
    if (certLen < 0)
    {
        ReportOpenSSLErrorAndExit("i2d_X509", res = false);
    }

    VerifyOrReturnError(chip::CanCastTo<size_t>(certLen), false);
    cert.reduce_size(static_cast<size_t>(certLen));

exit:
    return res;
}

bool X509ToChipCert(X509 * cert, MutableByteSpan & chipCert)
{
    bool res = true;
    CHIP_ERROR err;
    uint8_t * derCert = nullptr;
    int derCertLen;

    derCertLen = i2d_X509(cert, &derCert);
    if (derCertLen < 0)
    {
        ReportOpenSSLErrorAndExit("i2d_X509", res = false);
    }

    VerifyOrReturnError(chip::CanCastTo<size_t>(derCertLen), false);

    err = ConvertX509CertToChipCert(ByteSpan(derCert, static_cast<size_t>(derCertLen)), chipCert);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "ConvertX509CertToChipCert() failed\n%s\n", chip::ErrorStr(err));
        ExitNow(res = false);
    }

exit:
    OPENSSL_free(derCert);
    return res;
}

bool LoadChipCert(const char * fileNameOrStr, bool isTrused, ChipCertificateSet & certSet, MutableByteSpan & chipCert)
{
    bool res = true;
    CHIP_ERROR err;
    BitFlags<CertDecodeFlags> decodeFlags;
    std::unique_ptr<X509, void (*)(X509 *)> cert(X509_new(), &X509_free);

    res = ReadCert(fileNameOrStr, cert.get());
    VerifyTrueOrExit(res);

    res = X509ToChipCert(cert.get(), chipCert);
    VerifyTrueOrExit(res);

    if (isTrused)
    {
        decodeFlags.Set(CertDecodeFlags::kIsTrustAnchor);
    }
    else
    {
        decodeFlags.Set(CertDecodeFlags::kGenerateTBSHash);
    }

    err = certSet.LoadCert(chipCert, decodeFlags);
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "Error reading %s\n%s\n", fileNameOrStr, chip::ErrorStr(err));
        ExitNow(res = false);
    }

exit:
    return res;
}

bool WriteCert(const char * fileName, X509 * cert, CertFormat certFmt)
{
    bool res          = true;
    FILE * file       = nullptr;
    uint8_t * derCert = nullptr;

    VerifyOrReturnError(cert != nullptr, false);
    VerifyOrReturnError(certFmt != kCertFormat_Unknown, false);

    if (IsChipCertFormat(certFmt))
    {
        uint8_t chipCertBuf[kMaxCHIPCertLength];
        MutableByteSpan chipCert(chipCertBuf);

        VerifyOrReturnError(X509ToChipCert(cert, chipCert), false);

        return WriteChipCert(fileName, chipCert, certFmt);
    }

    if (certFmt == kCertFormat_X509_Hex)
    {
        int derCertLen = i2d_X509(cert, &derCert);
        if (derCertLen < 0)
        {
            ReportOpenSSLErrorAndExit("i2d_X509", res = false);
        }

        VerifyOrExit(CanCastTo<uint32_t>(derCertLen), res = false);
        VerifyOrExit(WriteDataIntoFile(fileName, derCert, static_cast<uint32_t>(derCertLen), kDataFormat_Hex), res = false);
        ExitNow(res = true);
    }

    VerifyOrExit(OpenFile(fileName, file, true), res = false);

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
    else
    {
        fprintf(stderr, "Unsupported certificate format\n");
        ExitNow(res = false);
    }

exit:
    OPENSSL_free(derCert);
    CloseFile(file);
    return res;
}

bool WriteChipCert(const char * fileName, const ByteSpan & chipCert, CertFormat certFmt)
{
    DataFormat dataFormat = kDataFormat_Unknown;

    VerifyOrReturnError(IsChipCertFormat(certFmt), false);

    if (certFmt == kCertFormat_Chip_Raw)
        dataFormat = kDataFormat_Raw;
    else if (certFmt == kCertFormat_Chip_Base64)
        dataFormat = kDataFormat_Base64;
    else
        dataFormat = kDataFormat_Hex;

    return WriteDataIntoFile(fileName, chipCert.data(), static_cast<uint32_t>(chipCert.size()), dataFormat);
}

bool MakeCert(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom,
              uint32_t validDays, int pathLen, const FutureExtensionWithNID * futureExts, uint8_t futureExtsCount, X509 * newCert,
              EVP_PKEY * newKey, CertStructConfig & certConfig)
{
    bool res  = true;
    bool isCA = (certType != kCertType_Node);

    VerifyOrExit(subjectDN != nullptr, res = false);
    VerifyOrExit(caCert != nullptr, res = false);
    VerifyOrExit(caKey != nullptr, res = false);
    VerifyOrExit(newCert != nullptr, res = false);
    VerifyOrExit(newKey != nullptr, res = false);

    // Set the certificate version (must be 2, a.k.a. v3).
    if (!X509_set_version(newCert, certConfig.GetCertVersion()))
    {
        ReportOpenSSLErrorAndExit("X509_set_version", res = false);
    }

    // Generate a serial number for the cert.
    if (certConfig.IsSerialNumberPresent())
    {
        res = SetCertSerialNumber(newCert);
        VerifyTrueOrExit(res);
    }

    // Set the issuer name for the certificate. In the case of a self-signed cert, this will be
    // the new cert's subject name.
    if (certConfig.IsIssuerPresent())
    {
        if (certType == kCertType_Root)
        {
            res = subjectDN->SetCertIssuerDN(newCert);
            VerifyTrueOrExit(res);
        }
        else
        {
            if (!X509_set_issuer_name(newCert, X509_get_subject_name(caCert)))
            {
                ReportOpenSSLErrorAndExit("X509_set_issuer_name", res = false);
            }
        }
    }

    // Set the certificate validity time.
    res = SetValidityTime(newCert, validFrom, validDays, certConfig);
    VerifyTrueOrExit(res);

    // Set the certificate's public key.
    if (!X509_set_pubkey(newCert, newKey))
    {
        ReportOpenSSLErrorAndExit("X509_set_pubkey", res = false);
    }

    // Injuct error into public key value.
    if (certConfig.IsPublicKeyError())
    {
        ASN1_BIT_STRING * pk = X509_get0_pubkey_bitstr(newCert);
        pk->data[CertStructConfig::kPublicKeyErrorByte] ^= 0xFF;
    }

    // Set certificate subject DN.
    if (certConfig.IsSubjectPresent())
    {
        res = subjectDN->SetCertSubjectDN(newCert);
        VerifyTrueOrExit(res);
    }

    // Add basic constraints certificate extensions.
    if (certConfig.IsExtensionBasicPathLenPresent() || !certConfig.IsExtensionBasicCAPresent())
    {
        pathLen = certConfig.GetExtensionBasicPathLenValue(certType);
    }
    res = SetBasicConstraintsExtension(newCert, isCA, pathLen, certConfig);
    VerifyTrueOrExit(res);

    // Add key usage certificate extensions.
    res = SetKeyUsageExtension(newCert, isCA, certConfig);
    VerifyTrueOrExit(res);

    // Add extended key usage certificate extensions.
    if (!certConfig.IsExtensionExtendedKeyUsageMissing())
    {
        if (certType == kCertType_Node)
        {
            res = AddExtension(newCert, NID_ext_key_usage, "critical,clientAuth,serverAuth");
            VerifyTrueOrExit(res);
        }
        else if (certType == kCertType_FirmwareSigning)
        {
            res = AddExtension(newCert, NID_ext_key_usage, "critical,codeSigning");
            VerifyTrueOrExit(res);
        }
    }

    // Add a subject key id extension for the certificate.
    if (certConfig.IsExtensionSKIDPresent())
    {
        res = AddSubjectKeyId(newCert);
        VerifyTrueOrExit(res);
    }

    // Add the authority key id extension from the signing certificate. For self-signed cert's this will
    // be the same as new cert's subject key id extension.
    if (certConfig.IsExtensionAKIDPresent())
    {
        if ((certType == kCertType_Root) && !certConfig.IsExtensionSKIDPresent())
        {
            res = AddSubjectKeyId(newCert);
            VerifyTrueOrExit(res);
            res = AddAuthorityKeyId(newCert, newCert);
            VerifyTrueOrExit(res);

            // Remove that temporary added subject key id
            int authKeyIdExtLoc = X509_get_ext_by_NID(newCert, NID_subject_key_identifier, -1);
            if (authKeyIdExtLoc != -1)
            {
                if (X509_delete_ext(newCert, authKeyIdExtLoc) == nullptr)
                {
                    ReportOpenSSLErrorAndExit("X509_delete_ext", res = false);
                }
            }
        }
        else
        {
            res = AddAuthorityKeyId(newCert, caCert);
            VerifyTrueOrExit(res);
        }
    }
    for (uint8_t i = 0; i < futureExtsCount; i++)
    {
        res = AddExtension(newCert, futureExts[i].nid, futureExts[i].info);
        VerifyTrueOrExit(res);
    }

    // Sign the new certificate.
    if (!X509_sign(newCert, caKey, certConfig.GetSignatureAlgorithmDER()))
    {
        ReportOpenSSLErrorAndExit("X509_sign", res = false);
    }

    // Injuct error into signature value.
    if (certConfig.IsSignatureError())
    {
        const ASN1_BIT_STRING * sig = nullptr;
        X509_get0_signature(&sig, nullptr, newCert);
        sig->data[20] ^= 0xFF;
    }

exit:
    return res;
}

CHIP_ERROR MakeCertChipTLV(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey,
                           const struct tm & validFrom, uint32_t validDays, int pathLen, const FutureExtensionWithNID * futureExts,
                           uint8_t futureExtsCount, X509 * x509Cert, EVP_PKEY * newKey, CertStructConfig & certConfig,
                           MutableByteSpan & chipCert)
{
    TLVWriter writer;
    TLVType containerType;
    TLVType containerType2;
    TLVType containerType3;
    uint8_t subjectPubkey[chip::Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES] = { 0 };
    uint8_t issuerPubkey[chip::Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES]  = { 0 };
    uint8_t keyid[chip::Crypto::kSHA1_Hash_Length]                         = { 0 };
    bool isCA;

    VerifyOrReturnError(subjectDN != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(caCert != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(caKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(x509Cert != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(newKey != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    isCA = (certType == kCertType_ICA || certType == kCertType_Root);

    uint8_t * p = subjectPubkey;
    VerifyOrReturnError(i2o_ECPublicKey(EVP_PKEY_get0_EC_KEY(newKey), &p) == chip::Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES,
                        CHIP_ERROR_INVALID_ARGUMENT);

    p = issuerPubkey;
    VerifyOrReturnError(i2o_ECPublicKey(EVP_PKEY_get0_EC_KEY(caKey), &p) == chip::Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES,
                        CHIP_ERROR_INVALID_ARGUMENT);

    writer.Init(chipCert);

    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, containerType));

    // serial number
    if (certConfig.IsSerialNumberPresent())
    {
        ASN1_INTEGER * asn1Integer = X509_get_serialNumber(x509Cert);
        uint64_t serialNumber;
        uint8_t serialNumberArray[sizeof(uint64_t)];
        VerifyOrReturnError(1 == ASN1_INTEGER_get_uint64(&serialNumber, asn1Integer), CHIP_ERROR_INVALID_ARGUMENT);
        Encoding::BigEndian::Put64(serialNumberArray, serialNumber);
        ReturnErrorOnFailure(writer.PutBytes(ContextTag(kTag_SerialNumber), serialNumberArray, sizeof(serialNumberArray)));
    }

    // signature algorithm
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SignatureAlgorithm), certConfig.GetSignatureAlgorithmTLVEnum()));

    // issuer Name
    if (certConfig.IsIssuerPresent())
    {
        if (certType == kCertType_Root)
        {
            ReturnErrorOnFailure(subjectDN->EncodeToTLV(writer, ContextTag(kTag_Issuer)));
        }
        else
        {
            uint8_t caChipCertBuf[kMaxCHIPCertLength];
            MutableByteSpan caChipCert(caChipCertBuf);
            VerifyOrReturnError(true == X509ToChipCert(caCert, caChipCert), CHIP_ERROR_INVALID_ARGUMENT);
            ChipDN issuerDN;
            ReturnErrorOnFailure(ExtractSubjectDNFromChipCert(caChipCert, issuerDN));
            ReturnErrorOnFailure(issuerDN.EncodeToTLV(writer, ContextTag(kTag_Issuer)));
        }
    }

    // validity
    uint32_t validFromChipEpoch;
    uint32_t validToChipEpoch;

    VerifyOrReturnError(true ==
                            CalendarToChipEpochTime(
                                static_cast<uint16_t>(validFrom.tm_year + 1900), static_cast<uint8_t>(validFrom.tm_mon + 1),
                                static_cast<uint8_t>(validFrom.tm_mday), static_cast<uint8_t>(validFrom.tm_hour),
                                static_cast<uint8_t>(validFrom.tm_min), static_cast<uint8_t>(validFrom.tm_sec), validFromChipEpoch),
                        CHIP_ERROR_INVALID_ARGUMENT);
    if (validDays == kCertValidDays_NoWellDefinedExpiration)
    {
        validToChipEpoch = 0;
    }
    else
    {
        VerifyOrReturnError(CanCastTo<uint32_t>(validFromChipEpoch + validDays * kSecondsPerDay - 1), CHIP_ERROR_INVALID_ARGUMENT);
        validToChipEpoch = validFromChipEpoch + validDays * kSecondsPerDay - 1;
    }
    if (!certConfig.IsValidityCorrect())
    {
        uint32_t validTemp = validFromChipEpoch;
        validFromChipEpoch = validToChipEpoch;
        validToChipEpoch   = validTemp;
    }
    if (certConfig.IsValidityNotBeforePresent())
    {
        ReturnErrorOnFailure(writer.Put(ContextTag(kTag_NotBefore), validFromChipEpoch));
    }
    if (certConfig.IsValidityNotAfterPresent())
    {
        ReturnErrorOnFailure(writer.Put(ContextTag(kTag_NotAfter), validToChipEpoch));
    }

    // subject Name
    if (certConfig.IsSubjectPresent())
    {
        ReturnErrorOnFailure(subjectDN->EncodeToTLV(writer, ContextTag(kTag_Subject)));
    }

    // public key algorithm
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_PublicKeyAlgorithm), GetOIDEnum(kOID_PubKeyAlgo_ECPublicKey)));

    // public key curve Id
    uint8_t ecCurveEnum = certConfig.IsSigCurveWrong() ? 0x02 : GetOIDEnum(kOID_EllipticCurve_prime256v1);
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_EllipticCurveIdentifier), ecCurveEnum));

    // public key
    if (certConfig.IsPublicKeyError())
    {
        subjectPubkey[CertStructConfig::kPublicKeyErrorByte] ^= 0xFF;
    }
    ReturnErrorOnFailure(
        writer.PutBytes(ContextTag(kTag_EllipticCurvePublicKey), subjectPubkey, chip::Crypto::CHIP_CRYPTO_PUBLIC_KEY_SIZE_BYTES));

    // extensions
    ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_Extensions), kTLVType_List, containerType2));
    {
        if (isCA)
        {
            // basic constraints
            if (certConfig.IsExtensionBasicPresent())
            {
                ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_BasicConstraints), kTLVType_Structure, containerType3));
                if (certConfig.IsExtensionBasicCAPresent())
                {
                    ReturnErrorOnFailure(writer.PutBoolean(ContextTag(kTag_BasicConstraints_IsCA),
                                                           certConfig.IsExtensionBasicCACorrect() ? true : false));
                }
                // TODO
                if (pathLen != kPathLength_NotSpecified)
                {
                    ReturnErrorOnFailure(
                        writer.Put(ContextTag(kTag_BasicConstraints_PathLenConstraint), static_cast<uint8_t>(pathLen)));
                }
                ReturnErrorOnFailure(writer.EndContainer(containerType3));
            }

            // key usage
            if (certConfig.IsExtensionKeyUsagePresent())
            {
                BitFlags<KeyUsageFlags> keyUsage;
                if (!certConfig.IsExtensionKeyUsageDigitalSigCorrect())
                {
                    keyUsage.Set(KeyUsageFlags::kDigitalSignature);
                }
                if (certConfig.IsExtensionKeyUsageKeyCertSignCorrect())
                {
                    keyUsage.Set(KeyUsageFlags::kKeyCertSign);
                }
                if (certConfig.IsExtensionKeyUsageCRLSignCorrect())
                {
                    keyUsage.Set(KeyUsageFlags::kCRLSign);
                }
                ReturnErrorOnFailure(writer.Put(ContextTag(kTag_KeyUsage), keyUsage.Raw()));
            }
        }
        else
        {
            // basic constraints
            if (certConfig.IsExtensionBasicPresent())
            {
                ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_BasicConstraints), kTLVType_Structure, containerType3));
                ReturnErrorOnFailure(writer.PutBoolean(ContextTag(kTag_BasicConstraints_IsCA), false));
                ReturnErrorOnFailure(writer.EndContainer(containerType3));
            }

            // key usage
            if (certConfig.IsExtensionKeyUsagePresent())
            {
                BitFlags<KeyUsageFlags> keyUsage;
                if (certConfig.IsExtensionKeyUsageDigitalSigCorrect())
                {
                    keyUsage.Set(KeyUsageFlags::kDigitalSignature);
                }
                if (!certConfig.IsExtensionKeyUsageKeyCertSignCorrect())
                {
                    keyUsage.Set(KeyUsageFlags::kKeyCertSign);
                }
                if (!certConfig.IsExtensionKeyUsageCRLSignCorrect())
                {
                    keyUsage.Set(KeyUsageFlags::kCRLSign);
                }
                ReturnErrorOnFailure(writer.Put(ContextTag(kTag_KeyUsage), keyUsage));
            }

            // extended key usage
            if (!certConfig.IsExtensionExtendedKeyUsageMissing() && (certType == kCertType_Node))
            {
                ReturnErrorOnFailure(writer.StartContainer(ContextTag(kTag_ExtendedKeyUsage), kTLVType_Array, containerType3));
                if (certType == kCertType_Node)
                {
                    ReturnErrorOnFailure(writer.Put(AnonymousTag(), GetOIDEnum(kOID_KeyPurpose_ClientAuth)));
                    ReturnErrorOnFailure(writer.Put(AnonymousTag(), GetOIDEnum(kOID_KeyPurpose_ServerAuth)));
                }
                else if (certType == kCertType_FirmwareSigning)
                {
                    ReturnErrorOnFailure(writer.Put(AnonymousTag(), GetOIDEnum(kOID_KeyPurpose_CodeSigning)));
                }
                ReturnErrorOnFailure(writer.EndContainer(containerType3));
            }
        }

        // subject key identifier
        if (certConfig.IsExtensionSKIDPresent())
        {
            ReturnErrorOnFailure(Crypto::Hash_SHA1(subjectPubkey, sizeof(subjectPubkey), keyid));
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_SubjectKeyIdentifier), ByteSpan(keyid)));
        }

        // authority key identifier
        if (certConfig.IsExtensionAKIDPresent())
        {
            ReturnErrorOnFailure(Crypto::Hash_SHA1(issuerPubkey, sizeof(issuerPubkey), keyid));
            ReturnErrorOnFailure(writer.Put(ContextTag(kTag_AuthorityKeyIdentifier), ByteSpan(keyid)));
        }

        for (uint8_t i = 0; i < futureExtsCount; i++)
        {
            ReturnErrorOnFailure(
                writer.Put(ContextTag(kTag_FutureExtension),
                           ByteSpan(reinterpret_cast<const uint8_t *>(futureExts[i].info), strlen(futureExts[i].info))));
        }
    }
    ReturnErrorOnFailure(writer.EndContainer(containerType2));

    // signature
    const ASN1_BIT_STRING * asn1Signature = nullptr;
    X509_get0_signature(&asn1Signature, nullptr, x509Cert);

    uint8_t signatureRawBuf[chip::Crypto::kP256_ECDSA_Signature_Length_Raw];
    MutableByteSpan signatureRaw(signatureRawBuf);
    ReturnErrorOnFailure(chip::Crypto::EcdsaAsn1SignatureToRaw(
        chip::Crypto::kP256_FE_Length, ByteSpan(asn1Signature->data, static_cast<size_t>(asn1Signature->length)), signatureRaw));

    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_ECDSASignature), signatureRaw));

    ReturnErrorOnFailure(writer.EndContainer(containerType));

    ReturnErrorOnFailure(writer.Finalize());

    chipCert.reduce_size(writer.GetLengthWritten());

    return CHIP_NO_ERROR;
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

bool MakeAttCert(AttCertType attCertType, const char * subjectCN, uint16_t subjectVID, uint16_t subjectPID,
                 bool encodeVIDandPIDasCN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom, uint32_t validDays,
                 X509 * newCert, EVP_PKEY * newKey, CertStructConfig & certConfig)
{
    bool res     = true;
    uint16_t vid = certConfig.IsSubjectVIDMismatch() ? static_cast<uint16_t>(subjectVID + 1) : subjectVID;
    uint16_t pid = certConfig.IsSubjectPIDMismatch() ? static_cast<uint16_t>(subjectPID + 1) : subjectPID;
    bool isCA    = (attCertType != kAttCertType_DAC);

    VerifyOrReturnError(subjectCN != nullptr, false);
    VerifyOrReturnError(caCert != nullptr, false);
    VerifyOrReturnError(caKey != nullptr, false);
    VerifyOrReturnError(newCert != nullptr, false);
    VerifyOrReturnError(newKey != nullptr, false);

    if (!X509_set_version(newCert, certConfig.GetCertVersion()))
    {
        ReportOpenSSLErrorAndExit("X509_set_version", res = false);
    }

    // Generate a serial number for the cert.
    res = SetCertSerialNumber(newCert);
    VerifyTrueOrExit(res);

    // Set the certificate validity time.
    res = SetValidityTime(newCert, validFrom, validDays, certConfig);
    VerifyTrueOrExit(res);

    // Set the certificate's public key.
    if (!X509_set_pubkey(newCert, newKey))
    {
        ReportOpenSSLErrorAndExit("X509_set_pubkey", res = false);
    }

    // Encode Common Name (CN) Attribute.
    {
        char cnAttrStr[chip::Crypto::kMax_CommonNameAttr_Length];
        size_t cnAttrStrLen = 0;
        if (subjectCN != nullptr)
        {
            VerifyOrReturnError(strlen(subjectCN) <= sizeof(cnAttrStr), false);
            memcpy(cnAttrStr, subjectCN, strlen(subjectCN));
            cnAttrStrLen += strlen(subjectCN);
        }

        if (encodeVIDandPIDasCN)
        {
            if (subjectVID != VendorId::NotSpecified)
            {
                // Add space to separate from the previous string.
                if (cnAttrStrLen > 0)
                {
                    VerifyOrReturnError((cnAttrStrLen + 1) <= sizeof(cnAttrStr), false);
                    cnAttrStr[cnAttrStrLen] = ' ';
                    cnAttrStrLen++;
                }

                VerifyOrReturnError((cnAttrStrLen + strlen(chip::Crypto::kVIDPrefixForCNEncoding) +
                                     chip::Crypto::kVIDandPIDHexLength) <= sizeof(cnAttrStr),
                                    false);

                memcpy(&cnAttrStr[cnAttrStrLen], chip::Crypto::kVIDPrefixForCNEncoding,
                       strlen(chip::Crypto::kVIDPrefixForCNEncoding));
                cnAttrStrLen += strlen(chip::Crypto::kVIDPrefixForCNEncoding);

                VerifyOrReturnError(Encoding::Uint16ToHex(vid, &cnAttrStr[cnAttrStrLen], chip::Crypto::kVIDandPIDHexLength,
                                                          Encoding::HexFlags::kUppercase) == CHIP_NO_ERROR,
                                    false);
                cnAttrStrLen += chip::Crypto::kVIDandPIDHexLength;
            }

            if (subjectPID != 0)
            {
                // Add space to separate from the previous string.
                if (cnAttrStrLen > 0)
                {
                    VerifyOrReturnError((cnAttrStrLen + 1) <= sizeof(cnAttrStr), false);
                    cnAttrStr[cnAttrStrLen++] = ' ';
                }

                VerifyOrReturnError((cnAttrStrLen + strlen(chip::Crypto::kPIDPrefixForCNEncoding) +
                                     chip::Crypto::kVIDandPIDHexLength) <= sizeof(cnAttrStr),
                                    false);

                memcpy(&cnAttrStr[cnAttrStrLen], chip::Crypto::kPIDPrefixForCNEncoding,
                       strlen(chip::Crypto::kPIDPrefixForCNEncoding));
                cnAttrStrLen += strlen(chip::Crypto::kPIDPrefixForCNEncoding);

                VerifyOrReturnError(Encoding::Uint16ToHex(pid, &cnAttrStr[cnAttrStrLen], chip::Crypto::kVIDandPIDHexLength,
                                                          Encoding::HexFlags::kUppercase) == CHIP_NO_ERROR,
                                    false);
                cnAttrStrLen += chip::Crypto::kVIDandPIDHexLength;
            }
        }

        // Add common name attribute to the certificate subject DN.
        if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(newCert), NID_commonName, MBSTRING_UTF8,
                                        reinterpret_cast<uint8_t *>(cnAttrStr), static_cast<int>(cnAttrStrLen), -1, 0))
        {
            ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
        }
    }

    if (!encodeVIDandPIDasCN)
    {
        // Add VID attribute to the certificate subject DN.
        if (subjectVID != VendorId::NotSpecified)
        {
            char chipAttrStr[chip::Crypto::kVIDandPIDHexLength];
            VerifyOrReturnError(Encoding::Uint16ToHex(vid, chipAttrStr, chip::Crypto::kVIDandPIDHexLength,
                                                      Encoding::HexFlags::kUppercase) == CHIP_NO_ERROR,
                                false);

            if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(newCert), gNIDChipAttAttrVID, MBSTRING_UTF8,
                                            reinterpret_cast<unsigned char *>(chipAttrStr), sizeof(chipAttrStr), -1, 0))
            {
                ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
            }
        }

        // Add PID attribute to the certificate subject DN.
        if (subjectPID != 0)
        {
            char chipAttrStr[chip::Crypto::kVIDandPIDHexLength];
            VerifyOrReturnError(Encoding::Uint16ToHex(pid, chipAttrStr, chip::Crypto::kVIDandPIDHexLength,
                                                      Encoding::HexFlags::kUppercase) == CHIP_NO_ERROR,
                                false);

            if (!X509_NAME_add_entry_by_NID(X509_get_subject_name(newCert), gNIDChipAttAttrPID, MBSTRING_UTF8,
                                            reinterpret_cast<unsigned char *>(chipAttrStr), sizeof(chipAttrStr), -1, 0))
            {
                ReportOpenSSLErrorAndExit("X509_NAME_add_entry_by_NID", res = false);
            }
        }
    }

    // Set the issuer name for the certificate. In the case of a self-signed cert, this will be
    // the new cert's subject name.
    if (!X509_set_issuer_name(newCert, X509_get_subject_name(caCert)))
    {
        ReportOpenSSLErrorAndExit("X509_set_issuer_name", res = false);
    }

    // Add basic constraints certificate extensions.
    res = SetBasicConstraintsExtension(newCert, isCA, certConfig.GetExtensionBasicPathLenValue(attCertType), certConfig);
    VerifyTrueOrExit(res);

    // Add key usage certificate extensions.
    res = SetKeyUsageExtension(newCert, isCA, certConfig);
    VerifyTrueOrExit(res);

    if (certConfig.IsExtensionSKIDPresent())
    {
        // Add a subject key id extension for the certificate.
        res = AddSubjectKeyId(newCert);
        VerifyTrueOrExit(res);
    }

    if (certConfig.IsExtensionAKIDPresent())
    {
        // Add the authority key id extension from the signing certificate.
        res = AddAuthorityKeyId(newCert, caCert);
        VerifyTrueOrExit(res);
    }

    if (certConfig.IsExtensionExtendedKeyUsagePresent())
    {
        // Add optional Extended Key Usage extentsion.
        res = AddExtension(newCert, NID_ext_key_usage, "critical,clientAuth,serverAuth");
        VerifyTrueOrExit(res);
    }

    if (certConfig.IsExtensionAuthorityInfoAccessPresent())
    {
        // Add optional Authority Informational Access extentsion.
        res = AddExtension(newCert, NID_info_access, "OCSP;URI:http://ocsp.example.com/");
        VerifyTrueOrExit(res);
    }

    if (certConfig.IsExtensionSubjectAltNamePresent())
    {
        // Add optional Subject Alternative Name extentsion.
        res = AddExtension(newCert, NID_subject_alt_name, "DNS:test.com");
        VerifyTrueOrExit(res);
    }

    // Sign the new certificate.
    if (!X509_sign(newCert, caKey, certConfig.GetSignatureAlgorithmDER()))
    {
        ReportOpenSSLErrorAndExit("X509_sign", res = false);
    }

exit:
    return res;
}
