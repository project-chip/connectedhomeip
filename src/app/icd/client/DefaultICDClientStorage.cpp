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

#include <app/icd/client/DefaultICDClientStorage.h>
#include <iterator>
#include <lib/core/Global.h>
#include <lib/support/Base64.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <limits>

namespace {
// FabricIndex is uint8_t, the tlv size with anonymous tag is 1(control bytes) + 1(value) = 2
constexpr size_t kFabricIndexTlvSize = 2;

// The array itself has a control byte and an end-of-array marker.
constexpr size_t kArrayOverHead  = 2;
constexpr size_t kFabricIndexMax = 255;

constexpr size_t kMaxFabricListTlvLength = kFabricIndexTlvSize * kFabricIndexMax + kArrayOverHead;
static_assert(kMaxFabricListTlvLength <= std::numeric_limits<uint16_t>::max(), "Expected size for fabric list TLV is too large!");
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

    return StoreFabricList();
}

CHIP_ERROR DefaultICDClientStorage::StoreFabricList()
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t counter = mFabricList.size();
    size_t total   = kFabricIndexTlvSize * counter + kArrayOverHead;
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
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

    ReturnErrorOnFailure(writer.Finalize(backingBuffer));
    return mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName(), backingBuffer.Get(),
                                              static_cast<uint16_t>(len));
}

CHIP_ERROR DefaultICDClientStorage::LoadFabricList()
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(kMaxFabricListTlvLength), CHIP_ERROR_NO_MEMORY);
    uint16_t length = kMaxFabricListTlvLength;
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName(), backingBuffer.Get(), length));

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), length);
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
        size_t count          = 0;
        size_t clientInfoSize = 0;
        if (mManager.LoadCounter(fabric_idx, count, clientInfoSize) != CHIP_NO_ERROR)
        {
            return 0;
        };
        IgnoreUnusedVariable(clientInfoSize);
        total += count;
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
    CHIP_ERROR err    = LoadFabricList();
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

DefaultICDClientStorage::ICDClientInfoIterator * DefaultICDClientStorage::IterateICDClientInfo()
{
    return mICDClientInfoIterators.CreateObject(*this);
}

CHIP_ERROR DefaultICDClientStorage::LoadCounter(FabricIndex fabricIndex, size_t & count, size_t & clientInfoSize)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t len = MaxICDCounterSize();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    uint16_t length = static_cast<uint16_t>(len);

    CHIP_ERROR err = mpClientInfoStore->SyncGetKeyValue(
        DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName(), backingBuffer.Get(), length);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), length);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType structType;
    ReturnErrorOnFailure(reader.EnterContainer(structType));
    uint32_t tempCount = 0;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(CounterTag::kCount)));
    ReturnErrorOnFailure(reader.Get(tempCount));
    count = static_cast<size_t>(tempCount);

    uint32_t tempClientInfoSize = 0;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(CounterTag::kSize)));
    ReturnErrorOnFailure(reader.Get(tempClientInfoSize));
    clientInfoSize = static_cast<size_t>(tempClientInfoSize);

    ReturnErrorOnFailure(reader.ExitContainer(structType));
    return reader.VerifyEndOfContainer();
}

CHIP_ERROR DefaultICDClientStorage::Load(FabricIndex fabricIndex, std::vector<ICDClientInfo> & clientInfoVector,
                                         size_t & clientInfoSize)
{
    size_t count = 0;
    ReturnErrorOnFailure(LoadCounter(fabricIndex, count, clientInfoSize));
    VerifyOrReturnError(count > 0, CHIP_NO_ERROR);
    size_t len = clientInfoSize * count + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    uint16_t length = static_cast<uint16_t>(len);
    CHIP_ERROR err  = mpClientInfoStore->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricIndex).KeyName(),
                                                         backingBuffer.Get(), length);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), length);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        ICDClientInfo clientInfo;
        TLV::TLVType ICDClientInfoType;
        NodeId nodeId;
        NodeId checkInNodeId;
        FabricIndex fabric;
        ReturnErrorOnFailure(reader.EnterContainer(ICDClientInfoType));
        // Peer Node ID
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kPeerNodeId)));
        ReturnErrorOnFailure(reader.Get(nodeId));

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kCheckInNodeId)));
        ReturnErrorOnFailure(reader.Get(checkInNodeId));

        // Fabric Index
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kFabricIndex)));
        ReturnErrorOnFailure(reader.Get(fabric));

        clientInfo.peer_node     = ScopedNodeId(nodeId, fabric);
        clientInfo.check_in_node = ScopedNodeId(checkInNodeId, fabric);

        // Start ICD Counter
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kStartICDCounter)));
        ReturnErrorOnFailure(reader.Get(clientInfo.start_icd_counter));

        // Offset
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kOffset)));
        ReturnErrorOnFailure(reader.Get(clientInfo.offset));

        // MonitoredSubject
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kMonitoredSubject)));
        ReturnErrorOnFailure(reader.Get(clientInfo.monitored_subject));

        // Aes key handle
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kAesKeyHandle)));
        ByteSpan aesBuf;
        ReturnErrorOnFailure(reader.Get(aesBuf));
        VerifyOrReturnError(aesBuf.size() == sizeof(Crypto::Symmetric128BitsKeyByteArray), CHIP_ERROR_INTERNAL);
        memcpy(clientInfo.aes_key_handle.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(), aesBuf.data(),
               sizeof(Crypto::Symmetric128BitsKeyByteArray));

        // Hmac key handle
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kHmacKeyHandle)));
        ByteSpan hmacBuf;
        ReturnErrorOnFailure(reader.Get(hmacBuf));
        VerifyOrReturnError(hmacBuf.size() == sizeof(Crypto::Symmetric128BitsKeyByteArray), CHIP_ERROR_INTERNAL);
        memcpy(clientInfo.hmac_key_handle.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(), hmacBuf.data(),
               sizeof(Crypto::Symmetric128BitsKeyByteArray));

        // ClientType
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(ClientInfoTag::kClientType)));
        ReturnErrorOnFailure(reader.Get(clientInfo.client_type));

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
    VerifyOrReturnError(keyData.size() == sizeof(Crypto::Symmetric128BitsKeyByteArray), CHIP_ERROR_INVALID_ARGUMENT);

    Crypto::Symmetric128BitsKeyByteArray keyMaterial;
    memcpy(keyMaterial, keyData.data(), sizeof(Crypto::Symmetric128BitsKeyByteArray));

    // TODO : Update key lifetime once creaKey method supports it.
    ReturnErrorOnFailure(mpKeyStore->CreateKey(keyMaterial, clientInfo.aes_key_handle));
    CHIP_ERROR err = mpKeyStore->CreateKey(keyMaterial, clientInfo.hmac_key_handle);
    if (err != CHIP_NO_ERROR)
    {
        mpKeyStore->DestroyKey(clientInfo.aes_key_handle);
    }
    return err;
}

