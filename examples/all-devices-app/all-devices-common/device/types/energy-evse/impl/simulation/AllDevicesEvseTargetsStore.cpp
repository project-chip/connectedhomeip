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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/energy-evse-server/Constants.h>
#include <app/server/Server.h>
#include <device/types/energy-evse/impl/simulation/AllDevicesEvseTargetsStore.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using chip::Protocols::InteractionModel::Status;

AllDevicesEvseTargetsDelegate::AllDevicesEvseTargetsDelegate() {}

AllDevicesEvseTargetsDelegate::~AllDevicesEvseTargetsDelegate() {}

CHIP_ERROR AllDevicesEvseTargetsDelegate::Init(PersistentStorageDelegate * targetStore)
{
    ChipLogProgress(AppServer, "EVSE: Initializing AllDevicesEvseTargetsDelegate");
    VerifyOrReturnError(targetStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mpTargetStore = targetStore;

    // Set FabricDelegate
    TEMPORARY_RETURN_IGNORED chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(this);
    mFabricDelegateRegistered = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR AllDevicesEvseTargetsDelegate::InitTargetsStorage(PersistentStorageDelegate * targetStore)
{
    VerifyOrReturnError(targetStore != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mpTargetStore = targetStore;
    return CHIP_NO_ERROR;
}

void AllDevicesEvseTargetsDelegate::Shutdown()
{
    if (mFabricDelegateRegistered)
    {
        chip::Server::GetInstance().GetFabricTable().RemoveFabricDelegate(this);
        mFabricDelegateRegistered = false;
    }
    mpTargetStore = nullptr;
}

const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & AllDevicesEvseTargetsDelegate::GetTargets()
{
    return mChargingTargetSchedulesList;
}

/* static */
uint16_t AllDevicesEvseTargetsDelegate::GetTlvSizeUpperBound()
{
    size_t kListOverhead = 4;
    size_t chargingTargetStuctEstimate =
        TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(Optional<chip::Percent>), sizeof(Optional<int64_t>));
    size_t chargingTargetScheduleStructEstimate = TLV::EstimateStructOverhead(sizeof(chip::BitMask<TargetDayOfWeekBitmap>)) +
        kListOverhead + kEvseTargetsMaxTargetsPerDay * chargingTargetStuctEstimate;
    size_t totalEstimate = kEvseTargetsMaxNumberOfDays * chargingTargetScheduleStructEstimate + kListOverhead;

    return static_cast<uint16_t>(totalEstimate);
}

CHIP_ERROR AllDevicesEvseTargetsDelegate::LoadTargets()
{
    // The DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> data structure contains a list of
    // ChargingTargetScheduleStructs which in turn contains a list of ChargingTargetStructs. Lists contain pointers
    // to objects allocated outside of the List. For mChargingTargetSchedulesList, that memory is allocated in
    // mChargingTargets and mChargingTargetSchedulesArray.

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    uint16_t length = GetTlvSizeUpperBound();
    VerifyOrReturnError(backingBuffer.Calloc(length), CHIP_ERROR_NO_MEMORY);

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
        mChargingTargets.BeginDayScheduleBuild(chargingTargetSchedulesIdx);

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
        err = mChargingTargets.CommitBuiltDaySchedule();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SetTargets: Failed to allocate memory during LoadTargets: %" CHIP_ERROR_FORMAT, err.Format());
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

CHIP_ERROR AllDevicesEvseTargetsDelegate::ReplaceTargetsInMemory(
    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedulesList)
{
    uint16_t chargingTargetSchedulesIdx = 0;
    for (auto & chargingTargetSchedule : chargingTargetSchedulesList)
    {
        VerifyOrReturnError(chargingTargetSchedulesIdx < kEvseTargetsMaxNumberOfDays, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(mChargingTargets.ReplaceDayScheduleTargets(chargingTargetSchedulesIdx,
                                                                          chargingTargetSchedule.chargingTargets));

        mChargingTargetSchedulesArray[chargingTargetSchedulesIdx].dayOfWeekForSequence =
            chargingTargetSchedule.dayOfWeekForSequence;

        mChargingTargetSchedulesArray[chargingTargetSchedulesIdx].chargingTargets =
            chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
                mChargingTargets.GetChargingTargets(), mChargingTargets.GetNumDailyChargingTargets());

        chargingTargetSchedulesIdx++;
    }

    mChargingTargetSchedulesList = DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(mChargingTargetSchedulesArray,
                                                                                                      chargingTargetSchedulesIdx);
    return CHIP_NO_ERROR;
}

void AllDevicesEvseTargetsDelegate::RestorePersistedTargetsIfLiveListWasModified(bool liveListWasModified)
{
    if (!liveListWasModified)
    {
        return;
    }

    CHIP_ERROR err = LoadTargets();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetTargets: Failed to restore targets from storage: %" CHIP_ERROR_FORMAT, err.Format());
    }
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
CHIP_ERROR AllDevicesEvseTargetsDelegate::SetTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & newChargingTargetSchedules)
{
    ChipLogProgress(AppServer, "SetTargets");

    bool liveListWasModified = false;

    // We'll need to have a local copy of the chargingTargets that are referenced from updatedChargingTargetSchedules (which
    // is a List<ChargingTargetScheduleStruct> where each ChargingTargetScheduleStruct has a List of ChargingTargetStructs).
    // Note updatedChargingTargets only needs to exist for the duration of this method. Merged targets are staged in memory
    // via ReplaceTargetsInMemory; persistent storage is updated once after all command entries are merged successfully.
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
        uint8_t coveredBitmask                     = 0;
        uint16_t updatedChargingTargetSchedulesIdx = 0;

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
                updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);
                CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(newChargingTargetSchedule.chargingTargets);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets: %" CHIP_ERROR_FORMAT,
                                 err.Format());
                    RestorePersistedTargetsIfLiveListWasModified(liveListWasModified);
                    return err;
                }

                coveredBitmask |= bitmaskA;
            }
            else
            {
                // This entry stays - but it has lost some days from the bitmask
                updatedBitmask = BitMask<TargetDayOfWeekBitmap>(bitmaskB);

                // Copy the existing chargingTargets
                updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);
                CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(currentChargingTargetSchedule.chargingTargets);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets: %" CHIP_ERROR_FORMAT,
                                 err.Format());
                    RestorePersistedTargetsIfLiveListWasModified(liveListWasModified);
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
        }

        // Add a new entry for the days in the new schedule that no existing entry covers
        uint8_t remainingBitmask = static_cast<uint8_t>(newBitmask & ~coveredBitmask);
        if (remainingBitmask != 0)
        {
            if (updatedChargingTargetSchedulesIdx >= kEvseTargetsMaxNumberOfDays)
            {
                RestorePersistedTargetsIfLiveListWasModified(liveListWasModified);
                return CHIP_ERROR_NO_MEMORY;
            }

            // Copy the new chargingTargets
            updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);
            CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(newChargingTargetSchedule.chargingTargets);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets: %" CHIP_ERROR_FORMAT, err.Format());
                RestorePersistedTargetsIfLiveListWasModified(liveListWasModified);
                return err;
            }

            // Update the new schedule with the dayOfWeekForSequence and list of chargingTargets
            updatedChargingTargetSchedulesArray[updatedChargingTargetSchedulesIdx].dayOfWeekForSequence =
                BitMask<TargetDayOfWeekBitmap>(remainingBitmask);

            updatedChargingTargetSchedulesArray[updatedChargingTargetSchedulesIdx].chargingTargets =
                chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
                    updatedChargingTargets.GetChargingTargets(), updatedChargingTargets.GetNumDailyChargingTargets());

            // We've added a new schedule entry
            updatedChargingTargetSchedulesIdx++;
        }

        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> updatedChargingTargetSchedulesList(
            updatedChargingTargetSchedulesArray, updatedChargingTargetSchedulesIdx);

        CHIP_ERROR replaceErr = ReplaceTargetsInMemory(updatedChargingTargetSchedulesList);
        if (replaceErr != CHIP_NO_ERROR)
        {
            RestorePersistedTargetsIfLiveListWasModified(true);
            return replaceErr;
        }
        liveListWasModified = true;
    }

    CHIP_ERROR saveErr = SaveTargets(mChargingTargetSchedulesList);
    if (saveErr != CHIP_NO_ERROR)
    {
        RestorePersistedTargetsIfLiveListWasModified(liveListWasModified);
        return saveErr;
    }

    CHIP_ERROR loadErr = LoadTargets();
    if (loadErr != CHIP_NO_ERROR)
    {
        RestorePersistedTargetsIfLiveListWasModified(liveListWasModified);
        return loadErr;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
AllDevicesEvseTargetsDelegate::SaveTargets(
    DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedulesList)
{
    uint16_t total = GetTlvSizeUpperBound();

    Platform::ScopedMemoryBuffer<uint8_t> backingBuffer;
    VerifyOrReturnError(backingBuffer.Calloc(total), CHIP_ERROR_NO_MEMORY);
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

    TEMPORARY_RETURN_IGNORED writer.Finalize(backingBuffer);

    ReturnErrorOnFailure(mpTargetStore->SyncSetKeyValue(spEvseTargetsKeyName, backingBuffer.Get(), static_cast<uint16_t>(len)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AllDevicesEvseTargetsDelegate::ClearTargets()
{
    CHIP_ERROR deleteErr = mpTargetStore->SyncDeleteKeyValue(spEvseTargetsKeyName);
    if (deleteErr != CHIP_NO_ERROR && deleteErr != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return deleteErr;
    }

    // Reload from persistent storage so that mChargingTargetSchedulesList gets updated (it will be empty)
    return LoadTargets();
}

void AllDevicesEvseTargetsDelegate::PrintTargets(
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
void AllDevicesEvseTargetsDelegate::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) {}
