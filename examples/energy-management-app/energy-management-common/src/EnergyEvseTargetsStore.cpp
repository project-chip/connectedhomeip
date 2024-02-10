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

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;

CHIP_ERROR EvseTargetsStore::LoadTargetsFromStore()
{
    uint8_t buf[kEnergyEvseTargetsTLVMaxSize];
    uint16_t size = sizeof(buf);

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::EVSETargets().KeyName(), buf, size));
    TLV::ContiguousBufferTLVReader reader;
    reader.Init(buf, size);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    // ReturnErrorOnFailure(reader.Next(kLastKnownGoodChipEpochSecondsTag));
    // ReturnErrorOnFailure(reader.Get(seconds));

    return CHIP_NO_ERROR;
}
/**
 * This routine tries to compress a list of entries which has:
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
CHIP_ERROR EvseTargets::CopyTarget(const Structs::ChargingTargetScheduleStruct::DecodableType & newEntry)
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
    //     }
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

CHIP_ERROR EvseTargets::ClearTargets()
{
    return CHIP_NO_ERROR;
}