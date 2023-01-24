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
#include <credentials/CHIPCertificateSet.h>
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

#ifndef CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES
#define CHIP_CONFIG_INTERNAL_FLAG_GENERATE_OP_CERT_TEST_CASES CHIP_CONFIG_TEST
#endif

#define COPYRIGHT_STRING                                                                                                           \
    "Copyright (c) 2021-2022 Project CHIP Authors. "                                                                               \
    "Copyright (c) 2019 Google LLC. "                                                                                              \
    "Copyright (c) 2013-2017 Nest Labs, Inc. "                                                                                     \
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
    kCertFormat_X509_Hex,
    kCertFormat_Chip_Raw,
    kCertFormat_Chip_Base64,
    kCertFormat_Chip_Hex,

    kCertFormat_Default = kCertFormat_Chip_Base64,
};

enum KeyFormat
{
    kKeyFormat_Unknown = 0,
    kKeyFormat_X509_DER,
    kKeyFormat_X509_PEM,
    kKeyFormat_X509_Hex,
    kKeyFormat_X509_Pubkey_PEM,
    kKeyFormat_Chip_Raw,
    kKeyFormat_Chip_Base64,
    kKeyFormat_Chip_Hex,
    kKeyFormat_Chip_Pubkey_Raw,
    kKeyFormat_Chip_Pubkey_Base64,
    kKeyFormat_Chip_Pubkey_Hex,

    kKeyFormat_Default = kKeyFormat_Chip_Base64,
};

enum DataFormat
{
    kDataFormat_Unknown = 0,
    kDataFormat_Raw,
    kDataFormat_Hex,
    kDataFormat_Base64,
};

extern bool IsChipCertFormat(CertFormat certFormat);
extern bool IsX509PrivateKeyFormat(KeyFormat keyFormat);
extern bool IsChipPrivateKeyFormat(KeyFormat keyFormat);
extern bool IsPrivateKeyFormat(KeyFormat keyFormat);
extern bool IsChipPublicKeyFormat(KeyFormat keyFormat);
extern bool IsPublicKeyFormat(KeyFormat keyFormat);

enum AttCertType
{
    kAttCertType_NotSpecified = 0, /**< The attestation certificate type has not been specified. */
    kAttCertType_PAA,              /**< Product Attestation Authority (PAA) Certificate. */
    kAttCertType_PAI,              /**< Product Attestation Intermediate (PAI) Certificate. */
    kAttCertType_DAC,              /**< Device Attestation Certificate (DAC). */
};

struct FutureExtensionWithNID
{
    int nid;
    const char * info;
};

/** Certificate Error Flags
 *
 * By default all methods (if none of the class setters were used) return valid
 * certificate configuration parameter as described in the spec.
 * These parameters can be modified to inject errors into certificate structure.
 */
class CertStructConfig
{
public:
    void EnableErrorTestCase() { mEnabled = true; }
    void SetCertVersionWrong() { mFlags.Set(CertErrorFlags::kCertVersion); }
    void SetSigAlgoWrong() { mFlags.Set(CertErrorFlags::kSigAlgo); }
    void SetSubjectVIDMismatch() { mFlags.Set(CertErrorFlags::kSubjectVIDMismatch); }
    void SetSubjectPIDMismatch() { mFlags.Set(CertErrorFlags::kSubjectPIDMismatch); }
    void SetSigCurveWrong() { mFlags.Set(CertErrorFlags::kSigCurve); }
    void SetPublicKeyError() { mFlags.Set(CertErrorFlags::kPublicKey); }
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
    void SetExtensionAKIDLengthInvalid() { mFlags.Set(CertErrorFlags::kExtAKIDLenInvalid); }
    void SetExtensionSKIDMissing() { mFlags.Set(CertErrorFlags::kExtSKIDMissing); }
    void SetExtensionSKIDLengthInvalid() { mFlags.Set(CertErrorFlags::kExtSKIDLenInvalid); }
    void SetExtensionExtendedKeyUsagePresent() { mFlags.Set(CertErrorFlags::kExtExtendedKeyUsage); }
    void SetExtensionAuthorityInfoAccessPresent() { mFlags.Set(CertErrorFlags::kExtAuthorityInfoAccess); }
    void SetExtensionSubjectAltNamePresent() { mFlags.Set(CertErrorFlags::kExtSubjectAltName); }
    void SetSignatureError() { mFlags.Set(CertErrorFlags::kSignature); }

