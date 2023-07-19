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

#include "LaundryWasherDataProvider.h"
#include <lib/core/TLV.h>
namespace chip {

CHIP_ERROR LaundryWasherDataProvider::StoreSpinSpeedList(EndpointId endpoint, ClusterId clusterId, const SpinSpeedList & spinSpeedList)
{
    uint8_t buffer[kLaundryWasherMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;
    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));
    for (auto const & spinSpeed : spinSpeedList)
    {
        ReturnErrorOnFailure(writer.PutString(TLV::AnonymousTag(), spinSpeed.data()));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::LaundryWasherCtrlSpinSpeedsList(endpoint, clusterId).KeyName(),
                                               buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR LaundryWasherDataProvider::LoadSpinSpeedList(EndpointId endpoint, ClusterId clusterId, SpinSpeedListCharSpan ** pSpinSpeedList, size_t & size)
{
    uint8_t buffer[kLaundryWasherMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    size           = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::LaundryWasherCtrlSpinSpeedsList(endpoint, clusterId).KeyName(), bufferSpan));
    TLV::TLVReader reader;
    TLV::TLVType outerType;
    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    size_t i                 = 0;
    SpinSpeedListCharSpan * head = nullptr;
    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        SpinSpeedListCharSpan * newSpinSpeed = chip::Platform::New<SpinSpeedListCharSpan>();
        if (newSpinSpeed == nullptr)
        {
            ChipLogProgress(Zcl, "Malloc error");
            break;
        }
        newSpinSpeed->Next = nullptr;
        CharSpan readSpinSpeed;
        ReturnErrorOnFailure(reader.Get(readSpinSpeed));

        if (readSpinSpeed.size())
        {
            if (readSpinSpeed.size() <= kSpinSpeedMaxSize)
            {
                char * dest = const_cast<char *>(newSpinSpeed->SpinSpeed_c);
                size_t len  = readSpinSpeed.size();
                memcpy(dest, readSpinSpeed.data(), len);
                newSpinSpeed->spinSpeed = CharSpan::fromCharString(newSpinSpeed->SpinSpeed_c);
            }
            else
            {
                err = CHIP_ERROR_BUFFER_TOO_SMALL;
            }
        }
        else
        {
            chip::Platform::Delete(newSpinSpeed);
            continue;
        }
        i++;
        if (head == nullptr)
        {
            head = newSpinSpeed;
        }
        else
        {
            SpinSpeedListCharSpan * pList = head;
            while (pList->Next != nullptr)
            {
                pList = pList->Next;
            }
            pList->Next = newSpinSpeed;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    size       = i;
    *pSpinSpeedList = head;
    return err;
}

void LaundryWasherDataProvider::ReleaseSpinSpeedList(SpinSpeedListCharSpan * spinSpeedList)
{
    while (spinSpeedList)
    {
        SpinSpeedListCharSpan * del = spinSpeedList;
        spinSpeedList           = spinSpeedList->Next;
        chip::Platform::Delete(del);
    }
}

CHIP_ERROR LaundryWasherDataProvider::ClearSpinSpeedList(EndpointId endpoint, ClusterId clusterId)
{
    return mPersistentStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::LaundryWasherCtrlSpinSpeedsList(endpoint, clusterId).KeyName());
}

CHIP_ERROR LaundryWasherDataProvider::StoreSupportedRinsesList(EndpointId endpoint, ClusterId clusterId, const SupportedRinsesList & supportedRinsesList)
{
    uint8_t buffer[kLaundryWasherMaxSerializedSize];
    TLV::TLVWriter writer;
    TLV::TLVType outerType;
    writer.Init(buffer);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outerType));
    for (auto const & supportedRinse : supportedRinsesList)
    {
        //ReturnErrorOnFailure(writer.PutString(TLV::AnonymousTag(), supportedRinse.data()));
        //ReturnErrorOnFailure(supportedRinse.Encode(writer, TLV::AnonymousTag()));
        ReturnErrorOnFailure(writer.Put(TLV::AnonymousTag(), supportedRinse));
    }
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    return mPersistentStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::LaundryWasherCtrlSupportedRinsesList(endpoint, clusterId).KeyName(),
                                               buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR LaundryWasherDataProvider::LoadSupportedRinsesList(EndpointId endpoint, ClusterId clusterId, SupportedRinsesListSpan ** pSupportedRinsesList, size_t & size)
{
    uint8_t buffer[kLaundryWasherMaxSerializedSize];
    MutableByteSpan bufferSpan(buffer);
    size           = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorOnFailure(Load(DefaultStorageKeyAllocator::LaundryWasherCtrlSupportedRinsesList(endpoint, clusterId).KeyName(), bufferSpan));
    TLV::TLVReader reader;
    TLV::TLVType outerType;
    reader.Init(bufferSpan.data(), bufferSpan.size());
    ReturnErrorOnFailure(reader.Next(TLV::TLVType::kTLVType_Array, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));
    size_t i                 = 0;
    SupportedRinsesListSpan * head = nullptr;
    while (reader.Next() != CHIP_ERROR_END_OF_TLV)
    {
        SupportedRinsesListSpan* newSupportedRinse = chip::Platform::New<SupportedRinsesListSpan>();
        if (newSupportedRinse== nullptr)
        {
            ChipLogProgress(Zcl, "Malloc error");
            break;
        }
        newSupportedRinse->Next = nullptr;
        NumberOfRinsesEnum numberOfRinses;
        ReturnErrorOnFailure(reader.Get(numberOfRinses));
        newSupportedRinse->numberOfRinses = numberOfRinses;
        i++;
        if (head == nullptr)
        {
            head = newSupportedRinse;
        }
        else
        {
            SupportedRinsesListSpan* pList = head;
            while (pList->Next != nullptr)
            {
                pList = pList->Next;
            }
            pList->Next = newSupportedRinse;
        }
    }
    ReturnErrorOnFailure(reader.ExitContainer(outerType));
    size       = i;
    *pSupportedRinsesList = head;
    return err;
}

void LaundryWasherDataProvider::ReleaseSupportedRinsesList(SupportedRinsesListSpan * supportedRinsesList)
{
    while (supportedRinsesList)
    {
        SupportedRinsesListSpan* del = supportedRinsesList;
        supportedRinsesList = supportedRinsesList->Next;
        chip::Platform::Delete(del);
    }
}

CHIP_ERROR LaundryWasherDataProvider::ClearSupportedRinsesList(EndpointId endpoint, ClusterId clusterId)
{
    return mPersistentStorage->SyncDeleteKeyValue(
        DefaultStorageKeyAllocator::LaundryWasherCtrlSupportedRinsesList(endpoint, clusterId).KeyName());
}

CHIP_ERROR LaundryWasherDataProvider::Load(const char * key, MutableByteSpan & buffer)
{
    uint16_t size = static_cast<uint16_t>(buffer.size());
    ReturnErrorOnFailure(mPersistentStorage->SyncGetKeyValue(key, buffer.data(), size));
    buffer = MutableByteSpan(buffer.data(), size);
    return CHIP_NO_ERROR;
}

}
