/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <EnergyEvseTargetsStore.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;

// using namespace chip::app::Clusters::EnergyEvse::Attributes;

#if 0

CHIP_ERROR EvseTargetsStorage::DecodableEntry::Decode(TLV::TLVReader & reader)
{
    ReturnErrorOnFailure(mStagingEntry.Decode(reader));
    ReturnErrorOnFailure(Unstage());
    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsStorage::DecodableEntry::Unstage()
{
    // ReturnErrorOnFailure(GetEvseTargets().PrepareEntry(mEntry));

    // Not sure we need this unstage?
    /*May need code to  auto iterator = mStagingEntry.subjects.Value().begin();
        while (iterator.Next())
        {
            StagingSubject tmp = { .nodeId = iterator.GetValue(), .authMode = mStagingEntry.authMode };
            NodeId subject;
            ReturnErrorOnFailure(Convert(tmp, subject));
            ReturnErrorOnFailure(mEntry.AddSubject(nullptr, subject));
        }
        ReturnErrorOnFailure(iterator.GetStatus());
        */
    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsStorage::EncodableEntry::EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    ReturnErrorOnFailure(Stage());
    //    ReturnErrorOnFailure(mStagingEntry.EncodeForRead(writer, tag));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsStorage::EncodableEntry::EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    ReturnErrorOnFailure(Stage());
    // TODO ReturnErrorOnFailure(mStagingEntry.EncodeForWrite(writer, tag));
    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsStorage::EncodableEntry::Stage() const
{
    /* May not need this??? */

    // {
    //     size_t count;
    //     ReturnErrorOnFailure(mEntry.GetTargetCount(count));
    //     if (count > 0)
    //     {
    //         for (size_t i = 0; i < count; ++i)
    //         {
    //             Target target;
    //             ReturnErrorOnFailure(mEntry.GetTarget(i, target));
    //             ReturnErrorOnFailure(Convert(target, mStagingTargets[i]));
    //         }
    //         mStagingEntry.targets.SetNonNull(mStagingTargets, count);
    //     }
    //     else
    //     {
    //         mStagingEntry.targets.SetNull();
    //     }
    // }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsStorage::Init(PersistentStorageDelegate & persistentStorage)
{
    ChipLogProgress(AppServer, "EvseTargetsStorage: initializing");

    // TODO
    static const uint16_t kEncodedEntryTotalBytes = 1170;

    CHIP_ERROR err;

    size_t count = 0;

    for (size_t index = 0; /**/; ++index)
    {
        uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };
        uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
        err = persistentStorage.SyncGetKeyValue(DefaultStorageKeyAllocator::EVSETargets().KeyName(), buffer, size);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            break;
        }
        SuccessOrExit(err);

        TLV::TLVReader reader;
        reader.Init(buffer, size);
        SuccessOrExit(err = reader.Next());

        DecodableEntry decodableEntry;
        SuccessOrExit(err = decodableEntry.Decode(reader));

        // Entry & entry = decodableEntry.GetEntry();

        // TODO add createentry
        // SuccessOrExit(err = GetEvseTargets().CreateEntry(entry));
        count++;
    }
    ChipLogProgress(AppServer, "EvseTargetsStorage: %u entries loaded", (unsigned) count);
    return CHIP_NO_ERROR;

exit:
    ChipLogError(AppServer, "EvseTargetsStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}


CHIP_ERROR EvseTargetsStore::LoadTargetsFromStore()
{
    // TODO reinstate this one
    //  uint8_t buf[kEnergyEvseTargetsTLVMaxSize];
    //  uint16_t size = sizeof(buf);

    // ReturnErrorOnFailure(mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::EVSETargets().KeyName(), buf, size));

    // TLV::ContiguousBufferTLVReader reader;
    // reader.Init(buf, size);

    // SuccessOrExit(err = reader.Next());

    // DecodableEntry decodableEntry;
    // SuccessOrExit(err = decodableEntry.Decode(reader));

    // exit:

    return err;
}
#endif
/**
 * This function tries to compress a list of entries which has:
 *  dayOfWeek bitmask
 *  chargingTargetsList
 *
 * It takes a new entry and scans the existing list to see if the
 * dayOfWeek bitmask is already included somewhere
 *
 *   compute bitmask values:
 *
 *   bitmaskA: (entry.bitmask & bitmask)
 *      work out which bits in the existing entry are the same (overlapping)
 *
 *   Create and append a new entry for the bits that are the same
 *      newEntry.bitmask = bitmaskA;
 *      newEntry.chargingTargetsList = chargingTargetsList
 *
 *      if entry.bitmask == bitmaskA
 *         this entry is being deleted and can share the newEntry
 *         delete it
 *
 *   bitmaskB = (entry.bitmask & ~bitmask);
 *      work out which bits in the existing entry are different
 *      Remove these bits from the existing entry, (effectively deleting them)
 *          entry.bitmask = bitmaskB;
 *
 *      NOTE: if `all` bits are removed then the existing entry can be deleted,
 *      but that's not possible because we check for a full match first
 *
 * We continue walking our list to see if other entries have overlapping bits
 * If they do, then the newEntry.bitmask |= bitmaskA
 *
 */
CHIP_ERROR EvseTargetsDelegate::CopyTarget(const Structs::ChargingTargetScheduleStruct::DecodableType & newEntry)
{
    // Structs::ChargingTargetScheduleStruct::DecodableType createdEntry;
    // uint8_t newEntryBitmask = 0;
    // uint8_t bitmaskA;
    // uint8_t bitmaskB;

    // DataModel::List<Structs::ChargingTargetScheduleStruct::DecodableType> updatedList;

    // uint8_t bitmask = newEntry.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(0x7F));
    // ChipLogProgress(AppServer, "DayOfWeekForSequence = 0x%02x", bitmask);

    // auto iter = mTargets.begin();
    // while (iter->Next())
    // {
    //     auto & entry         = iter->GetValue();
    //     uint8_t entryBitmask = entry.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(0x7F));

    //     bitmaskA = entryBitmask & bitmask;
    //     bitmaskB = entryBitmask & ~bitmask;
    //     newEntryBitmask |= bitmaskA;
    //     if (entryBitmask == bitmaskA)
    //     {
    //         /* This entry has the all the same bits as the newEntry
    //          * Delete this entry - we don't copy it
    //          */
    //     }
    //     else
    //     {
    //         /* this entry stays - but it has lost some days from the bitmask */
    //         entry.dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(bitmaskB);
    //         //  updatedList.Put(entry);
    // }

    // /* Append new entry */
    // ChipLogDetail(AppServer, "Adding new entry with bitmask 0x%02x", newEntryBitmask);
    // createdEntry.dayOfWeekForSequence = static_cast<TargetDayOfWeekBitmap>(newEntryBitmask);
    // createdEntry.chargingTargets      = newEntry.chargingTargets;
    // // updatedList.Put(createdEntry);

    // /* delete our original mTargets */
    // mTargets = updatedList; // TODO check this doesn't cause a memory leak?

    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsDelegate::Init(PersistentStorageDelegate * targetStore)
{
    VerifyOrReturnError(targetStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mpTargetStore == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpTargetStore = targetStore;

    return CHIP_NO_ERROR;
}
CHIP_ERROR EvseTargetsDelegate::LoadCounter(size_t & count, size_t & targetsSize)
{
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    size_t len = MaxTargetEntryCounterSize();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    uint16_t length = static_cast<uint16_t>(len);

    CHIP_ERROR err =
        mpTargetStore->SyncGetKeyValue(DefaultStorageKeyAllocator::EvseTargetEntryCounter().KeyName(), backingBuffer.Get(), length);
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

    uint32_t tempTargetsSize = 0;
    ReturnErrorOnFailure(reader.Next(TLV::ContextTag(CounterTag::kSize)));
    ReturnErrorOnFailure(reader.Get(tempTargetsSize));
    targetsSize = static_cast<size_t>(tempTargetsSize);

    ReturnErrorOnFailure(reader.ExitContainer(structType));
    return reader.VerifyEndOfContainer();
}
CHIP_ERROR EvseTargetsDelegate::IncreaseEntryCount()
{
    return UpdateEntryCount(/*increase*/ true);
}

CHIP_ERROR EvseTargetsDelegate::DecreaseEntryCount()
{
    return UpdateEntryCount(/*increase*/ false);
}
CHIP_ERROR EvseTargetsDelegate::UpdateEntryCount(bool increase)
{
    size_t count       = 0;
    size_t targetsSize = MaxTargetEntrySize();
    ReturnErrorOnFailure(LoadCounter(count, targetsSize));
    if (increase)
    {
        count++;
    }
    else
    {
        count--;
    }

    size_t total = MaxTargetEntryCounterSize();
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    TLV::TLVType structType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, structType));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(CounterTag::kCount), static_cast<uint32_t>(count)));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(CounterTag::kSize), static_cast<uint32_t>(targetsSize)));
    ReturnErrorOnFailure(writer.EndContainer(structType));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    writer.Finalize(backingBuffer);

    return mpTargetStore->SyncSetKeyValue(DefaultStorageKeyAllocator::EvseTargetEntryCounter().KeyName(), backingBuffer.Get(),
                                          static_cast<uint16_t>(len));
}

