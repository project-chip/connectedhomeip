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

/**
 *    @file
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#include <protocols/secure_channel/SessionResumptionStorage.h>

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>

namespace chip {

constexpr TLV::Tag SessionResumptionStorage::kFabricIndexTag;
constexpr TLV::Tag SessionResumptionStorage::kPeerNodeIdTag;
constexpr TLV::Tag SessionResumptionStorage::kResumptionIdTag;
constexpr TLV::Tag SessionResumptionStorage::kSharedSecretTag;
constexpr TLV::Tag SessionResumptionStorage::kCATTag;

const char * SessionResumptionStorage::StorageKey(DefaultStorageKeyAllocator & keyAlloc, ScopedNodeId node)
{
    return keyAlloc.FabricSession(node.GetFabricIndex(), node.GetNodeId());
}

const char * SessionResumptionStorage::StorageKey(DefaultStorageKeyAllocator & keyAlloc, ConstResumptionIdView resumptionId)
{
    char resumptionIdBase64[BASE64_ENCODED_LEN(resumptionId.size()) + 1] ; 
    auto len = Base64Encode(resumptionId.data(), resumptionId.size(), resumptionIdBase64);
    resumptionIdBase64[len] = '\0';
    return keyAlloc.SessionResumption(resumptionIdBase64);
}

CHIP_ERROR SessionResumptionStorage::Save(ScopedNodeId node, ConstResumptionIdView resumptionId,
                                          const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs)
{
    DefaultStorageKeyAllocator keyAlloc;
    {
        // Save session state into key: /f/<fabricIndex>/s/<nodeId>
        uint8_t buf[MaxStateSize()];

        TLV::TLVWriter writer;
        writer.Init(buf);

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));

        ReturnErrorOnFailure(writer.Put(kResumptionIdTag, resumptionId));
        ReturnErrorOnFailure(writer.Put(kSharedSecretTag, ByteSpan(sharedSecret.ConstBytes(), sharedSecret.Length())));
        CATValues::Serialized cat;
        peerCATs.Serialize(cat);
        ReturnErrorOnFailure(writer.Put(kCATTag, ByteSpan(cat)));

        ReturnErrorOnFailure(writer.EndContainer(outerType));

        const auto len = writer.GetLengthWritten();
        VerifyOrDie(CanCastTo<uint16_t>(len));

        ReturnErrorOnFailure(mStorage->SyncSetKeyValue(StorageKey(keyAlloc, node), buf, static_cast<uint16_t>(len)));
    }

    {
        // Save a link from resumptionId to node, in key: /f/<fabricIndex>/r/<resumptionId>
        uint8_t buf[MaxLinkSize()];

        TLV::TLVWriter writer;
        writer.Init(buf);

        TLV::TLVType outerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
        ReturnErrorOnFailure(writer.Put(kFabricIndexTag, node.GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, node.GetNodeId()));
        ReturnErrorOnFailure(writer.EndContainer(outerType));

        const auto len = writer.GetLengthWritten();
        VerifyOrDie(CanCastTo<uint16_t>(len));

        ReturnErrorOnFailure(mStorage->SyncSetKeyValue(StorageKey(keyAlloc, resumptionId), buf, static_cast<uint16_t>(len)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::FindByScopedNodeId(ScopedNodeId node, ResumptionIdView resumptionId,
                                                        Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs)
{
    DefaultStorageKeyAllocator keyAlloc;
    uint8_t buf[MaxStateSize()];
    uint16_t len = static_cast<uint16_t>(MaxStateSize());

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(StorageKey(keyAlloc, node), buf, len));

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf, len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ByteSpan resumptionIdSpan;
    ReturnErrorOnFailure(reader.Next(kResumptionIdTag));
    ReturnErrorOnFailure(reader.Get(resumptionIdSpan));
    std::copy(resumptionIdSpan.begin(), resumptionIdSpan.end(), resumptionId.begin());

    ByteSpan sharedSecretSpan;
    ReturnErrorOnFailure(reader.Next(kSharedSecretTag));
    ReturnErrorOnFailure(reader.Get(sharedSecretSpan));
    VerifyOrReturnError(sharedSecret.Length() <= sharedSecretSpan.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    ::memcpy(sharedSecret.Bytes(), sharedSecretSpan.data(), sharedSecretSpan.size());
    sharedSecret.SetLength(sharedSecretSpan.size());

    ByteSpan catSpan;
    ReturnErrorOnFailure(reader.Next(kCATTag));
    ReturnErrorOnFailure(reader.Get(catSpan));
    CATValues::Serialized cat;
    VerifyOrReturnError(sizeof(cat) == catSpan.size(), CHIP_ERROR_INVALID_TLV_ELEMENT);
    ::memcpy(cat, catSpan.data(), catSpan.size());
    peerCATs.Deserialize(cat);

    ReturnErrorOnFailure(reader.ExitContainer(containerType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::FindNodeByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node)
{
    DefaultStorageKeyAllocator keyAlloc;
    uint8_t buf[MaxLinkSize()];
    uint16_t len = static_cast<uint16_t>(MaxStateSize());

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(StorageKey(keyAlloc, resumptionId), buf, len));

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf, len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    FabricIndex fabricIndex;
    ReturnErrorOnFailure(reader.Next(kFabricIndexTag));
    ReturnErrorOnFailure(reader.Get(fabricIndex));

    NodeId peerNodeId;
    ReturnErrorOnFailure(reader.Next(kPeerNodeIdTag));
    ReturnErrorOnFailure(reader.Get(peerNodeId));

    ReturnErrorOnFailure(reader.ExitContainer(containerType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    node = ScopedNodeId(peerNodeId, fabricIndex);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::FindByResumptionId(ConstResumptionIdView resumptionId, ScopedNodeId & node,
                                                        Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs)
{
    ReturnErrorOnFailure(FindNodeByResumptionId(resumptionId, node));
    ResumptionIdStorage tmpResumptionId;
    ReturnErrorOnFailure(FindByScopedNodeId(node, tmpResumptionId, sharedSecret, peerCATs));
    VerifyOrReturnError(std::equal(tmpResumptionId.begin(), tmpResumptionId.end(), resumptionId.begin(), resumptionId.end()),
                        CHIP_ERROR_KEY_NOT_FOUND);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionResumptionStorage::Delete(ScopedNodeId node)
{
    DefaultStorageKeyAllocator keyAlloc;
    mStorage->SyncDeleteKeyValue(StorageKey(keyAlloc, node));
    return CHIP_NO_ERROR;
}

} // namespace chip
