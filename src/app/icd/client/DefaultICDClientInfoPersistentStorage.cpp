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
#include "DefaultICDStorageKey.h"
#include <iterator>
#include <lib/core/Global.h>
#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

Global<DefaultICDClientStorage> sDefaultICDClientStorage;

DefaultICDClientStorage * DefaultICDClientStorage::GetInstance()
{
    return &sDefaultICDClientStorage.get();
}

DefaultICDClientStorage::ICDClientInfoIteratorImpl::ICDClientInfoIteratorImpl(DefaultICDClientStorage & aManager) :
    mManager(aManager)
{
    mStorageIndex    = 0;
    mClientInfoIndex = 0;
}

size_t DefaultICDClientStorage::ICDClientInfoIteratorImpl::Count()
{
    size_t total = 0;
    for (auto & storageIterator : mManager.mStorages)
    {
        if (!storageIterator.IsValid())
        {
            continue;
        }
        for (size_t clientInfoIndex = 0; clientInfoIndex < mManager.mpICDStorageKeyDelegate->MaxKeyCounter(); clientInfoIndex++)
        {
            if (storageIterator.mpClientInfoStore->SyncDoesKeyExist(
                    mManager.mpICDStorageKeyDelegate->GetKey(clientInfoIndex).KeyName()))
            {
                total++;
            }
        }
    }
    return total;
}

bool DefaultICDClientStorage::ICDClientInfoIteratorImpl::Next(ICDClientInfo & aOutput)
{
    for (; mStorageIndex < mManager.Size(); mStorageIndex++)
    {
        ICDStorage & storage = mManager.mStorages[mStorageIndex];
        if (!storage.IsValid())
        {
            continue;
        }
        for (; mClientInfoIndex < mManager.mpICDStorageKeyDelegate->MaxKeyCounter(); mClientInfoIndex++)
        {
            CHIP_ERROR err = mManager.Load(storage, mClientInfoIndex, aOutput);
            if (err == CHIP_NO_ERROR)
            {
                mClientInfoIndex++;
                return true;
            }

            if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                ChipLogError(DataManagement, "Failed to load ICDClient Info at index %u fabric %u error %" CHIP_ERROR_FORMAT,
                             static_cast<unsigned>(mClientInfoIndex), static_cast<unsigned>(storage.mFabricIndex), err.Format());
                break;
            }
        }
        mClientInfoIndex = 0;
    }
    return false;
}

void DefaultICDClientStorage::ICDClientInfoIteratorImpl::Release()
{
    mManager.mICDClientInfoIterators.ReleaseObject(this);
}

CHIP_ERROR DefaultICDClientStorage::Init(ICDStorageKeyDelegate * apICDStorageKeyDelegate)
{
    VerifyOrReturnError(apICDStorageKeyDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpICDStorageKeyDelegate == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpICDStorageKeyDelegate = apICDStorageKeyDelegate;
    return CHIP_NO_ERROR;
}

ICDStorage * DefaultICDClientStorage::FindStorage(FabricIndex aFabricIndex)
{
    for (auto & storage : mStorages)
    {
        if (!storage.IsValid())
        {
            continue;
        }

        if (storage.mFabricIndex == aFabricIndex)
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

CHIP_ERROR DefaultICDClientStorage::Load(ICDStorage & aStorage, size_t aIndex, ICDClientInfo & aICDClientInfo)
{
    VerifyOrReturnError(aStorage.IsValid(), CHIP_ERROR_INVALID_ARGUMENT);
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(MaxICDClientInfoSize()), CHIP_ERROR_NO_MEMORY);
    size_t len = MaxICDClientInfoSize();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    uint16_t length = static_cast<uint16_t>(len);
    ReturnErrorOnFailure(aStorage.mpClientInfoStore->SyncGetKeyValue(mpICDStorageKeyDelegate->GetKey(aIndex).KeyName(),
                                                                     backingBuffer.Get(), length));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), MaxICDClientInfoSize());

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

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
    aICDClientInfo.mPeerNode = ScopedNodeId(nodeId, fabricIndex);
    // Start ICD Counter
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kStartICDCounter)));
    ReturnErrorOnFailure(reader.Get(aICDClientInfo.mStartICDCounter));

    // Offset
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kOffset)));
    ReturnErrorOnFailure(reader.Get(aICDClientInfo.mOffset));

    // MonitoredSubject
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kMonitoredSubject)));
    ReturnErrorOnFailure(reader.Get(aICDClientInfo.mMonitoredSubject));

    // Shared key
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kSharedKey)));
    ByteSpan buf(aICDClientInfo.mSharedKey.AsMutable<Crypto::Aes128KeyByteArray>());
    ReturnErrorOnFailure(reader.Get(buf));
    memcpy(aICDClientInfo.mSharedKey.AsMutable<Crypto::Aes128KeyByteArray>(), buf.data(), sizeof(Crypto::Aes128KeyByteArray));
    return reader.ExitContainer(ICDClientInfoType);
}

CHIP_ERROR DefaultICDClientStorage::SetKey(ICDClientInfo & aClientInfo, const ByteSpan aKeyData)
{
    ICDStorage * storage = FindStorage(aClientInfo.mPeerNode.GetFabricIndex());
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);

    VerifyOrReturnError(aKeyData.size() == sizeof(Crypto::Aes128KeyByteArray), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(storage->mpKeyStore != nullptr, CHIP_ERROR_INTERNAL);

    Crypto::Aes128KeyByteArray keyMaterial;
    memcpy(keyMaterial, aKeyData.data(), sizeof(Crypto::Aes128KeyByteArray));

    return storage->mpKeyStore->CreateKey(keyMaterial, aClientInfo.mSharedKey);
}

