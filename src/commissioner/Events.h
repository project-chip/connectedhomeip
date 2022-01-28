/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <controller/CommissioneeDeviceProxy.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <protocols/secure_channel/CASESession.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/QRCodeSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

namespace chip {
namespace Commissioner {
namespace Events {

constexpr size_t kMaxAttestationElementsLength = 900; // RESP_MAX
constexpr size_t kMaxNocsrElementsLength       = 900; // RESP_MAX
constexpr size_t kMaxNetworkIdLength           = 32;

struct SharedBuffer
{
    const ByteSpan Get() const { return ByteSpan(mData.get(), mLen); }
    MutableByteSpan GetMutable() { return MutableByteSpan(mData.get(), mCapacity); };
    CHIP_ERROR Set(const ByteSpan buffer)
    {
        if (mCapacity < buffer.size())
        {
            ReturnErrorOnFailure(Allocate(buffer.size()));
        }
        memmove(mData.get(), buffer.data(), buffer.size());
        mLen = buffer.size();
        return CHIP_NO_ERROR;
    }
    void Clear()
    {
        mData     = Platform::SharedPtr<uint8_t>(nullptr);
        mCapacity = 0;
        mLen      = 0;
    }
    CHIP_ERROR Allocate(size_t size)
    {
        mData = Platform::SharedPtr<uint8_t>(static_cast<uint8_t *>(chip::Platform::MemoryAlloc(size)), chip::Platform::MemoryFree);
        if (mData.get() == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mLen      = 0;
        mCapacity = size;
        return CHIP_NO_ERROR;
    }

private:
    Platform::SharedPtr<uint8_t> mData;
    size_t mCapacity = 0;
    size_t mLen      = 0;
};

template <size_t MaxSize>
struct CapacityBoundSharedBuffer : public SharedBuffer
{
    CHIP_ERROR Allocate(size_t size)
    {
        if (size > MaxSize)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        return SharedBuffer::Allocate(size);
    }
    CHIP_ERROR Set(const ByteSpan buffer)
    {
        if (buffer.size() > MaxSize)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        return SharedBuffer::Set(buffer);
    }
};

template <size_t MaxSize>
struct CapacityBoundStaticBuffer
{
    const ByteSpan Get() const { return ByteSpan(mBuffer, mBuffer.Length()); }
    void Clear() { mBuffer.SetLength(0); };
    CHIP_ERROR Set(ByteSpan buffer)
    {
        ReturnErrorOnFailure(mBuffer.SetLength(buffer.size()));
        memmove(mBuffer, buffer.data(), buffer.size());
        return CHIP_NO_ERROR;
    }

private:
    Crypto::CapacityBoundBuffer<MaxSize> mBuffer;
};

struct AesCcm128KeyBuffer : private AesCcm128Key
{
    AesCcm128KeyBuffer(AesCcm128KeySpan keySpan) : AesCcm128Key(keySpan) {}
    AesCcm128KeyBuffer() {}
    const AesCcm128KeySpan Get() const { return this->Span(); };
    void Set(AesCcm128KeySpan keySpan) { *this = AesCcm128KeyBuffer(keySpan); }
    void Clear() { *this = AesCcm128KeyBuffer(); }
};

using OnboardingPayload        = chip::Platform::SharedPtr<chip::SetupPayload>;
using ArmFailSafe              = chip::app::Clusters::GeneralCommissioning::Commands::ArmFailSafe::Type;
using NetworkFeatureMap        = app::Clusters::NetworkCommissioning::Attributes::FeatureMap::TypeInfo::DecodableType;
using AddOrUpdateWiFiNetwork   = app::Clusters::NetworkCommissioning::Commands::AddOrUpdateWiFiNetwork::Type;
using AddOrUpdateThreadNetwork = app::Clusters::NetworkCommissioning::Commands::AddOrUpdateThreadNetwork::Type;
using OperationalRecord        = chip::Platform::SharedPtr<Dnssd::ResolvedNodeData>;

struct Success
{
};

struct Failure
{
};

struct Await
{
};

struct RawOnboardingPayload
{
    const char * mPayload;
};

struct NetworkId : CapacityBoundStaticBuffer<kMaxNetworkIdLength>
{
    CHIP_ERROR FromThreadOperationalDataset(ByteSpan opaqueDataset)
    {
        Thread::OperationalDataset parsed;
        ReturnErrorOnFailure(parsed.Init(opaqueDataset));
        uint8_t extendedPanId[Thread::kSizeExtendedPanId];
        ReturnErrorOnFailure(parsed.GetExtendedPanId(extendedPanId));
        ReturnErrorOnFailure(Set(ByteSpan(extendedPanId)));
        return CHIP_NO_ERROR;
    }
};

class AttestationBase
{
public:
    AttestationBase(ByteSpan nonce) : mNonce(nonce) {}
    AttestationBase() = delete;

    class CapacityBoundNonce
    {
    public:
        CapacityBoundNonce(ByteSpan nonce) { Set(nonce); }
        CapacityBoundNonce() = delete;

