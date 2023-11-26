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

namespace chip {
namespace app {

Global<DefaultICDClientStorage> sDefaultICDClientStorage;

DefaultICDClientStorage * DefaultICDClientStorage::GetInstance()
{
    return &sDefaultICDClientStorage.get();
}

DefaultICDClientStorage::ICDClientInfoIteratorImpl::ICDClientInfoIteratorImpl(DefaultICDClientStorage & manager) : mManager(manager)
{
    mStorageIndex    = 0;
    mClientInfoIndex = 0;
    mClientInfoVector.clear();
}

size_t DefaultICDClientStorage::ICDClientInfoIteratorImpl::Count()
{
    size_t total = 0;
    for (auto & storage : mManager.mStorages)
    {
        if (!storage.IsValid())
        {
            continue;
        }

        size_t counter      = 0;
        uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
        if (storage.mpCounterStore->SyncGetKeyValue(
                DefaultStorageKeyAllocator::FabricICDClientInfoCounter(storage.mFabricIndex).KeyName(), &counter, counterLen) !=
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
    for (; mStorageIndex < mManager.Size(); mStorageIndex++)
    {
        if (mClientInfoVector.size() == 0)
        {
            ICDStorage & storage = mManager.mStorages[mStorageIndex];
            if (!storage.IsValid())
            {
                continue;
            }
            if (mManager.Load(storage, mClientInfoVector) != CHIP_NO_ERROR)
            {
                continue;
            }
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

ICDStorage * DefaultICDClientStorage::FindStorage(FabricIndex fabricIndex)
{
    for (auto & storage : mStorages)
    {
        if (!storage.IsValid())
        {
            continue;
        }

        if (storage.mFabricIndex == fabricIndex)
        {
            return &storage;
        }
    }
    return nullptr;
}

DefaultICDClientStorage::ICDClientInfoIterator * DefaultICDClientStorage::IterateICDClientInfo()
{
    return mICDClientInfoIterators.CreateObject(*this);
}

CHIP_ERROR DefaultICDClientStorage::Load(ICDStorage & storage, std::vector<ICDClientInfo> & clientInfoVector)
{
    VerifyOrReturnError(storage.IsValid(), CHIP_ERROR_INVALID_ARGUMENT);
    size_t counter      = 0;
    uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
    ReturnErrorOnFailure(storage.mpCounterStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(storage.mFabricIndex).KeyName(), &counter, counterLen));

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t len = MaxICDClientInfoSize() * counter;
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    uint16_t length = static_cast<uint16_t>(len);
    ReturnErrorOnFailure(storage.mpClientInfoStore->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey().KeyName(),
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
        FabricIndex fabricIndex;
        ReturnErrorOnFailure(reader.EnterContainer(ICDClientInfoType));
        // Peer Node ID
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kPeerNodeId)));
        ReturnErrorOnFailure(reader.Get(nodeId));

        // Fabric Index
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kFabricIndex)));
        ReturnErrorOnFailure(reader.Get(fabricIndex));
        clientInfo.mPeerNode = ScopedNodeId(nodeId, fabricIndex);

        // Start ICD Counter
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kStartICDCounter)));
        ReturnErrorOnFailure(reader.Get(clientInfo.mStartICDCounter));

        // Offset
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kOffset)));
        ReturnErrorOnFailure(reader.Get(clientInfo.mOffset));

        // MonitoredSubject
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kMonitoredSubject)));
        ReturnErrorOnFailure(reader.Get(clientInfo.mMonitoredSubject));

        // Shared key
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kSharedKey)));
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
    ICDStorage * storage = FindStorage(clientInfo.mPeerNode.GetFabricIndex());
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);

    VerifyOrReturnError(keyData.size() == sizeof(Crypto::Aes128KeyByteArray), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(storage->mpKeyStore != nullptr, CHIP_ERROR_INTERNAL);

    Crypto::Aes128KeyByteArray keyMaterial;
    memcpy(keyMaterial, keyData.data(), sizeof(Crypto::Aes128KeyByteArray));

    return storage->mpKeyStore->CreateKey(keyMaterial, clientInfo.mSharedKey);
}

CHIP_ERROR DefaultICDClientStorage::Save(TLV::TLVWriter & writer, const std::vector<ICDClientInfo> & clientInfoVector)
{
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & clientInfo : clientInfoVector)
    {
        TLV::TLVType ICDClientInfoContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ICDClientInfoContainerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Tag::kPeerNodeId), clientInfo.mPeerNode.GetNodeId()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Tag::kFabricIndex), clientInfo.mPeerNode.GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Tag::kStartICDCounter), clientInfo.mStartICDCounter));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Tag::kOffset), clientInfo.mOffset));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Tag::kMonitoredSubject), clientInfo.mMonitoredSubject));
        ByteSpan buf(clientInfo.mSharedKey.As<Crypto::Aes128KeyByteArray>());
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Tag::kSharedKey), buf));
        ReturnErrorOnFailure(writer.EndContainer(ICDClientInfoContainerType));
    }
    return writer.EndContainer(arrayType);
}

