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
 *      This file defines the public API symbols, constants, and interfaces
 *      for the 'chip-cert' command line tool.
 *
 *      The 'chip-cert' tool is a command line interface (CLI) utility
 *      used, primarily, for generating and manipulating CHIP
 *      certificate and private key material.
 *
 */

#pragma once

#include <ctype.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>

#include <CHIPVersion.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/asn1/ASN1.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TimeUtils.h>

using chip::ASN1::OID;

#ifndef CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES
#define CHIP_CONFIG_INTERNAL_FLAG_GENERATE_DA_TEST_CASES CHIP_CONFIG_TEST
#endif

#define COPYRIGHT_STRING                                                                                                           \
    "Copyright (c) 2021-2022 Project CHIP Authors"                                                                                 \
    "Copyright (c) 2019 Google LLC."                                                                                               \
    "Copyright (c) 2013-2017 Nest Labs, Inc."                                                                                      \
    "All rights reserved.\n"

enum
{
    kCertValidDays_Undefined               = 0,
    kCertValidDays_NoWellDefinedExpiration = UINT32_MAX,
    kPathLength_NotSpecified               = -1,
};

enum CertFormat
{
    kCertFormat_Unknown = 0,
    kCertFormat_X509_DER,
    kCertFormat_X509_PEM,
    kCertFormat_Chip_Raw,
    kCertFormat_Chip_Base64
};

enum KeyFormat
{
    kKeyFormat_Unknown = 0,
    kKeyFormat_X509_DER,
    kKeyFormat_X509_PEM,
    kKeyFormat_X509_PUBKEY_PEM,
    kKeyFormat_Chip_Raw,
    kKeyFormat_Chip_Base64
};

enum AttCertType
{
    kAttCertType_NotSpecified = 0, /**< The attestation certificate type has not been specified. */
    kAttCertType_PAA,              /**< Product Attestation Authority (PAA) Certificate. */
    kAttCertType_PAI,              /**< Product Attestation Intermediate (PAI) Certificate. */
    kAttCertType_DAC,              /**< Device Attestation Certificate (DAC). */
};

struct FutureExtension
{
    int nid;
    const char * info;
};

/** Attestation Certificate Error Flags
 *
 * By default all methods (if none of the class setters were used) return valid
 * attestation certificate configuration parameter as described in the spec.
 * These parameters can be modified to inject errors into certificate structure.
 */
class AttCertStructConfig
{
public:
    void EnableErrorTestCase() { mEnabled = true; }
    void SetCertVersionWrong() { mFlags.Set(CertErrorFlags::kCertVersion); }
    void SetSigAlgoWrong() { mFlags.Set(CertErrorFlags::kSigAlgo); }
    void SetSubjectVIDMismatch() { mFlags.Set(CertErrorFlags::kSubjectVIDMismatch); }
    void SetSubjectPIDMismatch() { mFlags.Set(CertErrorFlags::kSubjectPIDMismatch); }
    void SetSigCurveWrong() { mFlags.Set(CertErrorFlags::kSigCurve); }
    void SetExtensionBasicMissing() { mFlags.Set(CertErrorFlags::kExtBasicMissing); }
    void SetExtensionBasicCriticalMissing() { mFlags.Set(CertErrorFlags::kExtBasicCriticalMissing); }
    void SetExtensionBasicCriticalWrong() { mFlags.Set(CertErrorFlags::kExtBasicCriticalWrong); }
    void SetExtensionBasicCAMissing() { mFlags.Set(CertErrorFlags::kExtBasicCAMissing); }
    void SetExtensionBasicCAWrong() { mFlags.Set(CertErrorFlags::kExtBasicCAWrong); }
    void SetExtensionBasicPathLenPresenceWrong() { mFlags.Set(CertErrorFlags::kExtBasicPathLenWrong); }
    void SetExtensionBasicPathLen0() { mFlags.Set(CertErrorFlags::kExtBasicPathLen0); }
    void SetExtensionBasicPathLen1() { mFlags.Set(CertErrorFlags::kExtBasicPathLen1); }
    void SetExtensionBasicPathLen2() { mFlags.Set(CertErrorFlags::kExtBasicPathLen2); }
    void SetExtensionKeyUsageMissing() { mFlags.Set(CertErrorFlags::kExtKeyUsageMissing); }
    void SetExtensionKeyUsageCriticalMissing() { mFlags.Set(CertErrorFlags::kExtKeyUsageCriticalMissing); }
    void SetExtensionKeyUsageCriticalWrong() { mFlags.Set(CertErrorFlags::kExtKeyUsageCriticalWrong); }
    void SetExtensionKeyUsageDigitalSigWrong() { mFlags.Set(CertErrorFlags::kExtKeyUsageDigSig); }
    void SetExtensionKeyUsageKeyCertSignWrong() { mFlags.Set(CertErrorFlags::kExtKeyUsageKeyCertSign); }
    void SetExtensionKeyUsageCRLSignWrong() { mFlags.Set(CertErrorFlags::kExtKeyUsageCRLSign); }
    void SetExtensionAKIDMissing() { mFlags.Set(CertErrorFlags::kExtAKIDMissing); }
    void SetExtensionSKIDMissing() { mFlags.Set(CertErrorFlags::kExtSKIDMissing); }
    void SetExtensionExtendedKeyUsagePresent() { mFlags.Set(CertErrorFlags::kExtExtendedKeyUsage); }
    void SetExtensionAuthorityInfoAccessPresent() { mFlags.Set(CertErrorFlags::kExtAuthorityInfoAccess); }
    void SetExtensionSubjectAltNamePresent() { mFlags.Set(CertErrorFlags::kExtSubjectAltName); }