void DefaultICDClientStorage::RemoveKey(ICDClientInfo & clientInfo)
{
    mpKeyStore->DestroyKey(clientInfo.aes_key_handle);
    mpKeyStore->DestroyKey(clientInfo.hmac_key_handle);
}

CHIP_ERROR DefaultICDClientStorage::SerializeToTlv(TLV::TLVWriter & writer, const std::vector<ICDClientInfo> & clientInfoVector)
{
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & clientInfo : clientInfoVector)
    {
        TLV::TLVType ICDClientInfoContainerType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ICDClientInfoContainerType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kPeerNodeId), clientInfo.peer_node.GetNodeId()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kCheckInNodeId), clientInfo.check_in_node.GetNodeId()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kFabricIndex), clientInfo.peer_node.GetFabricIndex()));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kStartICDCounter), clientInfo.start_icd_counter));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kOffset), clientInfo.offset));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kMonitoredSubject), clientInfo.monitored_subject));
        ByteSpan aesBuf(clientInfo.aes_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>());
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kAesKeyHandle), aesBuf));
        ByteSpan hmacBuf(clientInfo.hmac_key_handle.As<Crypto::Symmetric128BitsKeyByteArray>());
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kHmacKeyHandle), hmacBuf));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(ClientInfoTag::kClientType), clientInfo.client_type));
        ReturnErrorOnFailure(writer.EndContainer(ICDClientInfoContainerType));
    }
    return writer.EndContainer(arrayType);
}

