/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "DefaultICDClientStorage.h"
#include <iterator>
#include <lib/core/Global.h>
#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {
// FabricIndex is uint8_t, the tlv size with anonumous tag is 1(control bytes) + 1(value) = 2
constexpr size_t kFabricIndexTlvSize = 2;

// The array itself has a control byte and an end-of-array marker.
constexpr size_t kArrayOverHead  = 2;
constexpr size_t kFabricIndexMax = 255;
} // namespace

namespace chip {
namespace app {
CHIP_ERROR DefaultICDClientStorage::UpdateFabricList(FabricIndex fabricIndex)
{
    for (auto & fabric_idx : mFabricList)
    {
        if (fabric_idx == fabricIndex)
        {
            return CHIP_NO_ERROR;
        }
    }

    mFabricList.push_back(fabricIndex);

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t counter = mFabricList.size();
    size_t total   = kFabricIndexTlvSize * counter + kArrayOverHead;
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & fabric_idx : mFabricList)
    {
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), fabric_idx));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    return mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName(), backingBuffer.Get(),
                                              static_cast<uint16_t>(len));
}

CHIP_ERROR DefaultICDClientStorage::LoadFabricList()
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t len = kFabricIndexTlvSize * kFabricIndexMax + kArrayOverHead;
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    uint16_t length = static_cast<uint16_t>(len);
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName(), backingBuffer.Get(), length));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    while ((reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        FabricIndex fabricIndex;
        ReturnErrorOnFailure(reader.Get(fabricIndex));
        mFabricList.push_back(fabricIndex);
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
}

DefaultICDClientStorage::ICDClientInfoIteratorImpl::ICDClientInfoIteratorImpl(DefaultICDClientStorage & manager) : mManager(manager)
{
    mFabricListIndex = 0;
    mClientInfoIndex = 0;
    mClientInfoVector.clear();
}

size_t DefaultICDClientStorage::ICDClientInfoIteratorImpl::Count()
{
    size_t total = 0;
    for (auto & fabric_idx : mManager.mFabricList)
    {
        size_t counter      = 0;
        uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
        if (mManager.mpClientInfoStore->SyncGetKeyValue(
                DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabric_idx).KeyName(), &counter, counterLen) !=
            CHIP_NO_ERROR)
        {
            return 0;
        }
        total += counter;
    }
    return total;
}

bool DefaultICDClientStorage::ICDClientInfoIteratorImpl::Next(ICDClientInfo & item)
{
    for (; mFabricListIndex < mManager.mFabricList.size(); mFabricListIndex++)
    {
        if (mClientInfoVector.size() == 0)
        {
            size_t clientInfoSize = 0;
            if (mManager.Load(mManager.mFabricList[mFabricListIndex], mClientInfoVector, clientInfoSize) != CHIP_NO_ERROR)
            {
                continue;
            }
            IgnoreUnusedVariable(clientInfoSize);
        }
        if (mClientInfoIndex < mClientInfoVector.size())
        {
            item = mClientInfoVector[mClientInfoIndex];
            mClientInfoIndex++;
            return true;
        }
        mClientInfoIndex = 0;
        mClientInfoVector.clear();
    }

    return false;
}

void DefaultICDClientStorage::ICDClientInfoIteratorImpl::Release()
{
    mManager.mICDClientInfoIterators.ReleaseObject(this);
}

CHIP_ERROR DefaultICDClientStorage::Init(PersistentStorageDelegate * clientInfoStore, Crypto::SymmetricKeystore * keyStore)
{
    VerifyOrReturnError(clientInfoStore != nullptr && keyStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpClientInfoStore == nullptr && mpKeyStore == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpClientInfoStore = clientInfoStore;
    mpKeyStore        = keyStore;
    return LoadFabricList();
}

DefaultICDClientStorage::ICDClientInfoIterator * DefaultICDClientStorage::IterateICDClientInfo()
{
    return mICDClientInfoIterators.CreateObject(*this);
}

CHIP_ERROR DefaultICDClientStorage::LoadCounter(FabricIndex fabricIndex, size_t & count, size_t & clientInfoSize)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t len = MaxICDCounterSize();
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    uint16_t length = static_cast<uint16_t>(len);
    ReturnErrorOnFailure(mpClientInfoStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName(), backingBuffer.Get(), length));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType structType;
    ReturnErrorOnFailure(reader.EnterContainer(structType));

    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(CounterTag::kCount)));
    ReturnErrorOnFailure(reader.Get(count));

    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(CounterTag::kSize)));
    ReturnErrorOnFailure(reader.Get(clientInfoSize));

    ReturnErrorOnFailure(reader.ExitContainer(structType));
    return reader.VerifyEndOfContainer();
}

CHIP_ERROR DefaultICDClientStorage::Load(FabricIndex fabricIndex, std::vector<ICDClientInfo> & clientInfoVector,
                                         size_t & clientInfoSize)
{
    size_t count = 0;
    ReturnErrorOnFailure(LoadCounter(fabricIndex, count, clientInfoSize));
    size_t len = clientInfoSize * count + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    uint16_t length = static_cast<uint16_t>(len);
    ReturnErrorOnFailure(mpClientInfoStore->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricIndex).KeyName(),
                                                            backingBuffer.Get(), length));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        ICDClientInfo clientInfo;
        TLV::TLVType ICDClientInfoType;
        NodeId nodeId;
        FabricIndex fabric;
        ReturnErrorOnFailure(reader.EnterContainer(ICDClientInfoType));
        // Peer Node ID
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kPeerNodeId)));
        ReturnErrorOnFailure(reader.Get(nodeId));

        // Fabric Index
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kFabricIndex)));
        ReturnErrorOnFailure(reader.Get(fabric));
        clientInfo.peer_node = ScopedNodeId(nodeId, fabric);

        // Start ICD Counter
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kStartICDCounter)));
        ReturnErrorOnFailure(reader.Get(clientInfo.start_icd_counter));

        // Offset
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kOffset)));
        ReturnErrorOnFailure(reader.Get(clientInfo.offset));

        // MonitoredSubject
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kMonitoredSubject)));
        ReturnErrorOnFailure(reader.Get(clientInfo.monitored_subject));

        // Shared key
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kSharedKey)));
        ByteSpan buf(clientInfo.mSharedKey.AsMutable<Crypto::Aes128KeyByteArray>());
        ReturnErrorOnFailure(reader.Get(buf));
        memcpy(clientInfo.mSharedKey.AsMutable<Crypto::Aes128KeyByteArray>(), buf.data(), sizeof(Crypto::Aes128KeyByteArray));
        ReturnErrorOnFailure(reader.ExitContainer(ICDClientInfoType));
        clientInfoVector.push_back(clientInfo);
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }
    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
}