CHIP_ERROR DefaultICDClientStorage::Save(TLV::TLVWriter & aWriter, const ICDClientInfo & aICDClientInfo)
{
    TLV::TLVType ICDClientInfoContainerType;
    ReturnErrorOnFailure(aWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ICDClientInfoContainerType));
    ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(Tag::kPeerNodeId), aICDClientInfo.mPeerNode.GetNodeId()));
    ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(Tag::kFabricIndex), aICDClientInfo.mPeerNode.GetFabricIndex()));
    ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(Tag::kStartICDCounter), aICDClientInfo.mStartICDCounter));
    ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(Tag::kOffset), aICDClientInfo.mOffset));
    ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(Tag::kMonitoredSubject), aICDClientInfo.mMonitoredSubject));
    ByteSpan buf(aICDClientInfo.mSharedKey.As<Crypto::Aes128KeyByteArray>());
    ReturnErrorOnFailure(aWriter.Put(TLV::ContextTag(Tag::kSharedKey), buf));
    ReturnErrorOnFailure(aWriter.EndContainer(ICDClientInfoContainerType));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::StoreEntry(ICDClientInfo & aICDClientInfo)
{
    ICDStorage * storage = FindStorage(aICDClientInfo.mPeerNode.GetFabricIndex());
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);

    // Find empty index or duplicate if exists
    size_t maxCount        = mpICDStorageKeyDelegate->MaxKeyCounter();
    size_t firstEmptyIndex = maxCount;
    for (size_t index = 0; index < maxCount; index++)
    {
        ICDClientInfo currentICDClientInfo;
        CHIP_ERROR err = Load(*storage, index, currentICDClientInfo);

        // if empty and firstEmptyIndex isn't set yet, then mark empty spot
        if ((firstEmptyIndex == maxCount) && (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
        {
            firstEmptyIndex = index;
        }

        // delete duplicate
        if (err == CHIP_NO_ERROR)
        {
            if (aICDClientInfo.mPeerNode.GetNodeId() == currentICDClientInfo.mPeerNode.GetNodeId())
            {
                Delete(*storage, index, aICDClientInfo.mSharedKey);
                // if duplicate is the first empty spot, then also set it
                if (firstEmptyIndex == maxCount)
                {
                    firstEmptyIndex = index;
                }
            }
        }
    }

    // Fail if no empty space
    if (firstEmptyIndex == maxCount)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Now construct ICD ClientInfo and save
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    backingBuffer.Calloc(MaxICDClientInfoSize());
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);

    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), MaxICDClientInfoSize());

    ReturnErrorOnFailure(Save(writer, aICDClientInfo));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(storage->mpClientInfoStore->SyncSetKeyValue(mpICDStorageKeyDelegate->GetKey(firstEmptyIndex).KeyName(),
                                                                     backingBuffer.Get(), static_cast<uint16_t>(len)));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::Delete(ICDStorage & aStorage, size_t aIndex, Crypto::Aes128KeyHandle & aSharedKey)
{
    VerifyOrReturnError(aStorage.mpClientInfoStore != nullptr, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(aStorage.mpClientInfoStore->SyncDeleteKeyValue(mpICDStorageKeyDelegate->GetKey(aIndex).KeyName()));
    return DeleteKey(aStorage.mpKeyStore, aSharedKey);
}

CHIP_ERROR DefaultICDClientStorage::DeleteKey(Crypto::SymmetricKeystore * apKeyStore, Crypto::Aes128KeyHandle & aSharedKey)
{
    VerifyOrReturnError(apKeyStore != nullptr, CHIP_ERROR_INTERNAL);
    apKeyStore->DestroyKey(aSharedKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::DeleteEntry(ScopedNodeId aPeerNode)
{
    ICDStorage * storage = FindStorage(aPeerNode.GetFabricIndex());
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);
    size_t maxCount = mpICDStorageKeyDelegate->MaxKeyCounter();
    for (size_t index = 0; index < maxCount; index++)
    {
        ICDClientInfo currentICDClientInfo;
        CHIP_ERROR err = Load(*storage, index, currentICDClientInfo);
        if (err == CHIP_NO_ERROR)
        {
            if (aPeerNode.GetNodeId() == currentICDClientInfo.mPeerNode.GetNodeId())
            {
                Delete(*storage, index, currentICDClientInfo.mSharedKey);
                break;
            }
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::DeleteAllEntries(FabricIndex aFabricIndex)
{
    ICDStorage * storage = FindStorage(aFabricIndex);
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_NOT_FOUND);
    size_t maxCount = mpICDStorageKeyDelegate->MaxKeyCounter();
    for (size_t index = 0; index < maxCount; index++)
    {
        ICDClientInfo currentICDClientInfo;
        CHIP_ERROR err = Load(*storage, index, currentICDClientInfo);
        if (err == CHIP_NO_ERROR)
        {
            Delete(*storage, index, currentICDClientInfo.mSharedKey);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::AddStorage(ICDStorage && aStorage)
{
    VerifyOrReturnError(aStorage.IsValid(), CHIP_ERROR_INVALID_ARGUMENT);
    mStorages.push_back(std::move(aStorage));
    return CHIP_NO_ERROR;
}

void DefaultICDClientStorage::RemoveStorage(FabricIndex aFabricIndex)
{
    DeleteAllEntries(aFabricIndex);
    for (auto storageIterator = mStorages.begin(); storageIterator != mStorages.end(); storageIterator++)
    {
        if (storageIterator->mFabricIndex == aFabricIndex)
        {
            mStorages.erase(storageIterator);
            break;
        }
    }
}

bool DefaultICDClientStorage::ValidateCheckInPayload(const ByteSpan & aPayload, ICDClientInfo & aClientInfo)
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
