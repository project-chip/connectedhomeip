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
    return LoadTargets();
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

CHIP_ERROR AllDevicesEvseTargetsDelegate::ParseTargetsFromReader(TLV::TLVReader & reader, ChargingTargetsMemMgr & memMgr,
                                                                 Structs::ChargingTargetScheduleStruct::Type * schedulesArray,
                                                                 uint16_t & outScheduleCount)
{
    outScheduleCount = 0;

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        TLV::TLVType evseTargetEntryType;

        ReturnErrorOnFailure(reader.EnterContainer(evseTargetEntryType));

        VerifyOrReturnError(outScheduleCount < kEvseTargetsMaxNumberOfDays, CHIP_ERROR_INCORRECT_STATE);

        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(TargetEntryTag::kDayOfWeek)));
        ReturnErrorOnFailure(reader.Get(schedulesArray[outScheduleCount].dayOfWeekForSequence));

        ChipLogProgress(AppServer, "LoadTargets: DayOfWeekForSequence = 0x%02x",
                        schedulesArray[outScheduleCount].dayOfWeekForSequence.GetField(
                            static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask)));

        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_List, TLV::ContextTag(TargetEntryTag::kChargingTargetsList)));
        TLV::TLVType chargingTargetsListType;
        ReturnErrorOnFailure(reader.EnterContainer(chargingTargetsListType));

        memMgr.BeginDayScheduleBuild(outScheduleCount);

        while ((err = reader.Next(TLV::kTLVType_Structure, TLV::ContextTag(TargetEntryTag::kChargingTargetsStruct))) ==
               CHIP_NO_ERROR)
        {
            TLV::TLVType chargingTargetsStructType = TLV::kTLVType_Structure;
            ReturnErrorOnFailure(reader.EnterContainer(chargingTargetsStructType));

            EnergyEvse::Structs::ChargingTargetStruct::Type chargingTarget;

            while ((err = reader.Next()) == CHIP_NO_ERROR)
            {
                auto type = reader.GetType();
                auto tag  = reader.GetTag();
                if (type == TLV::kTLVType_NotSpecified)
                {
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
                    return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsStructType));

            ChipLogProgress(AppServer,
                            "LoadingTargets: targetTimeMinutesPastMidnight %u targetSoC %u addedEnergy 0x" ChipLogFormatX64,
                            chargingTarget.targetTimeMinutesPastMidnight, chargingTarget.targetSoC.ValueOr(0),
                            ChipLogValueX64(chargingTarget.addedEnergy.ValueOr(0)));

            memMgr.AddChargingTarget(chargingTarget);
        }

        ReturnErrorOnFailure(reader.ExitContainer(chargingTargetsListType));
        ReturnErrorOnFailure(reader.ExitContainer(evseTargetEntryType));

        ReturnErrorOnFailure(memMgr.CommitBuiltDaySchedule());

        schedulesArray[outScheduleCount].chargingTargets =
            chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(memMgr.GetChargingTargets(),
                                                                                        memMgr.GetNumDailyChargingTargets());

        outScheduleCount++;
    }

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AllDevicesEvseTargetsDelegate::CommitStagedTargets(const Structs::ChargingTargetScheduleStruct::Type * stagingArray,
                                                              uint16_t stagedCount)
{
    VerifyOrReturnError(stagedCount <= kEvseTargetsMaxNumberOfDays, CHIP_ERROR_INCORRECT_STATE);

    const uint16_t previousCount = static_cast<uint16_t>(mChargingTargetSchedulesList.size());
    const DataModel::List<const Structs::ChargingTargetStruct::Type> emptyChargingTargets;

    ChargingTargetsMemMgr builtMemMgr;
    Structs::ChargingTargetScheduleStruct::Type builtArray[kEvseTargetsMaxNumberOfDays];

    for (uint16_t i = 0; i < stagedCount; i++)
    {
        VerifyOrReturnError(stagingArray != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        builtArray[i].dayOfWeekForSequence = stagingArray[i].dayOfWeekForSequence;
        ReturnErrorOnFailure(builtMemMgr.ReplaceDayScheduleTargets(i, stagingArray[i].chargingTargets));
        builtArray[i].chargingTargets =
            chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
                builtMemMgr.GetChargingTargets(), builtMemMgr.GetNumDailyChargingTargets());
    }

    for (uint16_t i = 0; i < stagedCount; i++)
    {
        mChargingTargetSchedulesArray[i].dayOfWeekForSequence = builtArray[i].dayOfWeekForSequence;
        mChargingTargets.AdoptDayScheduleBlock(i, builtMemMgr);
        mChargingTargetSchedulesArray[i].chargingTargets = builtArray[i].chargingTargets;
    }

    for (uint16_t i = stagedCount; i < previousCount; i++)
    {
        mChargingTargets.FreeDayScheduleSlot(i);
        mChargingTargetSchedulesArray[i].dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(0);
        mChargingTargetSchedulesArray[i].chargingTargets      = emptyChargingTargets;
    }

    mChargingTargetSchedulesList =
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(mChargingTargetSchedulesArray, stagedCount);

    return CHIP_NO_ERROR;
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
        return CommitStagedTargets(nullptr, 0);
    }
    ReturnErrorOnFailure(err);

    TLV::ScopedBufferTLVReader reader(std::move(backingBuffer), length);

    ChargingTargetsMemMgr stagingMemMgr;
    Structs::ChargingTargetScheduleStruct::Type stagingArray[kEvseTargetsMaxNumberOfDays];
    uint16_t stagedCount = 0;

    ReturnErrorOnFailure(ParseTargetsFromReader(reader, stagingMemMgr, stagingArray, stagedCount));
    ReturnErrorOnFailure(reader.VerifyEndOfContainer());

    return CommitStagedTargets(stagingArray, stagedCount);
}