CHIP_ERROR DefaultICDClientStorage::SetKey(ICDClientInfo & clientInfo, const ByteSpan keyData)
{
    VerifyOrReturnError(keyData.size() == sizeof(Crypto::Aes128KeyByteArray), CHIP_ERROR_INVALID_ARGUMENT);

    Crypto::Aes128KeyByteArray keyMaterial;
    memcpy(keyMaterial, keyData.data(), sizeof(Crypto::Aes128KeyByteArray));

    return mpKeyStore->CreateKey(keyMaterial, clientInfo.mSharedKey);
}

CHIP_ERROR DefaultICDClientStorage::Save(TLV::TLVWriter & writer, const std::vector<ICDClientInfo> & clientInfoVector)
{
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & clientInfo : clientInfoVector)
    {
        TLV::TLVType ICDClientInfoContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ICDClientInfoContainerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kPeerNodeId), clientInfo.peer_node.GetNodeId()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kFabricIndex), clientInfo.peer_node.GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kStartICDCounter), clientInfo.start_icd_counter));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kOffset), clientInfo.offset));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kMonitoredSubject), clientInfo.monitored_subject));
        ByteSpan buf(clientInfo.mSharedKey.As<Crypto::Aes128KeyByteArray>());
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kSharedKey), buf));
        ReturnErrorOnFailure(writer.EndContainer(ICDClientInfoContainerType));
    }
    return writer.EndContainer(arrayType);
}

CHIP_ERROR DefaultICDClientStorage::StoreEntry(ICDClientInfo & clientInfo)
{
    std::vector<ICDClientInfo> clientInfoVector;
    size_t clientInfoSize = 0;
    ReturnErrorOnFailure(Load(clientInfo.peer_node.GetFabricIndex(), clientInfoVector, clientInfoSize));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (clientInfo.peer_node.GetNodeId() == it->peer_node.GetNodeId())
        {
            clientInfoVector.erase(it);
            break;
        }
    }

    clientInfoVector.push_back(clientInfo);

    size_t total = clientInfoSize * clientInfoVector.size() + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(Save(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(mpClientInfoStore->SyncSetKeyValue(
        DefaultStorageKeyAllocator::ICDClientInfoKey(clientInfo.peer_node.GetFabricIndex()).KeyName(), backingBuffer.Get(),
        static_cast<uint16_t>(len)));

    return UpdateCounter(clientInfo.peer_node.GetFabricIndex(), true /*increase*/);
}

CHIP_ERROR DefaultICDClientStorage::UpdateCounter(FabricIndex fabricIndex, bool increase)
{
    size_t count          = 0;
    size_t clientInfoSize = MaxICDClientInfoSize();
    CHIP_ERROR err        = LoadCounter(fabricIndex, count, clientInfoSize);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    if (increase)
    {
        count++;
    }
    else
    {
        count--;
    }

    size_t total = MaxICDCounterSize();
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    TLV::TLVType structType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(CounterTag::kCount), count));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(CounterTag::kSize), clientInfoSize));
    ReturnErrorOnFailure(writer.EndContainer(structType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    writer.Finalize(backingBuffer);

    return mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricIndex).KeyName(),
                                              backingBuffer.Get(), static_cast<uint16_t>(len));
}

CHIP_ERROR DefaultICDClientStorage::DeleteEntry(const ScopedNodeId & peerNode)
{
    size_t clientInfoSize = 0;
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(peerNode.GetFabricIndex(), clientInfoVector, clientInfoSize));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (peerNode.GetNodeId() == it->peer_node.GetNodeId())
        {
            mpKeyStore->DestroyKey(it->mSharedKey);
            it = clientInfoVector.erase(it);
            break;
        }
    }

    ReturnErrorOnFailure(
        mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(peerNode.GetFabricIndex()).KeyName()));

    size_t total = clientInfoSize * clientInfoVector.size();
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(Save(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(peerNode.GetFabricIndex()).KeyName(),
                                           backingBuffer.Get(), static_cast<uint16_t>(len)));

    return UpdateCounter(peerNode.GetFabricIndex(), false /*increase*/);
}

CHIP_ERROR DefaultICDClientStorage::DeleteAllEntries(FabricIndex fabricIndex)
{
    size_t clientInfoSize = 0;
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(fabricIndex, clientInfoVector, clientInfoSize));
    IgnoreUnusedVariable(clientInfoSize);
    for (auto & clientInfo : clientInfoVector)
    {
        mpKeyStore->DestroyKey(clientInfo.mSharedKey);
    }
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricIndex).KeyName()));
    return mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName());
}

bool DefaultICDClientStorage::ValidateCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo)
{
    // TODO: Need to implement default decription code using CheckinMessage::ParseCheckinMessagePayload
    return false;
}
} // namespace app
} // namespace chip
