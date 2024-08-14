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

#include <EnergyEvseDelegateImpl.h>
#include <EnergyEvseTargetsStore.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/server/Server.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using chip::Protocols::InteractionModel::Status;

EvseTargetsDelegate::EvseTargetsDelegate() {}

EvseTargetsDelegate::~EvseTargetsDelegate() {}

CHIP_ERROR EvseTargetsDelegate::Init(PersistentStorageDelegate * targetStore)
{
    ChipLogProgress(AppServer, "EVSE: Initializing EvseTargetsDelegate");
    VerifyOrReturnError(targetStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mpTargetStore = targetStore;

    // Set FabricDelegate
    chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(this);

    return CHIP_NO_ERROR;
}

const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & EvseTargetsDelegate::GetTargets()
{
    return mChargingTargetSchedulesList;
}

/* static */
uint16_t EvseTargetsDelegate::GetTlvSizeUpperBound()
{
    size_t kListOverhead = 4;
    size_t chargingTargetStuctEstimate =
        TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(Optional<chip::Percent>), sizeof(Optional<int64_t>));
    size_t chargingTargetScheduleStructEstimate = TLV::EstimateStructOverhead(sizeof(chip::BitMask<TargetDayOfWeekBitmap>)) +
        kListOverhead + kEvseTargetsMaxTargetsPerDay * chargingTargetStuctEstimate;
    size_t totalEstimate = kEvseTargetsMaxNumberOfDays * chargingTargetScheduleStructEstimate + kListOverhead;

    return static_cast<uint16_t>(totalEstimate);
}

