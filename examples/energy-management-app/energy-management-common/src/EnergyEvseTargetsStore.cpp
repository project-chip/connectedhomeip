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

EvseTargetIteratorImpl * EvseTargetsDelegate::GetEvseTargetsIterator()
{
    if (mEvseTargetsIterator == nullptr)
    {
        // TODO replace this without using new
        EvseTargetsDelegate & dg = *this;
        mEvseTargetsIterator     = new EvseTargetIteratorImpl(dg);
    }
    return mEvseTargetsIterator;
};
EvseTargetsDelegate::~EvseTargetsDelegate()
{
    if (mEvseTargetsIterator)
    {
        delete mEvseTargetsIterator;
    }
}

size_t EvseTargetIteratorImpl::Count()
{
    return static_cast<size_t>(mTargetEntryVector.size());
}
bool EvseTargetIteratorImpl::Next(EvseTargetEntry & entry)
{
    if (mTargetEntryIndex < mTargetEntryVector.size())
    {
        entry = mTargetEntryVector[mTargetEntryIndex];
        mTargetEntryIndex++;
        return true;
    }
    return false;
}

void EvseTargetIteratorImpl::Release()
{
    mTargetEntryIndex = 0;
    mTargetEntryVector.clear();
}

CHIP_ERROR EvseTargetIteratorImpl::Load()
{
    size_t targetsSize = EvseTargetsDelegate::MaxTargetEntrySize();
    ReturnErrorOnFailure(mDelegate.Load(mTargetEntryVector, targetsSize));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsDelegate::Init(PersistentStorageDelegate * targetStore)
{
    ChipLogProgress(AppServer, "EVSE: Initializing EvseTargetsDelegate");
    VerifyOrReturnError(targetStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

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
        count       = 0;
        targetsSize = MaxTargetEntrySize();

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

            evseTarget.targetSoC.ClearValue();   // Optional: Default these to not being set
            evseTarget.addedEnergy.ClearValue(); // Optional: Default these to not being set

            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                auto type = reader.GetType();
                auto tag  = reader.GetTag();
                if (type == TLV::kTLVType_NotSpecified)
                {
                    // Something wrong - we've lost alignment
                    return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
                }
                if (tag == TLV::ContextTag(TargetEntryTag::kTargetTime))
                {
                    ReturnErrorOnFailure(reader.Get(evseTarget.targetTimeMinutesPastMidnight));
                }
                else if (tag == TLV::ContextTag(TargetEntryTag::kTargetSoC))
                {
                    chip::Percent tempSoC;
                    ReturnErrorOnFailure(reader.Get(tempSoC));
                    evseTarget.targetSoC.SetValue(tempSoC);
                }
                else if (tag == TLV::ContextTag(TargetEntryTag::kAddedEnergy))
                {
                    int64_t tempAddedEnergy;
                    ReturnErrorOnFailure(reader.Get(tempAddedEnergy));
                    evseTarget.addedEnergy.SetValue(tempAddedEnergy);
                }
                else
                {
                    // Something else unexpected here
                    return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
                }
            }

            targetEntry.dailyChargingTargets.push_back(evseTarget);
            ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsStructType));
        }
        ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsListType));
        ReturnErrorOnFailure(reader.ExitContainer(EvseTargetEntryType));

        targetEntryVector.push_back(targetEntry);
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));
    return reader.VerifyEndOfContainer();
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
                ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TargetEntryTag::kAddedEnergy), chargingTarget.addedEnergy.Value()));
            }

            ReturnErrorOnFailure(writer.EndContainer(chargingTargetsStructType));
        }
        ReturnErrorOnFailure(writer.EndContainer(chargingTargetsListType));
        ReturnErrorOnFailure(writer.EndContainer(EvseTargetEntryType));
    }
    return writer.EndContainer(arrayType);
}

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
CHIP_ERROR EvseTargetsDelegate::CopyTarget(const Structs::ChargingTargetScheduleStruct::DecodableType & newDataModelEntry)
{
    uint8_t bitmaskA;
    uint8_t bitmaskB;

    std::vector<EvseTargetEntry> targetEntryVector;
    size_t targetEntrySize = MaxTargetEntrySize();
    ReturnErrorOnFailure(Load(targetEntryVector, targetEntrySize));

    uint8_t bitmask = newDataModelEntry.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(0x7F));
    ChipLogProgress(AppServer, "DayOfWeekForSequence = 0x%02x", bitmask);

    int8_t index = 0;
    for (auto entry = targetEntryVector.begin(); entry != targetEntryVector.end(); /* No increment here */)
    {
        uint8_t entryBitmask = entry->dayOfWeekMap.GetField(static_cast<TargetDayOfWeekBitmap>(0x7F));
        ChipLogProgress(AppServer, " scanning existing entry %d of %d: bitmap 0x%02x", index,
                        static_cast<unsigned int>(targetEntryVector.size()), entryBitmask);
        bitmaskA = static_cast<uint8_t>(entryBitmask & bitmask);
        bitmaskB = static_cast<uint8_t>(entryBitmask & ~bitmask);
        if (entryBitmask == bitmaskA)
        {
            /* This entry has the all the same bits as the newEntry
             * Delete this entry - we don't copy it
             */
            entry->dailyChargingTargets.clear();
            entry = targetEntryVector.erase(entry);
            ReturnErrorOnFailure(DecreaseEntryCount());
            ChipLogProgress(AppServer, " ERASED");
        }
        else
        {
            /* this entry stays - but it has lost some days from the bitmask */
            entry->dayOfWeekMap = BitMask<TargetDayOfWeekBitmap>(bitmaskB);
            ++entry;
        }
        index++;
    }

    // Add the new Entry - we have to convert it from DataModel format to storage format
    EvseTargetEntry newStorageEntry;
    newStorageEntry.dayOfWeekMap = newDataModelEntry.dayOfWeekForSequence;

    auto it = newDataModelEntry.chargingTargets.begin();
    while (it.Next())
    {
        auto & chargingTargetStruct = it.GetValue();
        EvseChargingTarget temp;

        temp.targetTimeMinutesPastMidnight = chargingTargetStruct.targetTimeMinutesPastMidnight;
        temp.targetSoC                     = chargingTargetStruct.targetSoC;
        temp.addedEnergy                   = chargingTargetStruct.addedEnergy;
        newStorageEntry.dailyChargingTargets.push_back(temp);
    }

    targetEntryVector.push_back(newStorageEntry);

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
    /* We simply delete the data from the persistent store */
    mpTargetStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::EVSETargets().KeyName());
    mpTargetStore->SyncDeleteKeyValue(DefaultStorageKeyAllocator::EvseTargetEntryCounter().KeyName());
    return CHIP_NO_ERROR;
}