    void SetCertOversized() { mFlags.Set(CertErrorFlags::kCertOversized); }
    void SetSerialNumberMissing() { mFlags.Set(CertErrorFlags::kSerialNumberMissing); }
    void SetIssuerMissing() { mFlags.Set(CertErrorFlags::kIssuerMissing); }
    void SetValidityNotBeforeMissing() { mFlags.Set(CertErrorFlags::kValidityNotBeforeMissing); }
    void SetValidityNotAfterMissing() { mFlags.Set(CertErrorFlags::kValidityNotAfterMissing); }
    void SetValidityWrong() { mFlags.Set(CertErrorFlags::kValidityWrong); }
    void SetSubjectMissing() { mFlags.Set(CertErrorFlags::kSubjectMissing); }
    void SetSubjectMatterIdMissing() { mFlags.Set(CertErrorFlags::kSubjectMatterIdMissing); }
    void SetSubjectNodeIdInvalid() { mFlags.Set(CertErrorFlags::kSubjectNodeIdInvalid); }
    void SetSubjectMatterIdTwice() { mFlags.Set(CertErrorFlags::kSubjectMatterIdTwice); }
    void SetSubjectFabricIdMissing() { mFlags.Set(CertErrorFlags::kSubjectFabricIdMissing); }
    void SetSubjectFabricIdInvalid() { mFlags.Set(CertErrorFlags::kSubjectFabricIdInvalid); }
    void SetSubjectFabricIdTwice() { mFlags.Set(CertErrorFlags::kSubjectFabricIdTwice); }
    void SetSubjectFabricIdMismatch() { mFlags.Set(CertErrorFlags::kSubjectFabricIdMismatch); }
    void SetSubjectCATInvalid() { mFlags.Set(CertErrorFlags::kSubjectCATInvalid); }
    void SetSubjectCATTwice() { mFlags.Set(CertErrorFlags::kSubjectCATTwice); }
    void SetExtensionExtendedKeyUsageMissing() { mFlags.Set(CertErrorFlags::kExtExtendedKeyUsageMissing); }

    bool IsErrorTestCaseEnabled() { return mEnabled; }
    int GetCertVersion() { return (mEnabled && mFlags.Has(CertErrorFlags::kCertVersion)) ? 1 : 2; }
    const EVP_MD * GetSignatureAlgorithmDER()
    {
        return (mEnabled && mFlags.Has(CertErrorFlags::kSigAlgo)) ? EVP_sha1() : EVP_sha256();
    }
    uint8_t GetSignatureAlgorithmTLVEnum()
    {
        return (mEnabled && mFlags.Has(CertErrorFlags::kSigAlgo)) ? 0x02
                                                                  : GetOIDEnum(chip::ASN1::

                                                                                   kOID_SigAlgo_ECDSAWithSHA256);
    }
    bool IsSubjectVIDMismatch() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectVIDMismatch)); }
    bool IsSubjectPIDMismatch() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectPIDMismatch)); }
    bool IsSigCurveWrong() { return (mEnabled && mFlags.Has(CertErrorFlags::kSigCurve)); }
    bool IsPublicKeyError() { return (mEnabled && mFlags.Has(CertErrorFlags::kPublicKey)); }
    bool IsExtensionBasicPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicMissing)); }
    bool IsExtensionBasicCriticalPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCriticalMissing)); }
    bool IsExtensionBasicCritical() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCriticalWrong)); }
    bool IsExtensionBasicCAPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCAMissing)); }
    bool IsExtensionBasicCACorrect() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtBasicCAWrong)); }
    int GetExtensionBasicPathLenValue(AttCertType & attCertType)
    {
        if (mEnabled && mFlags.Has(CertErrorFlags::kExtBasicPathLen0))
        {
            return 0;
        }
        if (mEnabled && mFlags.Has(CertErrorFlags::kExtBasicPathLen1))
        {
            return 1;
        }
        if (mEnabled && mFlags.Has(CertErrorFlags::kExtBasicPathLen2))
        {
            return 2;
        }
        if (mEnabled && mFlags.Has(CertErrorFlags::kExtBasicPathLenWrong))
        {
            if (attCertType == kAttCertType_DAC)
            {
                return 0;
            }
            else
            {
                return IsExtensionBasicCAPresent() ? static_cast<int>(kPathLength_NotSpecified) : 2;
            }
        }

        // Correct Values:
        if (attCertType == kAttCertType_DAC)
        {
            return IsExtensionBasicCAPresent() ? static_cast<int>(kPathLength_NotSpecified) : 0;
        }
        if (attCertType == kAttCertType_PAI)
        {
            return 0;
        }
        return 1;
    }
    bool IsExtensionBasicPathLenPresent()
    {
        return (mEnabled &&
                (mFlags.Has(CertErrorFlags::kExtBasicPathLenWrong) || mFlags.Has(CertErrorFlags::kExtBasicPathLen0) ||
                 mFlags.Has(CertErrorFlags::kExtBasicPathLen1) || mFlags.Has(CertErrorFlags::kExtBasicPathLen2)));
    }
    int GetExtensionBasicPathLenValue(uint8_t & certType)
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
        if (mFlags.Has(CertErrorFlags::kExtBasicPathLenWrong))
        {
            if (certType == chip::Credentials::kCertType_Node)
            {
                return 2;
            }
            if (certType == chip::Credentials::kCertType_ICA)
            {
                return 1;
            }
            if (certType == chip::Credentials::kCertType_Root)
            {
                return 0;
            }
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
    bool IsExtensionAKIDLengthValid() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtAKIDLenInvalid)); }
    bool IsExtensionSKIDPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtSKIDMissing)); }
    bool IsExtensionSKIDLengthValid() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kExtSKIDLenInvalid)); }
    bool IsExtensionExtendedKeyUsagePresent() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtExtendedKeyUsage)); }
    bool IsExtensionAuthorityInfoAccessPresent() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtAuthorityInfoAccess)); }
    bool IsExtensionSubjectAltNamePresent() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtSubjectAltName)); }
    bool IsSignatureError() { return (mEnabled && mFlags.Has(CertErrorFlags::kSignature)); }

    bool IsCertOversized() { return (mEnabled && mFlags.Has(CertErrorFlags::kCertOversized)); }
    uint32_t GetExtraCertLength() { return IsCertOversized() ? kExtraBufferLengthForOvesizedCert : 0; }
    bool IsSerialNumberPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSerialNumberMissing)); }
    bool IsIssuerPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kIssuerMissing)); }
    bool IsValidityNotBeforePresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kValidityNotBeforeMissing)); }
    bool IsValidityNotAfterPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kValidityNotAfterMissing)); }
    bool IsValidityCorrect() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kValidityWrong)); }
    bool IsSubjectPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSubjectMissing)); }
    bool IsSubjectMatterIdPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSubjectMatterIdMissing)); }
    bool IsSubjectNodeIdValid() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSubjectNodeIdInvalid)); }
    bool IsSubjectMatterIdRepeatsTwice() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectMatterIdTwice)); }
    bool IsSubjectFabricIdPresent() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSubjectFabricIdMissing)); }
    bool IsSubjectFabricIdValid() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSubjectFabricIdInvalid)); }
    bool IsSubjectFabricIdRepeatsTwice() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectFabricIdTwice)); }
    bool IsSubjectFabricIdMismatch() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectFabricIdMismatch)); }
    bool IsSubjectCATValid() { return (!mEnabled || !mFlags.Has(CertErrorFlags::kSubjectCATInvalid)); }
    bool IsSubjectCATRepeatsTwice() { return (mEnabled && mFlags.Has(CertErrorFlags::kSubjectCATTwice)); }
    bool IsExtensionExtendedKeyUsageMissing() { return (mEnabled && mFlags.Has(CertErrorFlags::kExtExtendedKeyUsageMissing)); }

    static constexpr uint8_t kPublicKeyErrorByte = 20;

