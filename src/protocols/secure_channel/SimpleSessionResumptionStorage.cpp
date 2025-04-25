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

#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>

#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>

namespace chip {

constexpr TLV::Tag SimpleSessionResumptionStorage::kFabricIndexTag;
constexpr TLV::Tag SimpleSessionResumptionStorage::kPeerNodeIdTag;
constexpr TLV::Tag SimpleSessionResumptionStorage::kResumptionIdTag;
constexpr TLV::Tag SimpleSessionResumptionStorage::kSharedSecretTag;
constexpr TLV::Tag SimpleSessionResumptionStorage::kCATTag;

StorageKeyName SimpleSessionResumptionStorage::GetStorageKey(const ScopedNodeId & node)
{
    return DefaultStorageKeyAllocator::FabricSession(node.GetFabricIndex(), node.GetNodeId());
}

StorageKeyName SimpleSessionResumptionStorage::GetStorageKey(ConstResumptionIdView resumptionId)
{
    char resumptionIdBase64[BASE64_ENCODED_LEN(resumptionId.size()) + 1];
    auto len                = Base64Encode(resumptionId.data(), resumptionId.size(), resumptionIdBase64);
    resumptionIdBase64[len] = '\0';
    return DefaultStorageKeyAllocator::SessionResumption(resumptionIdBase64);
}

CHIP_ERROR SimpleSessionResumptionStorage::SaveIndex(const SessionIndex & index)
{
    std::array<uint8_t, MaxIndexSize()> buf;
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (size_t i = 0; i < index.mSize; ++i)
    {
        TLV::TLVType innerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, innerType));
        ReturnErrorOnFailure(writer.Put(kFabricIndexTag, index.mNodes[i].GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, index.mNodes[i].GetNodeId()));
        ReturnErrorOnFailure(writer.EndContainer(innerType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::SessionResumptionIndex().KeyName(), buf.data(),
                                                   static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSessionResumptionStorage::LoadIndex(SessionIndex & index)
{
    std::array<uint8_t, MaxIndexSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    if (mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::SessionResumptionIndex().KeyName(), buf.data(), len) != CHIP_NO_ERROR)
    {
        index.mSize = 0;
        return CHIP_NO_ERROR;
    }

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.data(), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    size_t count = 0;
    CHIP_ERROR err;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        if (count >= MATTER_ARRAY_SIZE(index.mNodes))
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        TLV::TLVType containerType;
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        FabricIndex fabricIndex;
        ReturnErrorOnFailure(reader.Next(kFabricIndexTag));
        ReturnErrorOnFailure(reader.Get(fabricIndex));

        NodeId peerNodeId;
        ReturnErrorOnFailure(reader.Next(kPeerNodeIdTag));
        ReturnErrorOnFailure(reader.Get(peerNodeId));

        index.mNodes[count++] = ScopedNodeId(peerNodeId, fabricIndex);

        ReturnErrorOnFailure(reader.ExitContainer(containerType));
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    index.mSize = count;

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSessionResumptionStorage::SaveLink(ConstResumptionIdView resumptionId, const ScopedNodeId & node)
{
    // Save a link from resumptionId to node, in key: /g/s/<resumptionId>
    std::array<uint8_t, MaxScopedNodeIdSize()> buf;
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(kFabricIndexTag, node.GetFabricIndex()));
    ReturnErrorOnFailure(writer.Put(kPeerNodeIdTag, node.GetNodeId()));
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    const auto len = writer.GetLengthWritten();
    VerifyOrDie(CanCastTo<uint16_t>(len));

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(GetStorageKey(resumptionId).KeyName(), buf.data(), static_cast<uint16_t>(len)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSessionResumptionStorage::LoadLink(ConstResumptionIdView resumptionId, ScopedNodeId & node)
{
    std::array<uint8_t, MaxScopedNodeIdSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(GetStorageKey(resumptionId).KeyName(), buf.data(), len));

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.data(), len);

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

CHIP_ERROR SimpleSessionResumptionStorage::DeleteLink(ConstResumptionIdView resumptionId)
{
    ReturnErrorOnFailure(mStorage->SyncDeleteKeyValue(GetStorageKey(resumptionId).KeyName()));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSessionResumptionStorage::SaveState(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                                                     const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs)
{
    // Save session state into key: /f/<fabricIndex>/s/<nodeId>
    std::array<uint8_t, MaxStateSize()> buf;
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

    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(GetStorageKey(node).KeyName(), buf.data(), static_cast<uint16_t>(len)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimpleSessionResumptionStorage::LoadState(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                                                     Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs)
{
    std::array<uint8_t, MaxStateSize()> buf;
    uint16_t len = static_cast<uint16_t>(buf.size());

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(GetStorageKey(node).KeyName(), buf.data(), len));

    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf.data(), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    ByteSpan resumptionIdSpan;
    ReturnErrorOnFailure(reader.Next(kResumptionIdTag));
    ReturnErrorOnFailure(reader.Get(resumptionIdSpan));
    VerifyOrReturnError(resumptionIdSpan.size() == resumptionId.size(), CHIP_ERROR_KEY_NOT_FOUND);
    std::copy(resumptionIdSpan.begin(), resumptionIdSpan.end(), resumptionId.begin());

    ByteSpan sharedSecretSpan;
    ReturnErrorOnFailure(reader.Next(kSharedSecretTag));
    ReturnErrorOnFailure(reader.Get(sharedSecretSpan));
    VerifyOrReturnError(sharedSecretSpan.size() <= sharedSecret.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);
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

CHIP_ERROR SimpleSessionResumptionStorage::DeleteState(const ScopedNodeId & node)
{
    ReturnErrorOnFailure(mStorage->SyncDeleteKeyValue(GetStorageKey(node).KeyName()));
    return CHIP_NO_ERROR;
}

} // namespace chip
