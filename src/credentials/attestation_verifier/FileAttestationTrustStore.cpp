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
    DIR * dir;

    dir = opendir(paaTrustStorePath);
    if (dir != nullptr)
    {
        // Nested directories are not handled.
        dirent * entry;
        while ((entry = readdir(dir)) != nullptr)
        {
            const char * fileExtension = GetFilenameExtension(entry->d_name);
            if (strncmp(fileExtension, "der", strlen("der")) == 0)
            {
                FILE * file;

                std::array<uint8_t, kMaxDERCertLength> certificate;
                std::string filename(paaTrustStorePath);

                filename += std::string("/") + std::string(entry->d_name);

                file = fopen(filename.c_str(), "rb");
                if (file != nullptr)
                {
                    uint32_t certificateLength = fread(certificate.data(), sizeof(uint8_t), kMaxDERCertLength, file);
                    if (certificateLength > 0)
                    {
                        mDerCerts.push_back(certificate);
                        mIsInitialized = true;
                    }
                    fclose(file);
                }
                else
                {
                    Cleanup();
                    break;
                }
            }
        }
        closedir(dir);
    }
}

FileAttestationTrustStore::~FileAttestationTrustStore()
{
    Cleanup();
}

void FileAttestationTrustStore::Cleanup()
{
    mDerCerts.clear();
    mIsInitialized = false;
}

CHIP_ERROR FileAttestationTrustStore::GetProductAttestationAuthorityCert(const ByteSpan & skid,
                                                                         MutableByteSpan & outPaaDerBuffer) const
{
    VerifyOrReturnError(!mDerCerts.empty(), CHIP_ERROR_CA_CERT_NOT_FOUND);
    VerifyOrReturnError(!skid.empty() && (skid.data() != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(skid.size() == Crypto::kSubjectKeyIdentifierLength, CHIP_ERROR_INVALID_ARGUMENT);

    for (auto candidate : mDerCerts)
    {
        uint8_t skidBuf[Crypto::kSubjectKeyIdentifierLength] = { 0 };
        MutableByteSpan candidateSkidSpan{ skidBuf };
        VerifyOrReturnError(CHIP_NO_ERROR ==
                                Crypto::ExtractSKIDFromX509Cert(ByteSpan{ candidate.data(), candidate.size() }, candidateSkidSpan),
                            CHIP_ERROR_INTERNAL);

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
