/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Credentials {

class TestOnlyLocalCertificateAuthority
{
public:
    TestOnlyLocalCertificateAuthority()
    {
        // Initializing the default start validity to start of 2021.
        chip::ASN1::ASN1UniversalTime effectiveTime;
        CHIP_ZERO_AT(effectiveTime);
        effectiveTime.Year  = 2021;
        effectiveTime.Month = 1;
        effectiveTime.Day   = 1;
        VerifyOrDie(ASN1ToChipEpochTime(effectiveTime, mNow) == CHIP_NO_ERROR);
    }

    ~TestOnlyLocalCertificateAuthority()
    {
        mRootKeypair.reset();
        mLastRcac.Free();
        mLastNoc.Free();
        mLastIcac.Free();
    }

    // Non-copyable
    TestOnlyLocalCertificateAuthority(TestOnlyLocalCertificateAuthority const &) = delete;
    void operator=(TestOnlyLocalCertificateAuthority const &) = delete;

    TestOnlyLocalCertificateAuthority & Init()
    {
        Crypto::P256SerializedKeypair emptyKeypair;
        return Init(emptyKeypair);
    }

    TestOnlyLocalCertificateAuthority & Init(Crypto::P256SerializedKeypair & rootKeyPair)
    {
        SuccessOrExit(mCurrentStatus);

        mRootKeypair = Platform::MakeUnique<Crypto::P256Keypair>();
        VerifyOrExit(mRootKeypair != nullptr, mCurrentStatus = CHIP_ERROR_NO_MEMORY);

        if (rootKeyPair.Length() != 0)
        {
            mCurrentStatus = mRootKeypair->Deserialize(rootKeyPair);
            SuccessOrExit(mCurrentStatus);
        }
        else
        {
            mRootKeypair->Initialize(Crypto::ECPKeyTarget::ECDSA);
        }
        mCurrentStatus = GenerateRootCert(*mRootKeypair.get());
        SuccessOrExit(mCurrentStatus);
    exit:
        return *this;
    }

    TestOnlyLocalCertificateAuthority & SetIncludeIcac(bool includeIcac)
    {
        mIncludeIcac   = includeIcac;
        mCurrentStatus = (mCurrentStatus != CHIP_NO_ERROR) ? mCurrentStatus : CHIP_NO_ERROR;
        return *this;
    }

    void ResetIssuer()
    {
        mCurrentStatus = CHIP_NO_ERROR;
        mIncludeIcac   = false;
        mLastNoc.Free();
        mLastIcac.Free();
    }

    CHIP_ERROR GetStatus() { return mCurrentStatus; }
    bool IsSuccess() { return mCurrentStatus == CHIP_NO_ERROR; }

    ByteSpan GetNoc() const { return ByteSpan{ mLastNoc.Get(), mLastNoc.AllocatedSize() }; }
    ByteSpan GetIcac() const
    {
        return mIncludeIcac ? ByteSpan{ mLastIcac.Get(), mLastIcac.AllocatedSize() } : ByteSpan{ nullptr, 0 };
    }
    ByteSpan GetRcac() const { return ByteSpan{ mLastRcac.Get(), mLastRcac.AllocatedSize() }; }

    TestOnlyLocalCertificateAuthority & GenerateNocChain(FabricId fabricId, NodeId nodeId,
                                                         const Crypto::P256PublicKey & nocPublicKey)
    {
        if (mCurrentStatus != CHIP_NO_ERROR)
        {
            return *this;
        }

        if (mRootKeypair.get() == nullptr)
        {
            mCurrentStatus = CHIP_ERROR_NO_SHARED_TRUSTED_ROOT;
            return *this;
        }

        mLastIcac.Free();
        mLastNoc.Free();
        mCurrentStatus = GenerateCertChainInternal(fabricId, nodeId, nocPublicKey);
        return *this;
    }

    TestOnlyLocalCertificateAuthority & GenerateNocChain(FabricId fabricId, NodeId nodeId, const ByteSpan & csr)
    {
        if (mCurrentStatus != CHIP_NO_ERROR)
        {
            return *this;
        }

        Crypto::P256PublicKey nocPublicKey;
        mCurrentStatus = Crypto::VerifyCertificateSigningRequest(csr.data(), csr.size(), nocPublicKey);
        if (mCurrentStatus != CHIP_NO_ERROR)
        {
            return *this;
        }

        return GenerateNocChain(fabricId, nodeId, nocPublicKey);
    }

protected:
    CHIP_ERROR GenerateCertChainInternal(FabricId fabricId, NodeId nodeId, const Crypto::P256PublicKey & nocPublicKey)
    {
        ChipDN rcac_dn;
        ChipDN icac_dn;
        ChipDN noc_dn;

        // Get subject DN of RCAC as our issuer field for ICAC and/or NOC depending on if ICAC is present
        ReturnErrorOnFailure(ExtractSubjectDNFromChipCert(ByteSpan{ mLastRcac.Get(), mLastRcac.AllocatedSize() }, rcac_dn));

        Crypto::P256Keypair icacKeypair;
        ReturnErrorOnFailure(icacKeypair.Initialize(Crypto::ECPKeyTarget::ECDSA)); // Maybe we won't use it, but it's OK

        Crypto::P256Keypair * nocIssuerKeypair = mRootKeypair.get();
        ChipDN * issuer_dn                     = &rcac_dn;

        // Generate ICAC if needed
        if (mIncludeIcac)
        {
            Platform::ScopedMemoryBufferWithSize<uint8_t> icacDerBuf;
            ReturnErrorCodeIf(!icacDerBuf.Alloc(Credentials::kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
            Platform::ScopedMemoryBufferWithSize<uint8_t> icacChipBuf;
            ReturnErrorCodeIf(!icacChipBuf.Alloc(Credentials::kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(icac_dn.AddAttribute_MatterFabricId(fabricId));
            ReturnErrorOnFailure(icac_dn.AddAttribute_MatterICACId(1234));

            X509CertRequestParams icac_request = { 0, mNow, mNow + mValidity, icac_dn, rcac_dn };

            MutableByteSpan icacDerSpan{ icacDerBuf.Get(), icacDerBuf.AllocatedSize() };
            ReturnErrorOnFailure(Credentials::NewICAX509Cert(icac_request, icacKeypair.Pubkey(), *mRootKeypair.get(), icacDerSpan));

            MutableByteSpan icacChipSpan{ icacChipBuf.Get(), icacChipBuf.AllocatedSize() };
            ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(icacDerSpan, icacChipSpan));

            ReturnErrorCodeIf(!mLastIcac.Alloc(icacChipSpan.size()), CHIP_ERROR_NO_MEMORY);

            memcpy(mLastIcac.Get(), icacChipSpan.data(), icacChipSpan.size());

            nocIssuerKeypair = &icacKeypair;
            issuer_dn        = &icac_dn;
        }

        // Generate NOC always, either issued from ICAC if present or from RCAC
        {
            Platform::ScopedMemoryBufferWithSize<uint8_t> nocDerBuf;
            ReturnErrorCodeIf(!nocDerBuf.Alloc(Credentials::kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
            Platform::ScopedMemoryBufferWithSize<uint8_t> nocChipBuf;
            ReturnErrorCodeIf(!nocChipBuf.Alloc(Credentials::kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

            ReturnErrorOnFailure(noc_dn.AddAttribute_MatterFabricId(fabricId));
            ReturnErrorOnFailure(noc_dn.AddAttribute_MatterNodeId(nodeId));

            X509CertRequestParams noc_request = { 0, mNow, mNow + mValidity, noc_dn, *issuer_dn };

            MutableByteSpan nocDerSpan{ nocDerBuf.Get(), nocDerBuf.AllocatedSize() };
            ReturnErrorOnFailure(Credentials::NewNodeOperationalX509Cert(noc_request, nocPublicKey, *nocIssuerKeypair, nocDerSpan));

            MutableByteSpan nocChipSpan{ nocChipBuf.Get(), nocChipBuf.AllocatedSize() };
            ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(nocDerSpan, nocChipSpan));

            ReturnErrorCodeIf(!mLastNoc.Alloc(nocChipSpan.size()), CHIP_ERROR_NO_MEMORY);

            memcpy(mLastNoc.Get(), nocChipSpan.data(), nocChipSpan.size());
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GenerateRootCert(Crypto::P256Keypair & rootKeyPair)
    {
        ChipDN rcac_dn;
        const uint64_t kIssuerId = 1234567;

        Platform::ScopedMemoryBufferWithSize<uint8_t> rcacDerBuf;
        VerifyOrReturnError(rcacDerBuf.Alloc(Credentials::kMaxDERCertLength), CHIP_ERROR_NO_MEMORY);
        Platform::ScopedMemoryBufferWithSize<uint8_t> rcacChipBuf;
        VerifyOrReturnError(rcacChipBuf.Alloc(Credentials::kMaxCHIPCertLength), CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(rcac_dn.AddAttribute_MatterRCACId(kIssuerId));

        X509CertRequestParams rcac_request = { 0, mNow, mNow + mValidity, rcac_dn, rcac_dn };

        MutableByteSpan rcacDerSpan{ rcacDerBuf.Get(), rcacDerBuf.AllocatedSize() };
        ReturnErrorOnFailure(Credentials::NewRootX509Cert(rcac_request, rootKeyPair, rcacDerSpan));

        MutableByteSpan rcacChipSpan{ rcacChipBuf.Get(), rcacChipBuf.AllocatedSize() };
        ReturnErrorOnFailure(Credentials::ConvertX509CertToChipCert(rcacDerSpan, rcacChipSpan));

        VerifyOrReturnError(mLastRcac.Alloc(rcacChipSpan.size()), CHIP_ERROR_NO_MEMORY);
        memcpy(mLastRcac.Get(), rcacChipSpan.data(), rcacChipSpan.size());

        return CHIP_NO_ERROR;
    }

    uint32_t mNow = 0;

    // By default, let's set validity to 10 years
    uint32_t mValidity = 365 * 24 * 60 * 60 * 10;

    CHIP_ERROR mCurrentStatus = CHIP_NO_ERROR;
    bool mIncludeIcac         = false;

    Platform::ScopedMemoryBufferWithSize<uint8_t> mLastNoc;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mLastIcac;
    Platform::ScopedMemoryBufferWithSize<uint8_t> mLastRcac;

    Platform::UniquePtr<Crypto::P256Keypair> mRootKeypair;
};

} // namespace Credentials
} // namespace chip
