/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#ifndef CommissionableDataProviderImpl_h
#define CommissionableDataProviderImpl_h

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/CommissionableDataProvider.h>

namespace chip {
namespace DeviceLayer {

class CommissionableDataProviderImpl : public CommissionableDataProvider
{
public:
    CommissionableDataProviderImpl() {}

    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override
    {
        if (mSetupPasscode > 0)
        {
            setupPasscode = mSetupPasscode;
        }
        else
        {
            setupPasscode = kDefaultTestPasscode;
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override
    {
        mSetupPasscode = setupPasscode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override
    {
        if (mSetupDiscriminator > 0)
        {
            setupDiscriminator = mSetupDiscriminator;
        }
        else
        {
            setupDiscriminator = kDefaultTestDiscriminator;
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override
    {
        mSetupDiscriminator = setupDiscriminator;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override
    {
        if (mSetupDiscriminator > 0)
        {
            iterationCount = mSpake2pIterationCount;
        }
        else
        {
            iterationCount = kDefaultTestVerifierIterationCount;
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSpake2pIterationCount(uint32_t iterationCount)
    {
        mSpake2pIterationCount = iterationCount;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override
    {
        size_t saltLen = mSpake2pSalt.data() == nullptr ? sizeof(kDefaultTestVerifierSalt) : mSpake2pSalt.size();
        if (saltBuf.size() < saltLen)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(saltBuf.data(), mSpake2pSalt.data() == nullptr ? kDefaultTestVerifierSalt : mSpake2pSalt.data(), saltLen);
        saltBuf.reduce_size(saltLen);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSpake2pSalt(ByteSpan saltBuf)
    {
        size_t saltLen = saltBuf.size();
        if (chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length < saltLen)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        mSpake2pSalt = MutableByteSpan(mSpake2pSaltBuf, chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length);

        memcpy(mSpake2pSalt.data(), saltBuf.data(), saltLen);
        mSpake2pSalt.reduce_size(saltLen);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & outVerifierLen) override
    {
        outVerifierLen = mSpake2pVerifier.data() == nullptr ? sizeof(kDefaultTestVerifier) : mSpake2pVerifier.size();
        if (verifierBuf.size() < outVerifierLen)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(verifierBuf.data(), mSpake2pVerifier.data() == nullptr ? kDefaultTestVerifier : mSpake2pVerifier.data(),
               outVerifierLen);
        verifierBuf.reduce_size(outVerifierLen);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSpake2pVerifier(MutableByteSpan verifierBuf)
    {
        size_t inVerifierBufLen = verifierBuf.size();
        if (chip::Crypto::kSpake2p_VerifierSerialized_Length < inVerifierBufLen)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        mSpake2pVerifier = MutableByteSpan(mSpake2pSaltBuf, chip::Crypto::kSpake2p_VerifierSerialized_Length);

        memcpy(mSpake2pVerifier.data(), verifierBuf.data(), inVerifierBufLen);
        mSpake2pVerifier.reduce_size(inVerifierBufLen);
        return CHIP_NO_ERROR;
    }

private:
    static constexpr uint32_t kDefaultTestPasscode               = 20202021;
    static constexpr uint16_t kDefaultTestDiscriminator          = 3840;
    static constexpr uint32_t kDefaultTestVerifierIterationCount = 1000;
    static constexpr uint8_t kDefaultTestVerifierSalt[16]        = {
        0x53, 0x50, 0x41, 0x4b, 0x45, 0x32, 0x50, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6c, 0x74,
    };
    static constexpr uint8_t kDefaultTestVerifier[97] = {
        0xb9, 0x61, 0x70, 0xaa, 0xe8, 0x03, 0x34, 0x68, 0x84, 0x72, 0x4f, 0xe9, 0xa3, 0xb2, 0x87, 0xc3, 0x03, 0x30, 0xc2, 0xa6,
        0x60, 0x37, 0x5d, 0x17, 0xbb, 0x20, 0x5a, 0x8c, 0xf1, 0xae, 0xcb, 0x35, 0x04, 0x57, 0xf8, 0xab, 0x79, 0xee, 0x25, 0x3a,
        0xb6, 0xa8, 0xe4, 0x6b, 0xb0, 0x9e, 0x54, 0x3a, 0xe4, 0x22, 0x73, 0x6d, 0xe5, 0x01, 0xe3, 0xdb, 0x37, 0xd4, 0x41, 0xfe,
        0x34, 0x49, 0x20, 0xd0, 0x95, 0x48, 0xe4, 0xc1, 0x82, 0x40, 0x63, 0x0c, 0x4f, 0xf4, 0x91, 0x3c, 0x53, 0x51, 0x38, 0x39,
        0xb7, 0xc0, 0x7f, 0xcc, 0x06, 0x27, 0xa1, 0xb8, 0x57, 0x3a, 0x14, 0x9f, 0xcd, 0x1f, 0xa4, 0x66, 0xcf,
    };

    uint32_t mSetupPasscode         = 0;
    uint16_t mSetupDiscriminator    = 0;
    uint32_t mSpake2pIterationCount = 0;

    uint8_t mSpake2pSaltBuf[chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length];
    MutableByteSpan mSpake2pSalt;

    MutableByteSpan mSpake2pVerifier;
    uint8_t mSpake2pVerifierBuf[chip::Crypto::kSpake2p_VerifierSerialized_Length];
};

} // namespace DeviceLayer
} // namespace chip

#endif /* CommissionableDataProviderImpl_h */