CHIP_ERROR DefaultICDClientStorage::StoreEntry(ICDClientInfo & clientInfo)
{
    ICDStorage * storage = FindStorage(clientInfo.mPeerNode.GetFabricIndex());
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(*storage, clientInfoVector));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (clientInfo.mPeerNode.GetNodeId() == it->mPeerNode.GetNodeId())
        {
            clientInfoVector.erase(it);
            break;
        }
    }

    clientInfoVector.push_back(clientInfo);

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t total = MaxICDClientInfoSize() * clientInfoVector.size();
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(Save(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(storage->mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey().KeyName(),
                                                                     backingBuffer.Get(), static_cast<uint16_t>(len)));

    return UpdateCounter(*storage, true /*increase*/);
}

CHIP_ERROR DefaultICDClientStorage::UpdateCounter(ICDStorage & storage, bool increase)
{
    VerifyOrReturnError(storage.mpCounterStore != nullptr, CHIP_ERROR_INTERNAL);
    size_t counter      = 0;
    uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
    ReturnErrorOnFailure(storage.mpCounterStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(storage.mFabricIndex).KeyName(), &counter, counterLen));
    if (increase)
    {
        counter++;
    }
    else
    {
        counter--;
    }
    ReturnErrorOnFailure(storage.mpClientInfoStore->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(storage.mFabricIndex).KeyName()));
    return storage.mpCounterStore->SyncSetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(storage.mFabricIndex).KeyName(), &counter, counterLen);
}

CHIP_ERROR DefaultICDClientStorage::DeleteCounter(ICDStorage & storage)
{
    VerifyOrReturnError(storage.mpCounterStore != nullptr, CHIP_ERROR_INTERNAL);
    return storage.mpCounterStore->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(storage.mFabricIndex).KeyName());
}

CHIP_ERROR DefaultICDClientStorage::DeleteClientInfo(ICDStorage & storage)
{
    VerifyOrReturnError(storage.mpClientInfoStore != nullptr, CHIP_ERROR_INTERNAL);
    return storage.mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey().KeyName());
}

CHIP_ERROR DefaultICDClientStorage::DeleteKey(ICDStorage & storage, Crypto::Aes128KeyHandle & sharedKey)
{
    VerifyOrReturnError(storage.mpKeyStore != nullptr, CHIP_ERROR_INTERNAL);
    storage.mpKeyStore->DestroyKey(sharedKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::DeleteEntry(const ScopedNodeId & peerNode)
{
    ICDStorage * storage = FindStorage(peerNode.GetFabricIndex());
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(*storage, clientInfoVector));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (peerNode.GetNodeId() == it->mPeerNode.GetNodeId())
        {
            DeleteKey(*storage, it->mSharedKey);
            it = clientInfoVector.erase(it);
            break;
        }
    }

    ReturnErrorOnFailure(DeleteClientInfo(*storage));

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t total = MaxICDClientInfoSize() * clientInfoVector.size();
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(Save(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(storage->mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey().KeyName(),
                                                                     backingBuffer.Get(), static_cast<uint16_t>(len)));

    return UpdateCounter(*storage, false /*increase*/);
}

CHIP_ERROR DefaultICDClientStorage::DeleteAllEntries(FabricIndex fabricIndex)
{
    ICDStorage * storage = FindStorage(fabricIndex);
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(*storage, clientInfoVector));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        DeleteKey(*storage, it->mSharedKey);
    }
    ReturnErrorOnFailure(DeleteClientInfo(*storage));
    return DeleteCounter(*storage);
}

CHIP_ERROR DefaultICDClientStorage::AddStorage(ICDStorage && storage)
{
    VerifyOrReturnError(storage.IsValid(), CHIP_ERROR_INVALID_ARGUMENT);
    mStorages.push_back(std::move(storage));
    return CHIP_NO_ERROR;
}

void DefaultICDClientStorage::RemoveStorage(FabricIndex fabricIndex)
{
    DeleteAllEntries(fabricIndex);
    for (auto storageIterator = mStorages.begin(); storageIterator != mStorages.end(); storageIterator++)
    {
        if (storageIterator->mFabricIndex == fabricIndex)
        {
            mStorages.erase(storageIterator);
            break;
        }
    }
}

bool DefaultICDClientStorage::ValidateCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo)
{
    // TODO: Need to implement default decription code using CheckinMessage::ParseCheckinMessagePayload
    return false;
}

size_t DefaultICDClientStorage::Size()
{
    return mStorages.size();
}
} // namespace app
} // namespace chip