private:
    enum class CertErrorFlags : uint64_t
    {
        kCertVersion                = 0x0000000000000001,
        kSigAlgo                    = 0x0000000000000002,
        kSubjectVIDMismatch         = 0x0000000000000004, // DA specific
        kSubjectPIDMismatch         = 0x0000000000000008, // DA specific
        kSigCurve                   = 0x0000000000000010,
        kPublicKey                  = 0x0000000000000020,
        kExtBasicMissing            = 0x0000000000000040,
        kExtBasicCriticalMissing    = 0x0000000000000080,
        kExtBasicCriticalWrong      = 0x0000000000000100,
        kExtBasicCAMissing          = 0x0000000000000200,
        kExtBasicCAWrong            = 0x0000000000000400,
        kExtBasicPathLenWrong       = 0x0000000000000800,
        kExtBasicPathLen0           = 0x0000000000001000,
        kExtBasicPathLen1           = 0x0000000000002000,
        kExtBasicPathLen2           = 0x0000000000004000,
        kExtKeyUsageMissing         = 0x0000000000008000,
        kExtKeyUsageCriticalMissing = 0x0000000000010000,
        kExtKeyUsageCriticalWrong   = 0x0000000000020000,
        kExtKeyUsageDigSig          = 0x0000000000040000,
        kExtKeyUsageKeyCertSign     = 0x0000000000080000,
        kExtKeyUsageCRLSign         = 0x0000000000100000,
        kExtAKIDMissing             = 0x0000000000200000,
        kExtAKIDLenInvalid          = 0x0000000000400000,
        kExtSKIDMissing             = 0x0000000000800000,
        kExtSKIDLenInvalid          = 0x0000000001000000,
        kExtExtendedKeyUsage        = 0x0000000002000000, // DA specific
        kExtAuthorityInfoAccess     = 0x0000000004000000, // DA specific
        kExtSubjectAltName          = 0x0000000008000000, // DA specific
        kSignature                  = 0x0000000010000000,

        // Op Cert Specific Flags:
        kCertOversized              = 0x0000000100000000,
        kSerialNumberMissing        = 0x0000000200000000,
        kIssuerMissing              = 0x0000000400000000,
        kValidityNotBeforeMissing   = 0x0000000800000000,
        kValidityNotAfterMissing    = 0x0000001000000000,
        kValidityWrong              = 0x0000002000000000,
        kSubjectMissing             = 0x0000004000000000,
        kSubjectMatterIdMissing     = 0x0000008000000000,
        kSubjectNodeIdInvalid       = 0x0000010000000000,
        kSubjectMatterIdTwice       = 0x0000020000000000,
        kSubjectFabricIdMissing     = 0x0000040000000000,
        kSubjectFabricIdInvalid     = 0x0000080000000000,
        kSubjectFabricIdTwice       = 0x0000100000000000,
        kSubjectFabricIdMismatch    = 0x0000200000000000,
        kSubjectCATInvalid          = 0x0000400000000000,
        kSubjectCATTwice            = 0x0000800000000000,
        kExtExtendedKeyUsageMissing = 0x0001000000000000,
    };

    static constexpr uint32_t kExtraBufferLengthForOvesizedCert = 300;

    bool mEnabled = false;
    chip::BitFlags<CertErrorFlags> mFlags;
};

