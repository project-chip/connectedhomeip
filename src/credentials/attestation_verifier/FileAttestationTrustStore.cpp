/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "FileAttestationTrustStore.h"

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

std::vector<std::vector<uint8_t>> LoadAllX509DerCerts(const char * trustStorePath)
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
                std::vector<uint8_t> certificate(kMaxDERCertLength + 1);
                std::string filename(trustStorePath);

                filename += std::string("/") + std::string(entry->d_name);

                FILE * file = fopen(filename.c_str(), "rb");
                if (file == nullptr)
                {
                    // On bad files, just skip.
                    continue;
                }

                size_t certificateLength = fread(certificate.data(), sizeof(uint8_t), certificate.size(), file);
                if ((certificateLength > 0) && (certificateLength <= kMaxDERCertLength))
                {
                    certificate.resize(certificateLength);
                    // Only accumulate certificate if it has a subject key ID extension
                    {
                        uint8_t kidBuf[Crypto::kSubjectKeyIdentifierLength] = { 0 };
                        MutableByteSpan kidSpan{ kidBuf };
                        ByteSpan certSpan{ certificate.data(), certificate.size() };

                        if (CHIP_NO_ERROR != VerifyAttestationCertificateFormat(certSpan, Crypto::AttestationCertType::kPAA))
                        {
                            continue;
                        }

                        if (CHIP_NO_ERROR == Crypto::ExtractSKIDFromX509Cert(certSpan, kidSpan))
                        {
                            certs.push_back(certificate);
                        }
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