CHIP_ERROR AllDevicesEvseTargetsDelegate::ReplaceTargetsInMemory(
    const DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedulesList)
{
    uint16_t chargingTargetSchedulesIdx = 0;
    for (auto & chargingTargetSchedule : chargingTargetSchedulesList)
    {
        VerifyOrReturnError(chargingTargetSchedulesIdx < kEvseTargetsMaxNumberOfDays, CHIP_ERROR_NO_MEMORY);

        ReturnErrorOnFailure(
            mChargingTargets.ReplaceDayScheduleTargets(chargingTargetSchedulesIdx, chargingTargetSchedule.chargingTargets));

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

CHIP_ERROR AllDevicesEvseTargetsDelegate::SnapshotLiveTargets(LiveTargetsRollbackSnapshot & snapshot)
{
    snapshot.count = static_cast<uint16_t>(mChargingTargetSchedulesList.size());
    VerifyOrReturnError(snapshot.count <= kEvseTargetsMaxNumberOfDays, CHIP_ERROR_INCORRECT_STATE);

    for (uint16_t i = 0; i < snapshot.count; i++)
    {
        snapshot.array[i].dayOfWeekForSequence = mChargingTargetSchedulesArray[i].dayOfWeekForSequence;
        ReturnErrorOnFailure(
            snapshot.memMgr.ReplaceDayScheduleTargets(i, mChargingTargetSchedulesArray[i].chargingTargets));
        snapshot.array[i].chargingTargets = chip::app::DataModel::List<EnergyEvse::Structs::ChargingTargetStruct::Type>(
            snapshot.memMgr.GetChargingTargets(), snapshot.memMgr.GetNumDailyChargingTargets());
    }

    return CHIP_NO_ERROR;
}

void AllDevicesEvseTargetsDelegate::RestorePersistedTargetsIfLiveListWasModified(
    bool liveListWasModified, const LiveTargetsRollbackSnapshot & rollbackSnapshot)
{
    if (!liveListWasModified)
    {
        return;
    }

    CHIP_ERROR err = LoadTargets();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetTargets: Failed to restore targets from storage: %" CHIP_ERROR_FORMAT, err.Format());
        CHIP_ERROR restoreErr = CommitStagedTargets(rollbackSnapshot.array, rollbackSnapshot.count);
        if (restoreErr != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "SetTargets: Failed to restore targets from pre-merge snapshot: %" CHIP_ERROR_FORMAT,
                         restoreErr.Format());
        }
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

    {
        auto validateIter = newChargingTargetSchedules.begin();
        while (validateIter.Next())
        {
            const uint8_t bitmask = validateIter.GetValue().dayOfWeekForSequence.GetField(
                static_cast<TargetDayOfWeekBitmap>(kAllTargetDaysMask));
            VerifyOrReturnError(bitmask != 0, CHIP_ERROR_INVALID_ARGUMENT);
        }
        ReturnErrorOnFailure(validateIter.GetStatus());
    }

    LiveTargetsRollbackSnapshot rollbackSnapshot;
    ReturnErrorOnFailure(SnapshotLiveTargets(rollbackSnapshot));

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
                    RestorePersistedTargetsIfLiveListWasModified(liveListWasModified, rollbackSnapshot);
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
                    RestorePersistedTargetsIfLiveListWasModified(liveListWasModified, rollbackSnapshot);
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
                RestorePersistedTargetsIfLiveListWasModified(liveListWasModified, rollbackSnapshot);
                return CHIP_ERROR_NO_MEMORY;
            }

            // Copy the new chargingTargets
            updatedChargingTargets.PrepareDaySchedule(updatedChargingTargetSchedulesIdx);
            CHIP_ERROR err = updatedChargingTargets.AllocAndCopy(newChargingTargetSchedule.chargingTargets);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "SetTargets: Failed to copy the new chargingTargets: %" CHIP_ERROR_FORMAT, err.Format());
                RestorePersistedTargetsIfLiveListWasModified(liveListWasModified, rollbackSnapshot);
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
            RestorePersistedTargetsIfLiveListWasModified(true, rollbackSnapshot);
            return replaceErr;
        }
        liveListWasModified = true;
    }

    const uint16_t newScheduleCount = static_cast<uint16_t>(mChargingTargetSchedulesList.size());
    const DataModel::List<const Structs::ChargingTargetStruct::Type> emptyChargingTargets;
    for (uint16_t i = newScheduleCount; i < rollbackSnapshot.count; i++)
    {
        CHIP_ERROR trimErr = mChargingTargets.ReplaceDayScheduleTargets(i, emptyChargingTargets);
        if (trimErr != CHIP_NO_ERROR)
        {
            RestorePersistedTargetsIfLiveListWasModified(liveListWasModified, rollbackSnapshot);
            return trimErr;
        }
        mChargingTargetSchedulesArray[i].dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(0);
        mChargingTargetSchedulesArray[i].chargingTargets      = emptyChargingTargets;
    }

    CHIP_ERROR saveErr = SaveTargets(mChargingTargetSchedulesList);
    if (saveErr != CHIP_NO_ERROR)
    {
        RestorePersistedTargetsIfLiveListWasModified(liveListWasModified, rollbackSnapshot);
        return saveErr;
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