CHIP_ERROR EvseTargetsDelegate::Load(std::vector<EvseTargetEntry> & targetEntryVector, size_t & targetsSize)
{
    size_t count = 0;

    ReturnErrorOnFailure(LoadCounter(count, targetsSize));
    size_t len = targetsSize * count + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(!backingBuffer.Calloc(len), CHIP_ERROR_NO_MEMORY);
    uint16_t length = static_cast<uint16_t>(len);
    CHIP_ERROR err =
        mpTargetStore->SyncGetKeyValue(DefaultStorageKeyAllocator::EVSETargets().KeyName(), backingBuffer.Get(), length);
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
        EvseTargetEntry targetEntry;
        TLV::TLVType EvseTargetEntryType;

        ReturnErrorOnFailure(reader.EnterContainer(EvseTargetEntryType));
        // DayOfWeek bitmap
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TargetEntryTag::kDayOfWeek)));
        ReturnErrorOnFailure(reader.Get(targetEntry.dayOfWeekMap));

        // ChargingTargets List
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_List, TLV::ContextTag(TargetEntryTag::kChargingTargetsList)));
        TLV::TLVType chargingTargetsListType;
        ReturnErrorOnFailure(reader.EnterContainer(chargingTargetsListType));

        while ((err = reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(TargetEntryTag::kChargingTargetsStruct))) ==
               CHIP_NO_ERROR)
        {
            EvseChargingTarget evseTarget;
            TLV::TLVType chargingTargetsStructType = TLV::kTLVType_Structure;
            ReturnErrorOnFailure(reader.EnterContainer(chargingTargetsStructType));

            // targetTime
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TargetEntryTag::kTargetTime)));
            ReturnErrorOnFailure(reader.Get(evseTarget.targetTimeMinutesPastMidnight));

            // targetSoC (Optional)
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TargetEntryTag::kTargetSoC)));
            if (reader.GetType() == TLV::kTLVType_Null)
            {
                evseTarget.targetSoC.ClearValue();
            }
            else
            {
                chip::Percent tempSoC;
                ReturnErrorOnFailure(reader.Get(tempSoC));
                evseTarget.targetSoC.SetValue(tempSoC);
            }

            // addedEnergy (Optional)
            ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TargetEntryTag::kAddedEnergy)));
            // TODO is this TLVNULL right?
            if (reader.GetType() == TLV::kTLVType_Null)
            {
                evseTarget.addedEnergy.ClearValue();
            }
            else
            {
                int64_t tempAddedEnergy;
                ReturnErrorOnFailure(reader.Get(tempAddedEnergy));
                evseTarget.addedEnergy.SetValue(tempAddedEnergy);
            }

            targetEntry.dailyChargingTargets.push_back(evseTarget);
            ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsStructType));
        }
        ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsListType));
        ReturnErrorOnFailure(reader.ExitContainer(EvseTargetEntryType));

        targetEntryVector.push_back(targetEntry);
    }

    if (err != CHIP_END_OF_TLV)
    {
        return err;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();

    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsDelegate::SerializeToTlv(TLV::TLVWriter & writer, const std::vector<EvseTargetEntry> & targetEntryVector)
{
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & targetEntry : targetEntryVector)
    {
        TLV::TLVType EvseTargetEntryType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, EvseTargetEntryType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TargetEntryTag::kDayOfWeek), targetEntry.dayOfWeekMap));

        TLV::TLVType chargingTargetsListType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(TargetEntryTag::kChargingTargetsList), TLV::kTLVType_List,
                                                   chargingTargetsListType));
        for (auto & chargingTarget : targetEntry.dailyChargingTargets)
        {
            TLV::TLVType chargingTargetsStructType = TLV::kTLVType_Structure;
            ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(TargetEntryTag::kChargingTargetsStruct),
                                                       TLV::kTLVType_Structure, chargingTargetsStructType));
            ReturnErrorOnFailure(
                writer.Put(TLV::ContextTag(TargetEntryTag::kTargetTime), chargingTarget.targetTimeMinutesPastMidnight));
            if (chargingTarget.targetSoC.HasValue())
            {
                ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TargetEntryTag::kTargetSoC), chargingTarget.targetSoC.Value()));
            }

            if (chargingTarget.addedEnergy.HasValue())
            {
                // TODO should we be putting in a NULL instead?
                ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TargetEntryTag::kAddedEnergy), chargingTarget.addedEnergy.Value()));
            }

            ReturnErrorOnFailure(writer.EndContainer(chargingTargetsStructType));
        }
        ReturnErrorOnFailure(writer.EndContainer(chargingTargetsListType));
        ReturnErrorOnFailure(writer.EndContainer(EvseTargetEntryType));
    }
    return writer.EndContainer(arrayType);
}

