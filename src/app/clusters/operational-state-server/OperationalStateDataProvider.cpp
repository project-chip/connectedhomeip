/*
 *
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

#include "OperationalStateDataProvider.h"

#include <lib/core/TLV.h>
namespace chip {

CHIP_ERROR OperationalStateDataProvider::LoadOperationalStateList(EndpointId endpoint, ClusterId clusterId,
                                                                  OperationalStateStructDynamicList ** operationalStateList,
                                                                  size_t & size)
{
    uint8_t buffer[kOperationalStateMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    size           = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::OperationalStateOpStateList(endpoint, clusterId).KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    size_t i                                 = 0;
    OperationalStateStructDynamicList * head = nullptr;
    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        OperationalStateStructDynamicList * state = chip::Platform::New<OperationalStateStructDynamicList>();
        if (state == nullptr)
        {
            ChipLogProgress(Zcl, "Malloc error");
            break;
        }
        OperationalStateStructType operationalState;
        ReturnErrorOnFailure(operationalState.Decode(reader));

        state->Next               = nullptr;
        state->operationalStateID = operationalState.operationalStateID;
        if (operationalState.operationalStateLabel.size())
        {
            if (operationalState.operationalStateLabel.size() <= kOperationalStateLabelMaxSize)
            {
                char * dest = const_cast<char *>(state->OperationalStateLabel);
                size_t len  = operationalState.operationalStateLabel.size();
                memcpy(dest, operationalState.operationalStateLabel.data(), len);
                state->operationalStateLabel = CharSpan::fromCharString(state->OperationalStateLabel);
            }
            else
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
        }
        else
        {
        }
        i++;
        if (head == nullptr)
        {
            head = state;
        }
        else
        {
            OperationalStateStructDynamicList * pList = head;
            while (pList->Next != nullptr)
            {
                pList = pList->Next;
            }
            pList->Next = state;
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    size                  = i;
    *operationalStateList = head;

    return err;
}

void OperationalStateDataProvider::ReleaseOperationalStateList(OperationalStateStructDynamicList * operationalStateList)
{
    while (operationalStateList)
    {
        OperationalStateStructDynamicList * del = operationalStateList;
        operationalStateList                    = operationalStateList->Next;
        chip::Platform::Delete(del);
    }
}

CHIP_ERROR OperationalStateDataProvider::ClearOperationalStateList(EndpointId endpoint, ClusterId clusterId)
{
    return mPersistentStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::OperationalStateOpStateList(endpoint, clusterId).KeyName());
}

CHIP_ERROR OperationalStateDataProvider::UseOpStateIDGetOpStateStruct(EndpointId endpoint, ClusterId clusterId,
                                                                      OperationalStateStructDynamicList & targetOp)
{
    uint8_t buffer[kOperationalStateMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::OperationalStateOpStateList(endpoint, clusterId).KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        OperationalStateStructType operationalState;
        ReturnErrorOnFailure(operationalState.Decode(reader));

        if (targetOp.operationalStateID != operationalState.operationalStateID)
            continue;

        char * dest = const_cast<char *>(targetOp.OperationalStateLabel);
        size_t len  = operationalState.operationalStateLabel.size();
        memcpy(dest, operationalState.operationalStateLabel.data(), len);
        targetOp.operationalStateLabel = CharSpan::fromCharString(targetOp.OperationalStateLabel);
        break;
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));

    return err;
}

CHIP_ERROR OperationalStateDataProvider::StorePhaseList(EndpointId endpoint, ClusterId clusterId, const PhaseList & phaseList)
{
    uint8_t buffer[kOperationalStateMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;

    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));

    for (auto const & phase : phaseList)
    {
        ReturnErrorOnFailure(writer.PutString(TLV::AnonymousTag(), phase.data()));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerType));

    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::OperationalStatePhaseList(endpoint, clusterId).KeyName(),
                                               buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR OperationalStateDataProvider::LoadPhaseList(EndpointId endpoint, ClusterId clusterId, PhaseListCharSpan ** phaseList,
                                                       size_t & size)
{
    uint8_t buffer[kOperationalStateMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    size           = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::OperationalStatePhaseList(endpoint, clusterId).KeyName(), bufferSpan));

    TLV::TLVReader reader;
    TLV::TLVType outerType;

    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    size_t i                 = 0;
    PhaseListCharSpan * head = nullptr;
    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        PhaseListCharSpan * newPhase = chip::Platform::New<PhaseListCharSpan>();
        if (newPhase == nullptr)
        {
            ChipLogProgress(Zcl, "Malloc error");
            break;
        }
        newPhase->Next = nullptr;

        CharSpan readPhase;
        ReturnErrorOnFailure(reader.Get(readPhase));

        if (readPhase.size())
        {
            if (readPhase.size() <= kPhaseMaxSize)
            {
                char * dest = const_cast<char *>(newPhase->Phase);
                size_t len  = readPhase.size();
                memcpy(dest, readPhase.data(), len);
                newPhase->phase = CharSpan::fromCharString(newPhase->Phase);
            }
            else
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
        }
        else
        {
            chip::Platform::Delete(newPhase);
            continue;
        }
        i++;
        if (head == nullptr)
        {
            head = newPhase;
        }
        else
        {
            PhaseListCharSpan * pList = head;
            while (pList->Next != nullptr)
            {
                pList = pList->Next;
            }
            pList->Next = newPhase;
        }
    }

    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    size       = i;
    *phaseList = head;

    return err;
}

void OperationalStateDataProvider::ReleasePhaseList(PhaseListCharSpan * phaseList)
{
    while (phaseList)
    {
        PhaseListCharSpan * del = phaseList;
        phaseList               = phaseList->Next;
        chip::Platform::Delete(del);
    }
}

CHIP_ERROR OperationalStateDataProvider::ClearPhaseStateList(EndpointId endpoint, ClusterId clusterId)
{
    return mPersistentStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::OperationalStatePhaseList(endpoint, clusterId).KeyName());
}

CHIP_ERROR OperationalStateDataProvider::Load(const char * key, MutableByteSpan & buffer)
{
    uint16_t size = static_cast<uint16_t>(buffer.size());
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key, buffer.data(), size));

    buffer.reduce_size(size);
    return CHIP_NO_ERROR;
}

} // namespace chip