CHIP_ERROR EvseTargetsDelegate::LoadTargets()
{
    // The DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> data structure contains a list of
    // ChargingTargetScheduleStructs which in turn contains a list of ChargingTargetStructs. Lists contain pointers
    // to objects allocated outside of the List. For mChargingTargetSchedulesList, that memory is allocated in
    // mChargingTargets and mChargingTargetSchedulesArray.

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    uint16_t length = GetTlvSizeUpperBound();
    ReturnErrorCodeIf(!backingBuffer.Calloc(length), CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err = mpTargetStore->SyncGetKeyValue(spEvseTargetsKeyName, backingBuffer.Get(), length);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // Targets does not exist persistent storage -> initialise mChargingTargetSchedulesList as empty
        mChargingTargetSchedulesList = DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>();

        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), length);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    uint16_t chargingTargetSchedulesIdx = 0;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        TLV::TLVType evseTargetEntryType;

        ReturnErrorOnFailure(reader.EnterContainer(evseTargetEntryType));

        // Check we are not exceeding the size of the mChargingTargetSchedulesArray
        VerifyOrReturnError(chargingTargetSchedulesIdx < kEvseTargetsMaxNumberOfDays, CHIP_ERROR_INCORRECT_STATE);

        // DayOfWeek bitmap
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TargetEntryTag::kDayOfWeek)));
        ReturnErrorOnFailure(reader.Get(mChargingTargetSchedulesArray[chargingTargetSchedulesIdx].dayOfWeekForSequence));

        ChipLogProgress(AppServer, "LoadTargets: DayOfWeekForSequence = 0x%02x",
                        mChargingTargetSchedulesArray[chargingTargetSchedulesIdx].dayOfWeekForSequence.GetField(
                            static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask)));

        // ChargingTargets List
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_List, TLV::ContextTag(TargetEntryTag::kChargingTargetsList)));
        TLV::TLVType chargingTargetsListType;
        ReturnErrorOnFailure(reader.EnterContainer(chargingTargetsListType));

        // The mChargingTargets object handles the allocation of the chargingTargets. Let it know the currentSchedule index
        mChargingTargets.PrepareDaySchedule(chargingTargetSchedulesIdx);

        // Load the chargingTargets associated with this schedule
        while ((err = reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(TargetEntryTag::kChargingTargetsStruct))) ==
               CHIP_NO_ERROR)
        {
            TLV::TLVType chargingTargetsStructType = TLV::kTLVType_Structure;
            ReturnErrorOnFailure(reader.EnterContainer(chargingTargetsStructType));

            // Keep track of the current chargingTarget being loaded
            EnergyEvse::Structs::ChargingTargetStruct::Type chargingTarget;

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
                    ReturnErrorOnFailure(reader.Get(chargingTarget.targetTimeMinutesPastMidnight));
                }
                else if (tag == TLV::ContextTag(TargetEntryTag::kTargetSoC))
                {
                    chip::Percent tempSoC;
                    ReturnErrorOnFailure(reader.Get(tempSoC));
                    chargingTarget.targetSoC.SetValue(tempSoC);
                }
                else if (tag == TLV::ContextTag(TargetEntryTag::kAddedEnergy))
                {
                    int64_t tempAddedEnergy;
                    ReturnErrorOnFailure(reader.Get(tempAddedEnergy));
                    chargingTarget.addedEnergy.SetValue(tempAddedEnergy);
                }
                else
                {
                    // Something else unexpected here
                    return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsStructType));

            ChipLogProgress(AppServer,
                            "LoadingTargets: targetTimeMinutesPastMidnight %u targetSoC %u addedEnergy 0x" ChipLogFormatX64,
                            chargingTarget.targetTimeMinutesPastMidnight, chargingTarget.targetSoC.ValueOr(0),
                            ChipLogValueX64(chargingTarget.addedEnergy.ValueOr(0)));

            // Update mChargingTargets which is tracking the chargingTargets
            mChargingTargets.AddChargingTarget(chargingTarget);
        }

        ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsListType));
        ReturnErrorOnFailure(reader.ExitContainer(evseTargetEntryType));

        // Allocate an array for the chargingTargets loaded for this schedule and copy the chargingTargets into that array.
        // The allocated array will be pointed to in the List below.
        err = mChargingTargets.AllocAndCopy();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SetTargets: Failed to allocate memory during LoadTargets %s", chip::ErrorStr(err));
            return err;
        }

        // Construct the List<ChargingTargetStruct>. mChargingTargetSchedulesArray will be pointed to in the
        // List<ChargingTargetScheduleStruct> mChargingTargetSchedulesList below
        mChargingTargetSchedulesArray[chargingTargetSchedulesIdx].chargingTargets =
            chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
                mChargingTargets.GetChargingTargets(), mChargingTargets.GetNumDailyChargingTargets());

        chargingTargetSchedulesIdx++;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));

    // Finalise mChargingTargetSchedulesList
    mChargingTargetSchedulesList = DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(mChargingTargetSchedulesArray,
                                                                                                      chargingTargetSchedulesIdx);

    return reader.VerifyEndOfContainer();
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
CHIP_ERROR EvseTargetsDelegate::SetTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & newChargingTargetSchedules)
{
    ChipLogProgress(AppServer, "SetTargets");

    // We'll need to have a local copy of the chargingTargets that are referenced from updatedChargingTargetSchedules (which
    // is a List<ChargingTargetScheduleStruct> where each ChargingTargetScheduleStruct has a List of ChargingTargetStructs).
    // Note updatedChargingTargets only needs to exist for the duration of this method as once the new targets have been merged
    // with the existing targets, we'll save the updated Targets structure to persistent storage and the reload it into
    // mChargingTargetSchedulesList
    ChargingTargetsMemMgr updatedChargingTargets;

    // Build up a new Targets structure
    DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> updatedChargingTargetSchedules;

    // updatedChargingTargetSchedules contains a List of ChargingTargetScheduleStruct where the memory of
    // ChargingTargetScheduleStruct is which is allocated here.
    Structs::ChargingTargetScheduleStruct::Type updatedChargingTargetSchedulesArray[kEvseTargetsMaxNumberOfDays];

    // Iterate across the list of new schedules. For each schedule, iterate through the existing Target
    // (mChargingTargetSchedulesList) working out how to merge the new schedule.
    auto newIter = newChargingTargetSchedules.begin();
    while (newIter.Next())
    {
        auto & newChargingTargetSchedule = newIter.GetValue();

        uint8_t newBitmask =
            newChargingTargetSchedule.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask));

        ChipLogProgress(AppServer, "SetTargets: DayOfWeekForSequence = 0x%02x", newBitmask);

        PrintTargets(mChargingTargetSchedulesList);

        // Iterate across the existing schedule entries, seeing if there is overlap with
        // the dayOfWeekForSequenceBitmap
        bool found                                 = false;
        uint16_t updatedChargingTargetSchedulesIdx = 0;

        // Let the updatedChargingTargets object of the schedule index
        updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);

        for (auto & currentChargingTargetSchedule : mChargingTargetSchedulesList)
        {
            uint8_t currentBitmask =
                currentChargingTargetSchedule.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask));

            ChipLogProgress(AppServer, "SetTargets: Scanning current entry %d of %d: bitmap 0x%02x",
                            updatedChargingTargetSchedulesIdx, static_cast<unsigned int>(mChargingTargetSchedulesList.size()),
                            currentBitmask);

            // Work out if the new schedule dayOfWeekSequence overlaps with any existing schedules
            uint8_t bitmaskA = static_cast<uint8_t>(currentBitmask & newBitmask);
            uint8_t bitmaskB = static_cast<uint8_t>(currentBitmask & ~newBitmask);

            BitMask<TargetDayOfWeekBitmap> updatedBitmask;

            if (currentBitmask == bitmaskA)
            {
                // This entry has the all the same bits as the newEntry
                updatedBitmask = BitMask<TargetDayOfWeekBitmap>(bitmaskA);

                // Copy the new chargingTargets to this schedule index
                CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(newChargingTargetSchedule.chargingTargets);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets %s", chip::ErrorStr(err));
                    return err;
                }

                found = true;
            }
            else
            {
                // This entry stays - but it has lost some days from the bitmask
                updatedBitmask = BitMask<TargetDayOfWeekBitmap>(bitmaskB);

                // Copy the existing chargingTargets
                CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(currentChargingTargetSchedule.chargingTargets);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets %s", chip::ErrorStr(err));
                    return err;
                }
            }

            // Update the new schedule with the dayOfWeekForSequence and list of chargingTargets
            updatedChargingTargetSchedulesArray[updatedChargingTargetSchedulesIdx].dayOfWeekForSequence = updatedBitmask;

            updatedChargingTargetSchedulesArray[updatedChargingTargetSchedulesIdx].chargingTargets =
                chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
                    updatedChargingTargets.GetChargingTargets(), updatedChargingTargets.GetNumDailyChargingTargets());

            // Going to look at the next schedule entry
            updatedChargingTargetSchedulesIdx++;

            // Let the updatedChargingTargets object of the schedule index
            updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);
        }

        // If found is false, then there were no existing entries for the dayOfWeekForSequence. Add a new entry
        if (!found)
        {
            // Copy the new chargingTargets
            CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(newChargingTargetSchedule.chargingTargets);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets %s", chip::ErrorStr(err));
                return err;
            }

            // Update the new schedule with the dayOfWeekForSequence and list of chargingTargets
            updatedChargingTargetSchedulesArray[updatedChargingTargetSchedulesIdx].dayOfWeekForSequence =
                newChargingTargetSchedule.dayOfWeekForSequence;

            updatedChargingTargetSchedulesArray[updatedChargingTargetSchedulesIdx].chargingTargets =
                chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
                    updatedChargingTargets.GetChargingTargets(), updatedChargingTargets.GetNumDailyChargingTargets());

            // We've added a new schedule entry
            updatedChargingTargetSchedulesIdx++;

            // Let the updatedChargingTargets object of the schedule index
            updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);
        }

        // Now create the full Target data structure that we are going to save to persistent storage
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> updatedChargingTargetSchedulesList(
            updatedChargingTargetSchedulesArray, updatedChargingTargetSchedulesIdx);

        CHIP_ERROR err = SaveTargets(updatedChargingTargetSchedulesList);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SetTargets: Failed to save Target to persistent storage %s", chip::ErrorStr(err));
            return err;
        }

        // Now reload from persistent storage so that mChargingTargetSchedulesList gets the update Target
        err = LoadTargets();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SetTargets: Failed to load Target from persistent storage %s", chip::ErrorStr(err));
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
EvseTargetsDelegate::SaveTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedulesList)
{
    uint16_t total = GetTlvSizeUpperBound();

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    ReturnErrorCodeIf(!backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
    TLV::ScopedBufferTLVWriter writer(std::move(backingBuffer), total);

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));
    for (auto & chargingTargetSchedule : chargingTargetSchedulesList)
    {
        ChipLogProgress(
            AppServer, "SaveTargets: DayOfWeekForSequence = 0x%02x",
            chargingTargetSchedule.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask)));

        TLV::TLVType evseTargetEntryType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, evseTargetEntryType));
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(TargetEntryTag::kDayOfWeek), chargingTargetSchedule.dayOfWeekForSequence));

        TLV::TLVType chargingTargetsListType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(TargetEntryTag::kChargingTargetsList), TLV::kTLVType_List,
                                                   chargingTargetsListType));
        for (auto & chargingTarget : chargingTargetSchedule.chargingTargets)
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
        ReturnErrorOnFailure(writer.EndContainer(evseTargetEntryType));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    uint64_t len = static_cast<uint64_t>(writer.GetLengthWritten());
    ChipLogProgress(AppServer, "SaveTargets: length written 0x" ChipLogFormatX64, ChipLogValueX64(len));

    writer.Finalize(backingBuffer);

    ReturnErrorOnFailure(mpTargetStore->SyncSetKeyValue(spEvseTargetsKeyName, backingBuffer.Get(), static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EvseTargetsDelegate::ClearTargets()
{
    /* We simply delete the data from the persistent store */
    mpTargetStore->SyncDeleteKeyValue(spEvseTargetsKeyName);

    // Now reload from persistent storage so that mChargingTargetSchedulesList gets updated (it will be empty)
    CHIP_ERROR err = LoadTargets();

    return err;
}

void EvseTargetsDelegate::PrintTargets(
    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules)
{
    ChipLogProgress(AppServer, "---------------------- TARGETS ---------------------");

    uint16_t chargingTargetScheduleIdx = 0;
    for (auto & chargingTargetSchedule : chargingTargetSchedules)
    {
        [[maybe_unused]] uint8_t bitmask =
            chargingTargetSchedule.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask));
        ChipLogProgress(AppServer, "idx %u dayOfWeekForSequence 0x%02x", chargingTargetScheduleIdx, bitmask);

        uint16_t chargingTargetIdx = 0;
        for (auto & chargingTarget : chargingTargetSchedule.chargingTargets)
        {
            [[maybe_unused]] int64_t addedEnergy = chargingTarget.addedEnergy.HasValue() ? chargingTarget.addedEnergy.Value() : 0;

            ChipLogProgress(
                AppServer, "chargingTargetIdx %u targetTimeMinutesPastMidnight %u targetSoC %u addedEnergy 0x" ChipLogFormatX64,
                chargingTargetIdx, chargingTarget.targetTimeMinutesPastMidnight,
                chargingTarget.targetSoC.HasValue() ? chargingTarget.targetSoC.Value() : 0, ChipLogValueX64(addedEnergy));

            chargingTargetIdx++;
        }

        chargingTargetScheduleIdx++;
    }
}

/**
 * Part of the FabricTable::Delegate interface. Gets called when a fabric is deleted, such as on FabricTable::Delete().
 **/
void EvseTargetsDelegate::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) {}
