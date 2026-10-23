/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "FileAttestationTrustStore.h"

#include <algorithm>
#include <array>
#include <crypto/CHIPCryptoPAL.h>
#include <cstdio>
#include <cstring>
#include <string>

extern "C" {
#include <dirent.h>
}

namespace chip {
namespace Credentials {

namespace {

enum class CertificateFamily
{
    kP256,
    kMlDsa44,
    kMlDsa65,
    kUnknown,
};

constexpr size_t kCertTypeProbeLength = 512;

// Keep unknown algorithms permissive to avoid skipping future cert profiles.
constexpr size_t kMaxUnknownDerCertLength = kMaxDERCertLengthMlDsa65;

// DER-encoded OIDs for certificate signature algorithms.
constexpr uint8_t kOidEcdsaWithSha256[] = { 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02 };
constexpr uint8_t kOidMlDsa44[]         = { 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x03, 0x11 };
constexpr uint8_t kOidMlDsa65[]         = { 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x03, 0x12 };

bool ContainsSequence(const uint8_t * haystack, size_t haystackLen, const uint8_t * needle, size_t needleLen)
{
    if (haystack == nullptr || needle == nullptr || needleLen == 0 || haystackLen < needleLen)
    {
        return false;
    }

    const uint8_t * begin = haystack;
    const uint8_t * end   = haystack + haystackLen;
    return std::search(begin, end, needle, needle + needleLen) != end;
}

CertificateFamily DetermineCertificateFamilyFromPrefix(const uint8_t * prefix, size_t prefixLen)
{
    if (ContainsSequence(prefix, prefixLen, kOidMlDsa44, sizeof(kOidMlDsa44)))
    {
        return CertificateFamily::kMlDsa44;
    }

    if (ContainsSequence(prefix, prefixLen, kOidMlDsa65, sizeof(kOidMlDsa65)))
    {
        return CertificateFamily::kMlDsa65;
    }

    if (ContainsSequence(prefix, prefixLen, kOidEcdsaWithSha256, sizeof(kOidEcdsaWithSha256)))
    {
        return CertificateFamily::kP256;
    }

    return CertificateFamily::kUnknown;
}

CertificateFamily DetermineCertificateFamilyFromFile(const std::string & filename)
{
    std::array<uint8_t, kCertTypeProbeLength> prefix = { 0 };

    FILE * file = fopen(filename.c_str(), "rb");
    if (file == nullptr)
    {
        return CertificateFamily::kUnknown;
    }

    const size_t bytesRead = fread(prefix.data(), sizeof(uint8_t), prefix.size(), file);
    fclose(file);

    return DetermineCertificateFamilyFromPrefix(prefix.data(), bytesRead);
}

size_t MaxDerCertLengthForFamily(CertificateFamily family)
{
    switch (family)
    {
    case CertificateFamily::kP256:
        return kMaxDERCertLength;
    case CertificateFamily::kMlDsa44:
        return kMaxDERCertLengthMlDsa44;
    case CertificateFamily::kMlDsa65:
        return kMaxDERCertLengthMlDsa65;
    case CertificateFamily::kUnknown:
    default:
        return kMaxUnknownDerCertLength;
    }
}

const char * GetFilenameExtension(const char * filename)
{
    const char * dot = strrchr(filename, '.');
    if (!dot || dot == filename)
    {
        return "";
    }
    return dot + 1;
}
} // namespace

FileAttestationTrustStore::FileAttestationTrustStore(const char * paaTrustStorePath)
{
    VerifyOrReturn(paaTrustStorePath != nullptr);

    if (paaTrustStorePath != nullptr)
    {
        mPAADerCerts = LoadAllX509DerCerts(paaTrustStorePath);
        VerifyOrReturn(paaCount());
    }

    mIsInitialized = true;
}

std::vector<std::vector<uint8_t>> LoadAllX509DerCerts(const char * trustStorePath, CertificateValidationMode validationMode)
{
    std::vector<std::vector<uint8_t>> certs;
    if (trustStorePath == nullptr)
    {
        return certs;
    }

    DIR * dir;

    dir = opendir(trustStorePath);
    if (dir != nullptr)
    {
        // Nested directories are not handled.
        dirent * entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            const char * fileExtension = GetFilenameExtension(entry->d_name);
            if (strncmp(fileExtension, "der", strlen("der")) == 0)
            {
                std::string filename(trustStorePath);

                filename += std::string("/") + std::string(entry->d_name);

                const CertificateFamily certificateFamily = DetermineCertificateFamilyFromFile(filename);
                const size_t maxDerCertLengthForFile      = MaxDerCertLengthForFamily(certificateFamily);
                std::vector<uint8_t> certificate(maxDerCertLengthForFile + 1);

                FILE * file = fopen(filename.c_str(), "rb");
                if (file == nullptr)
                {
                    // On bad files, just skip.
                    continue;
                }

                size_t certificateLength = fread(certificate.data(), sizeof(uint8_t), certificate.size(), file);
                if ((certificateLength > 0) && (certificateLength <= maxDerCertLengthForFile))
                {
                    certificate.resize(certificateLength);
                    ByteSpan certSpan{ certificate.data(), certificate.size() };

                    // Only accumulate certificate if it passes validation.
                    bool isValid = false;
                    switch (validationMode)
                    {
                    case CertificateValidationMode::kPAA: {
                        if (CHIP_NO_ERROR != VerifyAttestationCertificateFormat(certSpan, Crypto::AttestationCertType::kPAA))
                        {
                            break;
                        }

                        uint8_t kidBuf[Crypto::kSubjectKeyIdentifierLength] = { 0 };
                        MutableByteSpan kidSpan{ kidBuf };
                        if (CHIP_NO_ERROR == Crypto::ExtractSKIDFromX509Cert(certSpan, kidSpan))
                        {
                            isValid = true;
                        }
                        break;
                    }
                    case CertificateValidationMode::kPublicKeyOnly: {
                        Crypto::P256PublicKey publicKey;
                        if (CHIP_NO_ERROR == Crypto::ExtractPubkeyFromX509Cert(certSpan, publicKey))
                        {
                            isValid = true;
                        }
                        break;
                    }
                    }

                    if (isValid)
                    {
                        certs.push_back(certificate);
                    }
                }
                fclose(file);
            }
        }
        closedir(dir);
    }