    bool IsErrorTestCaseEnabled() { return mEnabled; }
    int GetCertVersion() { return (mEnabled && mFlags.Has(CertErrorFlags::kCertVersion)) ? 1 : 2; }
    const EVP_MD * GetSignatureAlgorithm()
    {
        return (mEnabled && mFlags.Has(CertErrorFlags::kSigAlgo)) ? EVP_sha1() : EVP_sha256();
    }
    bool IsSubjectVIDMismatch() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectVIDMismatch)); }
    bool IsSubjectPIDMismatch() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectPIDMismatch)); }
    bool IsSigCurveWrong() { return (mEnabled && mFlags.Has(CertErrorFlags::kSigCurve)); }
    bool IsExtensionBasicPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicMissing)); }
    bool IsExtensionBasicCriticalPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCriticalMissing)); }
    bool IsExtensionBasicCritical() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCriticalWrong)); }
    bool IsExtensionBasicCAPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCAMissing)); }
    bool IsExtensionBasicCACorrect() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCAWrong)); }
    bool IsExtensionBasicPathLenPresent(AttCertType & attCertType)
    {
        bool normallyPresent = (attCertType != kAttCertType_DAC);
        bool testCaseWrong   = (mEnabled && mFlags.Has(CertErrorFlags::kExtBasicPathLenWrong));
        return (normallyPresent ^ testCaseWrong);
    }
    int GetExtensionBasicPathLenValue(AttCertType & attCertType)
    {
        if (mFlags.Has(CertErrorFlags::kExtBasicPathLen0))
        {
            return 0;
        }
        if (mFlags.Has(CertErrorFlags::kExtBasicPathLen1))
        {
            return 1;
        }
        if (mFlags.Has(CertErrorFlags::kExtBasicPathLen2))
        {
            return 2;
        }
        if (attCertType == kAttCertType_PAA)
        {
            return 1;
        }
        if (attCertType == kAttCertType_PAI)
        {
            return 0;
        }
        return 0;
    }
    bool IsExtensionKeyUsagePresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtKeyUsageMissing)); }
    bool IsExtensionKeyUsageCriticalPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtKeyUsageCriticalMissing)); }
    bool IsExtensionKeyUsageCritical() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtKeyUsageCriticalWrong)); }
    bool IsExtensionKeyUsageDigitalSigCorrect() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtKeyUsageDigSig)); }
    bool IsExtensionKeyUsageKeyCertSignCorrect() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtKeyUsageKeyCertSign)); }
    bool IsExtensionKeyUsageCRLSignCorrect() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtKeyUsageCRLSign)); }
    bool IsExtensionAKIDPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtAKIDMissing)); }
    bool IsExtensionSKIDPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtSKIDMissing)); }
    bool IsExtensionExtendedKeyUsagePresent() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtExtendedKeyUsage)); }
    bool IsExtensionAuthorityInfoAccessPresent() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtAuthorityInfoAccess)); }
    bool IsExtensionSubjectAltNamePresent() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtSubjectAltName)); }

private:
    enum class CertErrorFlags : uint32_t
    {
        kCertVersion                = 0x00000001,
        kSigAlgo                    = 0x00000002,
        kSubjectVIDMismatch         = 0x00000004,
        kSubjectPIDMismatch         = 0x00000008,
        kSigCurve                   = 0x00000010,
        kExtBasicMissing            = 0x00000020,
        kExtBasicCriticalMissing    = 0x00000040,
        kExtBasicCriticalWrong      = 0x00000080,
        kExtBasicCAMissing          = 0x00000100,
        kExtBasicCAWrong            = 0x00000200,
        kExtBasicPathLenWrong       = 0x00000400,
        kExtBasicPathLen0           = 0x00000800,
        kExtBasicPathLen1           = 0x00001000,
        kExtBasicPathLen2           = 0x00002000,
        kExtKeyUsageMissing         = 0x00004000,
        kExtKeyUsageCriticalMissing = 0x00008000,
        kExtKeyUsageCriticalWrong   = 0x00010000,
        kExtKeyUsageDigSig          = 0x00020000,
        kExtKeyUsageKeyCertSign     = 0x00040000,
        kExtKeyUsageCRLSign         = 0x00080000,
        kExtAKIDMissing             = 0x00100000,
        kExtSKIDMissing             = 0x00200000,
        kExtExtendedKeyUsage        = 0x00400000,
        kExtAuthorityInfoAccess     = 0x00800000,
        kExtSubjectAltName          = 0x01000000,
    };

    bool mEnabled = false;
    chip::BitFlags<CertErrorFlags> mFlags;
};

