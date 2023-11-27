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

#define FABRIC_INDEX_TLV_SIZE 2

namespace chip {
namespace app {
CHIP_ERROR DefaultICDClientStorage::UpdateFabricList(FabricIndex fabricIndex)
{
    for (auto & fabric : mFabricList)
    {
        if (fabric == fabricIndex)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    mFabricList.push_back(fabricIndex);

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t counter      = mFabricList.size() + 1;
    size_t total = FABRIC_INDEX_TLV_SIZE * counter;
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & fabric : mFabricList)
    {
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), fabric));
    }
    return writer.EndContainer(arrayType);

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName(),
                                                                     backingBuffer.Get(), static_cast<uint16_t>(len)));
    uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
    ReturnErrorOnFailure(mpClientInfoStore->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::ICDFabricListCounter().KeyName()));
    return mpClientInfoStore->SyncSetKeyValue(
        DefaultStorageKeyAllocator::ICDFabricListCounter().KeyName(), &counter, counterLen);
}

CHIP_ERROR DefaultICDClientStorage::LoadFabricList()
{
    size_t counter      = 0;
    uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
    ReturnErrorOnFailure(mpClientInfoStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::ICDFabricListCounter().KeyName(), &counter, counterLen));

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t len = FABRIC_INDEX_TLV_SIZE * counter;
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    uint16_t length = static_cast<uint16_t>(len);
    ReturnErrorOnFailure(mpClientInfoStore->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName(),
                                                                    backingBuffer.Get(), length));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), len);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next(TLV::kTLVType_UnsignedInteger, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        FabricIndex fabricIndex;
        ReturnErrorOnFailure(reader.Get(fabricIndex));
        mFabricList.push_back(fabricIndex);
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }
    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
}

DefaultICDClientStorage::ICDClientInfoIteratorImpl::ICDClientInfoIteratorImpl(DefaultICDClientStorage & manager) : mManager(manager)
{
    mFabricListIndex    = 0;
    mClientInfoIndex = 0;
    mClientInfoVector.clear();
}

size_t DefaultICDClientStorage::ICDClientInfoIteratorImpl::Count()
{
    size_t total = 0;
    for (auto & fabric : mManager.mFabricList)
    {
        size_t counter      = 0;
        uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
        if (mManager.mpClientInfoStore->SyncGetKeyValue(
                DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabric).KeyName(), &counter, counterLen) !=
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
            if (mManager.Load(mManager.mFabricList[mFabricListIndex], mClientInfoVector) != CHIP_NO_ERROR)
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

CHIP_ERROR DefaultICDClientStorage::Init(PersistentStorageDelegate * clientInfoStore, Crypto::SymmetricKeystore * keyStore, size_t clientInfoSize)
{
    VerifyOrReturnError(clientInfoStore != nullptr && keyStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpClientInfoStore == nullptr && mpKeyStore == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpClientInfoStore = clientInfoStore;
    mpKeyStore = keyStore;
    LoadFabricList();

    uint16_t sizeLen = static_cast<uint16_t>(sizeof(clientInfoSize));
    return mpClientInfoStore->SyncSetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoMaxSize().KeyName(), &clientInfoSize, sizeLen);
}

DefaultICDClientStorage::ICDClientInfoIterator * DefaultICDClientStorage::IterateICDClientInfo()
{
    return mICDClientInfoIterators.CreateObject(*this);
}

CHIP_ERROR DefaultICDClientStorage::MaxICDClientInfoSize(size_t & size)
{
    uint16_t sizeLen = static_cast<uint16_t>(sizeof(size));
    return mpClientInfoStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoMaxSize().KeyName(), &size, sizeLen);
}

CHIP_ERROR DefaultICDClientStorage::Load(FabricIndex fabricIndex, std::vector<ICDClientInfo> & clientInfoVector)
{
    size_t counter      = 0;
    uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
    ReturnErrorOnFailure(mpClientInfoStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName(), &counter, counterLen));

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t clientInfoSize = 0;
    ReturnErrorOnFailure(MaxICDClientInfoSize(clientInfoSize));
    size_t len = clientInfoSize * counter;
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
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kPeerNodeId)));
        ReturnErrorOnFailure(reader.Get(nodeId));

        // Fabric Index
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(Tag::kFabricIndex)));
        ReturnErrorOnFailure(reader.Get(fabric));
        clientInfo.mPeerNode = ScopedNodeId(nodeId, fabric);

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
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(clientInfo.mPeerNode.GetFabricIndex(), clientInfoVector));

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
    size_t clientInfoSize = 0;
    ReturnErrorOnFailure(MaxICDClientInfoSize(clientInfoSize));
    size_t total = clientInfoSize * clientInfoVector.size();
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(Save(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(clientInfo.mPeerNode.GetFabricIndex()).KeyName(),
                                                                     backingBuffer.Get(), static_cast<uint16_t>(len)));

    return UpdateCounter(clientInfo.mPeerNode.GetFabricIndex(), true /*increase*/);
}

CHIP_ERROR DefaultICDClientStorage::UpdateCounter(FabricIndex fabricIndex, bool increase)
{
    size_t counter      = 0;
    uint16_t counterLen = static_cast<uint16_t>(sizeof(counter));
    ReturnErrorOnFailure(mpClientInfoStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName(), &counter, counterLen));
    if (increase)
    {
        counter++;
    }
    else
    {
        counter--;
    }
    ReturnErrorOnFailure(mpClientInfoStore->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName()));
    return mpClientInfoStore->SyncSetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName(), &counter, counterLen);
}

CHIP_ERROR DefaultICDClientStorage::DeleteEntry(const ScopedNodeId & peerNode)
{
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(peerNode.GetFabricIndex(), clientInfoVector));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (peerNode.GetNodeId() == it->mPeerNode.GetNodeId())
        {
            mpKeyStore->DestroyKey(it->mSharedKey);
            it = clientInfoVector.erase(it);
            break;
        }
    }

    ReturnErrorOnFailure(mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(peerNode.GetFabricIndex()).KeyName()));
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t clientInfoSize = 0;
    ReturnErrorOnFailure(MaxICDClientInfoSize(clientInfoSize));
    size_t total = clientInfoSize * clientInfoVector.size();
    backingBuffer.Calloc(total);
    ReturnErrorCodeIf(backingBuffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(Save(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(peerNode.GetFabricIndex()).KeyName(),
                                                                     backingBuffer.Get(), static_cast<uint16_t>(len)));

    return UpdateCounter(peerNode.GetFabricIndex(), false /*increase*/);
}

CHIP_ERROR DefaultICDClientStorage::DeleteAllEntries(FabricIndex fabricIndex)
{
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(fabricIndex, clientInfoVector));

    for (auto & clientInfo : clientInfoVector)
    {
        mpKeyStore->DestroyKey(clientInfo.mSharedKey);
    }
    ReturnErrorOnFailure(mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricIndex).KeyName()));
    return mpClientInfoStore->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName());
}

bool DefaultICDClientStorage::ValidateCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo)
{
    // TODO: Need to implement default decription code using CheckinMessage::ParseCheckinMessagePayload
    return false;
}
} // namespace app
} // namespace chip