bool DefaultICDClientStorage::FabricExists(FabricIndex fabricIndex)
{
    for (auto & fabric_idx : mFabricList)
    {
        if (fabric_idx == fabricIndex)
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR DefaultICDClientStorage::StoreEntry(const ICDClientInfo & clientInfo)
{
    VerifyOrReturnError(FabricExists(clientInfo.peer_node.GetFabricIndex()), CHIP_ERROR_INVALID_FABRIC_INDEX);
    std::vector<ICDClientInfo> clientInfoVector;
    size_t clientInfoSize = MaxICDClientInfoSize();
    ReturnErrorOnFailure(Load(clientInfo.peer_node.GetFabricIndex(), clientInfoVector, clientInfoSize));

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (clientInfo.peer_node.GetNodeId() == it->peer_node.GetNodeId())
        {
            ReturnErrorOnFailure(DecreaseEntryCountForFabric(clientInfo.peer_node.GetFabricIndex()));
            clientInfoVector.erase(it);
            break;
        }
    }
    clientInfoVector.push_back(clientInfo);
    size_t total = clientInfoSize * clientInfoVector.size() + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(SerializeToTlv(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(writer.Finalize(backingBuffer));
    ReturnErrorOnFailure(mpClientInfoStore->SyncSetKeyValue(
        DefaultStorageKeyAllocator::ICDClientInfoKey(clientInfo.peer_node.GetFabricIndex()).KeyName(), backingBuffer.Get(),
        static_cast<uint16_t>(len)));

    ReturnErrorOnFailure(IncreaseEntryCountForFabric(clientInfo.peer_node.GetFabricIndex()));
    ChipLogProgress(ICD,
                    "Store ICD entry successfully with peer nodeId " ChipLogFormatScopedNodeId
                    " and checkin nodeId " ChipLogFormatScopedNodeId,
                    ChipLogValueScopedNodeId(clientInfo.peer_node), ChipLogValueScopedNodeId(clientInfo.check_in_node));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::IncreaseEntryCountForFabric(FabricIndex fabricIndex)
{
    return UpdateEntryCountForFabric(fabricIndex, /*increase*/ true);
}

CHIP_ERROR DefaultICDClientStorage::DecreaseEntryCountForFabric(FabricIndex fabricIndex)
{
    return UpdateEntryCountForFabric(fabricIndex, /*increase*/ false);
}

CHIP_ERROR DefaultICDClientStorage::UpdateEntryCountForFabric(FabricIndex fabricIndex, bool increase)
{
    size_t count          = 0;
    size_t clientInfoSize = MaxICDClientInfoSize();
    ReturnErrorOnFailure(LoadCounter(fabricIndex, count, clientInfoSize));
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
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    TLV::TLVType structType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(CounterTag::kCount), static_cast<uint32_t>(count)));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(CounterTag::kSize), static_cast<uint32_t>(clientInfoSize)));
    ReturnErrorOnFailure(writer.EndContainer(structType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(writer.Finalize(backingBuffer));

    return mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName(),
                                              backingBuffer.Get(), static_cast<uint16_t>(len));
}

CHIP_ERROR DefaultICDClientStorage::DeleteEntry(const ScopedNodeId & peerNode)
{
    VerifyOrReturnError(FabricExists(peerNode.GetFabricIndex()), CHIP_NO_ERROR);
    size_t clientInfoSize = 0;
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(peerNode.GetFabricIndex(), clientInfoVector, clientInfoSize));
    VerifyOrReturnError(clientInfoVector.size() > 0, CHIP_NO_ERROR);

    for (auto it = clientInfoVector.begin(); it != clientInfoVector.end(); it++)
    {
        if (peerNode.GetNodeId() == it->peer_node.GetNodeId())
        {
            RemoveKey(*it);
            it = clientInfoVector.erase(it);
            break;
        }
    }

    ReturnErrorOnFailure(
        mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(peerNode.GetFabricIndex()).KeyName()));
    size_t total = clientInfoSize * clientInfoVector.size() + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(SerializeToTlv(writer, clientInfoVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(writer.Finalize(backingBuffer));
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(peerNode.GetFabricIndex()).KeyName(),
                                           backingBuffer.Get(), static_cast<uint16_t>(len)));

    ReturnErrorOnFailure(DecreaseEntryCountForFabric(peerNode.GetFabricIndex()));
    ChipLogProgress(ICD, "Remove ICD entry successfully with peer nodeId " ChipLogFormatScopedNodeId,
                    ChipLogValueScopedNodeId(peerNode));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::DeleteAllEntries(FabricIndex fabricIndex)
{
    VerifyOrReturnError(FabricExists(fabricIndex), CHIP_NO_ERROR);

    size_t clientInfoSize = 0;
    std::vector<ICDClientInfo> clientInfoVector;
    ReturnErrorOnFailure(Load(fabricIndex, clientInfoVector, clientInfoSize));
    IgnoreUnusedVariable(clientInfoSize);
    for (auto & clientInfo : clientInfoVector)
    {
        RemoveKey(clientInfo);
    }
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDClientInfoKey(fabricIndex).KeyName()));
    ReturnErrorOnFailure(
        mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::FabricICDClientInfoCounter(fabricIndex).KeyName()));

    for (auto fabric = mFabricList.begin(); fabric != mFabricList.end(); fabric++)
    {
        if (*fabric == fabricIndex)
        {
            mFabricList.erase(fabric);
            break;
        }
    }

    if (mFabricList.size() == 0)
    {
        return mpClientInfoStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::ICDFabricList().KeyName());
    }

    ReturnErrorOnFailure(StoreFabricList());
    ChipLogProgress(ICD, "Remove all ICD entries successfully for fabric index %u", fabricIndex);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultICDClientStorage::ProcessCheckInPayload(const ByteSpan & payload, ICDClientInfo & clientInfo,
                                                          Protocols::SecureChannel::CounterType & counter)
{
    uint8_t appDataBuffer[kAppDataLength];
    MutableByteSpan appData(appDataBuffer);
    auto * iterator = IterateICDClientInfo();
    VerifyOrReturnError(iterator != nullptr, CHIP_ERROR_NO_MEMORY);
    while (iterator->Next(clientInfo))
    {
        CHIP_ERROR err = chip::Protocols::SecureChannel::CheckinMessage::ParseCheckinMessagePayload(
            clientInfo.aes_key_handle, clientInfo.hmac_key_handle, payload, counter, appData);
        if (CHIP_NO_ERROR == err)
        {
            iterator->Release();
            return CHIP_NO_ERROR;
        }
    }
    iterator->Release();
    return CHIP_ERROR_NOT_FOUND;
}

void DefaultICDClientStorage::Shutdown()
{
    mICDClientInfoIterators.ReleaseAll();
    mpClientInfoStore = nullptr;
    mpKeyStore        = nullptr;
    mFabricList.clear();
}

} // namespace app
} // namespace chip