class ToolChipDN : public chip::Credentials::ChipDN
{
public:
    bool SetCertSubjectDN(X509 * cert) const;
    bool HasAttr(chip::ASN1::OID oid) const;
    void PrintDN(FILE * file, const char * name) const;
};

extern bool Cmd_GenCD(int argc, char * argv[]);
extern bool Cmd_GenCert(int argc, char * argv[]);
extern bool Cmd_ConvertCert(int argc, char * argv[]);
extern bool Cmd_ConvertKey(int argc, char * argv[]);
extern bool Cmd_ResignCert(int argc, char * argv[]);
extern bool Cmd_ValidateAttCert(int argc, char * argv[]);
extern bool Cmd_ValidateCert(int argc, char * argv[]);
extern bool Cmd_PrintCert(int argc, char * argv[]);
extern bool Cmd_GenAttCert(int argc, char * argv[]);

extern bool ReadCert(const char * fileName, X509 * cert);
extern bool ReadCert(const char * fileName, X509 * cert, CertFormat & origCertFmt);
extern bool ReadCertDERRaw(const char * fileName, chip::MutableByteSpan & cert);
extern bool LoadChipCert(const char * fileName, bool isTrused, chip::Credentials::ChipCertificateSet & certSet,
                         chip::MutableByteSpan & chipCert);

extern bool WriteCert(const char * fileName, X509 * cert, CertFormat certFmt);

extern bool MakeCert(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom,
                     uint32_t validDays, int pathLen, const FutureExtension * futureExts, uint8_t futureExtsCount, X509 * newCert,
                     EVP_PKEY * newKey);
extern bool ResignCert(X509 * cert, X509 * caCert, EVP_PKEY * caKey);

extern bool MakeAttCert(AttCertType attCertType, const char * subjectCN, uint16_t subjectVID, uint16_t subjectPID,
                        bool encodeVIDandPIDasCN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom, uint32_t validDays,
                        X509 * newCert, EVP_PKEY * newKey, AttCertStructConfig & certConfig);
extern bool GenerateKeyPair(EVP_PKEY * key);
extern bool GenerateKeyPair_Secp256k1(EVP_PKEY * key);
extern bool ReadKey(const char * fileName, EVP_PKEY * key, bool ignorErrorIfUnsupportedCurve = false);
extern bool WritePrivateKey(const char * fileName, EVP_PKEY * key, KeyFormat keyFmt);
extern bool SerializeKeyPair(EVP_PKEY * key, chip::Crypto::P256SerializedKeypair & serializedKeypair);

extern bool X509ToChipCert(X509 * cert, chip::MutableByteSpan & chipCert);

extern bool InitOpenSSL();
extern bool Base64Encode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen);
extern bool Base64Decode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen);
extern bool IsBase64String(const char * str, uint32_t strLen);
extern bool ContainsPEMMarker(const char * marker, const uint8_t * data, uint32_t dataLen);
extern bool ParseDateTime(const char * str, struct tm & date);
extern bool ReadFileIntoMem(const char * fileName, uint8_t * data, uint32_t & dataLen);
extern bool OpenFile(const char * fileName, FILE *& file, bool toWrite = false);
extern void CloseFile(FILE *& file);

extern int gNIDChipNodeId;
extern int gNIDChipFirmwareSigningId;
extern int gNIDChipICAId;
extern int gNIDChipRootId;
extern int gNIDChipFabricId;
extern int gNIDChipCASEAuthenticatedTag;
extern int gNIDChipCurveP256;
extern int gNIDTestCurveSecp256k1;
extern int gNIDChipAttAttrVID;
extern int gNIDChipAttAttrPID;

/**
 *  @def VerifyTrueOrExit(aStatus)
 *
 *  @brief
 *    Checks for the specified status, which is expected to be 'true',
 *    and branches to the local label 'exit' if the status is 'false'.
 *
 *  @param[in]  aStatus     A boolean status to be evaluated.
 */
#define VerifyTrueOrExit(aStatus) nlEXPECT(aStatus, exit)

/**
 *  @def ReportOpenSSLErrorAndExit(aFunct, ACTION)
 *
 *  @brief
 *    Prints error, which was result of execusion of the specified OpenSSL
 *    function, then performs specified actions and branches to the local label 'exit'.
 *
 *  @param[in]  aFunct      Name of an OpenSSL function that reported an error.
 */
#define ReportOpenSSLErrorAndExit(aFunct, ...)                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        fprintf(stderr, "OpenSSL %s() failed\n", aFunct);                                                                          \
        ERR_print_errors_fp(stderr);                                                                                               \
        __VA_ARGS__;                                                                                                               \
        goto exit;                                                                                                                 \
    } while (0)