        const ByteSpan Get() const { return ByteSpan(mNonce, mNonce.Length()); }
        void Clear() { mNonce.SetLength(0); };
        void Set(ByteSpan nonce)
        {
            mNonce.SetLength(std::min(nonce.size(), mNonce.Capacity()));
            memmove(mNonce, nonce.data(), mNonce.Length());
        }

    private:
        Crypto::CapacityBoundBuffer<kAttestationNonceLength> mNonce;
    };

    auto Nonce() { return &mNonce; }
    auto Challenge() { return &mChallenge; }
    auto Signature() { return &mSignature; }
    auto Pai() { return &mPai; }
    auto Dac() { return &mDac; }

private:
    CapacityBoundNonce mNonce;
    AesCcm128KeyBuffer mChallenge;
    CapacityBoundSharedBuffer<kMax_ECDSA_Signature_Length> mSignature;
    CapacityBoundSharedBuffer<Credentials::kMaxDERCertLength> mPai;
    CapacityBoundSharedBuffer<Credentials::kMaxDERCertLength> mDac;
};

struct AttestationInformation : public AttestationBase
{
    AttestationInformation(ByteSpan nonce) : AttestationBase(nonce) {}
    auto AttestationElements() { return &mAttestationElements; }

private:
    CapacityBoundSharedBuffer<kMaxAttestationElementsLength> mAttestationElements;
};

struct NocsrInformation : public AttestationBase
{
    NocsrInformation(AttestationInformation attestationInformation, ByteSpan nonce) : AttestationBase(attestationInformation)
    {
        Nonce()->Set(nonce);
        Signature()->Clear(); // Don't retain signature from AttestationBase.
    }

    auto NocsrElements() { return &mNocsrElements; }

private:
    CapacityBoundSharedBuffer<kMaxNocsrElementsLength> mNocsrElements;
};

struct OperationalCredentials
{
    auto Rcac() { return &mRcac; }
    auto Icac() { return &mIcac; }
    auto Noc() { return &mNoc; }
    auto Ipk() { return &mIpk; }
    chip::NodeId mAdminSubject = kUndefinedNodeId;
    uint16_t mAdminVendorId    = kUndefinedVendorId;

    CHIP_ERROR GetRootPublicKey(Crypto::P256PublicKey & rootPublicKey)
    {
        Credentials::P256PublicKeySpan rootPublicKeySpan;
        ReturnErrorOnFailure(Credentials::ExtractPublicKeyFromChipCert(this->Rcac()->Get(), rootPublicKeySpan));
        rootPublicKey = Crypto::P256PublicKey(rootPublicKeySpan);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetFabricAndNodeId(FabricId & fabricId, NodeId & nodeId)
    {
        ReturnErrorOnFailure(Credentials::ExtractNodeIdFabricIdFromOpCert(this->Noc()->Get(), &nodeId, &fabricId));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetCompressedFabricId(CompressedFabricId & compressedFabricId)
    {
        Crypto::P256PublicKey rootPublicKey;
        FabricId fabricId;
        NodeId unused;
        ReturnErrorOnFailure(GetRootPublicKey(rootPublicKey));
        ReturnErrorOnFailure(GetFabricAndNodeId(fabricId, unused));
        ReturnErrorOnFailure(GetCompressedFabricId(rootPublicKey, fabricId, compressedFabricId));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetOperationalId(PeerId & operationalId)
    {
        CompressedFabricId compressedFabricId;
        Crypto::P256PublicKey rootPublicKey;
        FabricId fabricId;
        NodeId nodeId;
        ReturnErrorOnFailure(GetRootPublicKey(rootPublicKey));
        ReturnErrorOnFailure(GetFabricAndNodeId(fabricId, nodeId));
        ReturnErrorOnFailure(GetCompressedFabricId(rootPublicKey, fabricId, compressedFabricId));
        operationalId.SetCompressedFabricId(compressedFabricId);
        operationalId.SetNodeId(nodeId);
        return CHIP_NO_ERROR;
    }

private:
    static CHIP_ERROR GetCompressedFabricId(Crypto::P256PublicKey & rootPublicKey, FabricId fabricId,
                                            CompressedFabricId & compressedFabricId)
    {
        uint8_t allocated[sizeof(uint64_t)];
        MutableByteSpan span(allocated);
        ReturnErrorOnFailure(GenerateCompressedFabricId(rootPublicKey, fabricId, span));
        // Decode compressed fabric ID accounting for endianness, as GenerateCompressedFabricId()
        // returns a binary buffer and is agnostic of usage of the output as an integer type.
        compressedFabricId = Encoding::BigEndian::Get64(allocated);
        return CHIP_NO_ERROR;
    }

    CapacityBoundSharedBuffer<Credentials::kMaxCHIPCertLength> mRcac;
    CapacityBoundSharedBuffer<Credentials::kMaxCHIPCertLength> mIcac;
    CapacityBoundSharedBuffer<Credentials::kMaxCHIPCertLength> mNoc;
    AesCcm128KeyBuffer mIpk;
};

} // namespace Events
} // namespace Commissioner
} // namespace chip