CHIP_ERROR EvseTargetsDelegate::StoreEntry(const EvseTargetEntry & entry)
{
    std::vector<EvseTargetEntry> targetEntryVector;
    size_t targetEntrySize = MaxTargetEntrySize();
    ReturnErrorOnFailure(Load(targetEntryVector, targetEntrySize));

    for (auto it = targetEntryVector.begin(); it != targetEntryVector.end(); it++)
    {
        // TODO work out how we want to overwrite a targetBitmap
        // ReturnErrorOnFailure(IncreaseEntryCount());
        // targetEntryVector.erase(it);
        // break;
    }

    targetEntryVector.push_back(entry);

    size_t total = targetEntrySize * targetEntryVector.size() + kArrayOverHead;
    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    ReturnErrorOnFailure(SerializeToTlv(writer, targetEntryVector));

    const auto len = writer.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(len), CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Finalize(backingBuffer);
    ReturnErrorOnFailure(mpTargetStore->SyncSetKeyValue(DefaultStorageKeyAllocator::EVSETargets().KeyName(), backingBuffer.Get(),
                                                        static_cast<uint16_t>(len)));

    return IncreaseEntryCount();
}

CHIP_ERROR EvseTargetsDelegate::ClearTargets()
{
    EvseTargetIteratorImpl * iterator = GetTargetsIterator();
    EvseTargetEntry entry;
    CHIP_ERROR err;

    while (iterator->Next(entry))
    {
        err = DeleteEntry(entry);
        if (CHIP_NO_ERROR == err)
        {
            iterator->Release();
            return CHIP_NO_ERROR;
        }
    }
    iterator->Release();
    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsDelegate::CreateEntry(EvseTargetEntry & entry)
{

#if 0
    EvseTargetIteratorImpl * iterator = GetTargetsIterator();
    CHIP_ERROR err;

    while (iterator->Next(entry))
    {
        err = DeleteEntry(entry);
        if (CHIP_NO_ERROR == err)
        {
            iterator->Release();
            return CHIP_NO_ERROR;
        }
    }
    iterator->Release();
#endif
    return CHIP_NO_ERROR;
}
CHIP_ERROR EvseTargetsDelegate::DeleteEntry(EvseTargetEntry & entry)
{

    return CHIP_NO_ERROR;
}
CHIP_ERROR EvseTargetsDelegate::UpdateEntry(EvseTargetEntry & entry)
{

    return CHIP_NO_ERROR;
}

EvseTargetsDelegate::EvseTargetIteratorImpl * EvseTargetsDelegate::GetTargetsIterator()
{
    return mEvseTargetsIterators.CreateObject(*this);
}

size_t EvseTargetsDelegate::EvseTargetIteratorImpl::Count()
{
    return static_cast<size_t>(mTargetEntryVector.size());
}
bool EvseTargetsDelegate::EvseTargetIteratorImpl::Next(EvseTargetEntry & entry)
{
    if (mTargetEntryIndex < mTargetEntryVector.size())
    {
        entry = mTargetEntryVector[mTargetEntryIndex];
        mTargetEntryIndex++;
        return true;
    }
    return false;
}

void EvseTargetsDelegate::EvseTargetIteratorImpl::Release()
{
    return mDelegate.mEvseTargetsIterators.ReleaseObject(this);
}