    return certs;
}

FileAttestationTrustStore::~FileAttestationTrustStore()
{
    Cleanup();
}

void FileAttestationTrustStore::Cleanup()
{
    mPAADerCerts.clear();
    mIsInitialized = false;
}

CHIP_ERROR FileAttestationTrustStore::GetProductAttestationAuthorityCert(const ByteSpan & skid,
                                                                         MutableByteSpan & outPaaDerBuffer) const
{
    // If the constructor has not tried to initialize the PAA certificates database, return CHIP_ERROR_NOT_IMPLEMENTED to use the
    // testing trust store if the DefaultAttestationVerifier is in use.
    if (mIsInitialized && paaCount() == 0)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    VerifyOrReturnError(!mPAADerCerts.empty(), CHIP_ERROR_CA_CERT_NOT_FOUND);
    VerifyOrReturnError(!skid.empty() && (skid.data() != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(skid.size() == Crypto::kSubjectKeyIdentifierLength, CHIP_ERROR_INVALID_ARGUMENT);

    for (auto candidate : mPAADerCerts)
    {
        uint8_t skidBuf[Crypto::kSubjectKeyIdentifierLength] = { 0 };
        MutableByteSpan candidateSkidSpan{ skidBuf };
        if (CHIP_NO_ERROR != Crypto::ExtractSKIDFromX509Cert(ByteSpan{ candidate.data(), candidate.size() }, candidateSkidSpan))
        {
            continue;
        }

        if (skid.data_equal(candidateSkidSpan))
        {
            // Found a match
            return CopySpanToMutableSpan(ByteSpan{ candidate.data(), candidate.size() }, outPaaDerBuffer);
        }
    }

    return CHIP_ERROR_CA_CERT_NOT_FOUND;
}

} // namespace Credentials
} // namespace chip
