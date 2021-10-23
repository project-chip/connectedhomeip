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

#define COPYRIGHT_STRING                                                                                                           \
    "Copyright (c) 2021 Project CHIP Authors.\nCopyright (c) 2019 Google LLC.\nCopyright (c) 2013-2017 Nest Labs, Inc.\nAll "      \
    "rights reserved.\n"

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

class ToolChipDN : public chip::Credentials::ChipDN
{
public:
    bool SetCertSubjectDN(X509 * cert) const;
    bool HasAttr(chip::ASN1::OID oid) const;
    void PrintDN(FILE * file, const char * name) const;
};

extern bool Cmd_GenCert(int argc, char * argv[]);
extern bool Cmd_ConvertCert(int argc, char * argv[]);
extern bool Cmd_ConvertKey(int argc, char * argv[]);
extern bool Cmd_ResignCert(int argc, char * argv[]);
extern bool Cmd_ValidateCert(int argc, char * argv[]);
extern bool Cmd_PrintCert(int argc, char * argv[]);
extern bool Cmd_GenAttCert(int argc, char * argv[]);

extern bool ReadCert(const char * fileName, X509 * cert);
extern bool ReadCert(const char * fileName, X509 * cert, CertFormat & origCertFmt);
extern bool LoadChipCert(const char * fileName, bool isTrused, chip::Credentials::ChipCertificateSet & certSet,
                         chip::MutableByteSpan & chipCert);

extern bool WriteCert(const char * fileName, X509 * cert, CertFormat certFmt);

extern bool MakeCert(uint8_t certType, const ToolChipDN * subjectDN, X509 * caCert, EVP_PKEY * caKey, const struct tm & validFrom,
                     uint32_t validDays, int pathLen, const FutureExtension * futureExts, uint8_t futureExtsCount, X509 * newCert,
                     EVP_PKEY * newKey);
extern bool ResignCert(X509 * cert, X509 * caCert, EVP_PKEY * caKey);

extern bool MakeAttCert(AttCertType attCertType, const char * subjectCN, uint16_t subjectVID, uint16_t subjectPID, X509 * caCert,
                        EVP_PKEY * caKey, const struct tm & validFrom, uint32_t validDays, X509 * newCert, EVP_PKEY * newKey);

extern bool GenerateKeyPair(EVP_PKEY * key);
extern bool ReadKey(const char * fileName, EVP_PKEY * key);
extern bool WritePrivateKey(const char * fileName, EVP_PKEY * key, KeyFormat keyFmt);

extern bool X509ToChipCert(X509 * cert, chip::MutableByteSpan & chipCert);

extern bool InitOpenSSL();
extern bool Base64Encode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen);
extern bool Base64Decode(const uint8_t * inData, uint32_t inDataLen, uint8_t * outBuf, uint32_t outBufSize, uint32_t & outDataLen);
extern bool IsBase64String(const char * str, uint32_t strLen);
extern bool ContainsPEMMarker(const char * marker, const uint8_t * data, uint32_t dataLen);
extern bool ParseChip64bitAttr(const char * str, uint64_t & val);
extern bool ParseDateTime(const char * str, struct tm & date);
extern bool ReadFileIntoMem(const char * fileName, uint8_t * data, uint32_t & dataLen);
extern bool OpenFile(const char * fileName, FILE *& file, bool toWrite = false);
extern void CloseFile(FILE *& file);

extern int gNIDChipNodeId;
extern int gNIDChipFirmwareSigningId;
extern int gNIDChipICAId;
extern int gNIDChipRootId;
extern int gNIDChipFabricId;
extern int gNIDChipAuthTag1;
extern int gNIDChipAuthTag2;
extern int gNIDChipCurveP256;
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