class ToolChipDN : public chip::Credentials::ChipDN
{
public:
    bool SetCertName(X509_NAME * name) const;
    bool SetCertSubjectDN(X509 * cert) const { return SetCertName(X509_get_subject_name(cert)); };
    bool SetCertIssuerDN(X509 * cert) const { return SetCertName(X509_get_issuer_name(cert)); };
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
extern bool Cmd_PrintCD(int argc, char * argv[]);
extern bool Cmd_GenAttCert(int argc, char * argv[]);

extern bool ReadCert(const char * fileNameOrStr, std::unique_ptr<X509, void (*)(X509 *)> & cert);
extern bool ReadCert(const char * fileNameOrStr, std::unique_ptr<X509, void (*)(X509 *)> & cert, CertFormat & origCertFmt);
extern bool ReadCertDER(const char * fileNameOrStr, chip::MutableByteSpan & cert);
extern bool LoadChipCert(const char * fileNameOrStr, bool isTrused, chip::Credentials::ChipCertificateSet & certSet,
                         chip::MutableByteSpan & chipCert);

extern bool WriteCert(const char * fileName, X509 * cert, CertFormat certFmt);
extern bool WriteChipCert(const char * fileName, const chip::ByteSpan & cert, CertFormat certFmt);

extern bool MakeCert(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom,
                     uint32_t validDays, int pathLen, const FutureExtensionWithNID * futureExts, uint8_t futureExtsCount,
                     X509 * newCert, EVP_PKEY * newKey, CertStructConfig & certConfig);
extern CHIP_ERROR MakeCertTLV(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey,
                              const struct tm & validFrom, uint32_t validDays, int pathLen,
                              const FutureExtensionWithNID * futureExts, uint8_t futureExtsCount, X509 * x509Cert,
                              EVP_PKEY * newKey, CertStructConfig & certConfig, chip::MutableByteSpan & chipCert);
extern bool ResignCert(X509 * cert, X509 * caCert, EVP_PKEY * caKey);

extern bool MakeAttCert(AttCertType attCertType, const char * subjectCN, uint16_t subjectVID, uint16_t subjectPID,
                        bool encodeVIDandPIDasCN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom, uint32_t validDays,
                        X509 * newCert, EVP_PKEY * newKey, CertStructConfig & certConfig);
extern bool GenerateKeyPair(EVP_PKEY * key);
extern bool GenerateKeyPair_Secp256k1(EVP_PKEY * key);
extern bool ReadKey(const char * fileNameOrStr, std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY *)> & key,
                    bool ignorErrorIfUnsupportedCurve = false);
extern bool WriteKey(const char * fileName, EVP_PKEY * key, KeyFormat keyFmt);
extern bool SerializeKeyPair(EVP_PKEY * key, chip::Crypto::P256SerializedKeypair & serializedKeypair);

extern bool X509ToChipCert(X509 * cert, chip::MutableByteSpan & chipCert);

extern bool InitOpenSSL();
extern bool Base64Encode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen);
extern bool Base64Decode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen);
extern bool IsBase64String(const char * str, uint32_t strLen);
extern bool ContainsPEMMarker(const char * marker, const uint8_t * data, uint32_t dataLen);
extern bool ParseDateTime(const char * str, struct tm & date);
extern bool ReadFileIntoMem(const char * fileName, uint8_t * data, uint32_t & dataLen);
extern bool WriteDataIntoFile(const char * fileName, const uint8_t * data, size_t dataLen, DataFormat dataFmt);
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
