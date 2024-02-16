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

#pragma once

#include <app/clusters/energy-evse-server/energy-evse-server.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Pool.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class EvseTargetIteratorImpl;
class EvseTargetsDelegate
{
public:
    using EvseTargetIterator = CommonIterator<EvseTargetEntry>;

    ~EvseTargetsDelegate();

    CHIP_ERROR Init(PersistentStorageDelegate * targetStore);
    EvseTargetIteratorImpl * GetEvseTargetsIterator();
    CHIP_ERROR Load(std::vector<EvseTargetEntry> & targetEntryVector, size_t & targetsSize);
    CHIP_ERROR StoreEntry(const EvseTargetEntry & entry);

    CHIP_ERROR IncreaseEntryCount();
    CHIP_ERROR DecreaseEntryCount();
    CHIP_ERROR UpdateEntryCount(bool increase);
    CHIP_ERROR LoadCounter(size_t & count, size_t & targetsSize);
    CHIP_ERROR SerializeToTlv(TLV::TLVWriter & writer, const std::vector<EvseTargetEntry> & targetEntryVector);

    CHIP_ERROR CreateEntry(EvseTargetEntry &);
    CHIP_ERROR DeleteEntry(EvseTargetEntry &);
    CHIP_ERROR UpdateEntry(EvseTargetEntry &);

    /**
     * @brief   Copies a ChargingTargetSchedule into our mTargets
     *
     * @param [in] an entry from the SetTargets list containing:
     *             dayOfWeekForSequence and chargingTargets (list)
     *
     * This routine scans the existing mTargets to see if we have a day of week
     * set that matches the new target dayOfWeek bits. If there is an existing
     * matching day then it replaces the days existing targets with the new entry
     */
    CHIP_ERROR
    CopyTarget(const Structs::ChargingTargetScheduleStruct::DecodableType &);

    /**
     *  @brief   This deletes all targets and resets the list to empty
     */
    CHIP_ERROR ClearTargets();

    static constexpr size_t MaxTargetEntryCounterSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(size_t), sizeof(size_t));
    }

    static constexpr size_t MaxTargetEntrySize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(chip::BitMask<TargetDayOfWeekBitmap>)) +
            kEvseTargetsMaxNumberOfDays *
            TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(Optional<chip::Percent>), sizeof(Optional<int64_t>));
    }

protected:
    enum class TargetEntryTag : uint8_t
    {
        kTargetEntry           = 1,
        kDayOfWeek             = 2,
        kChargingTargetsList   = 3,
        kChargingTargetsStruct = 4,
        kTargetTime            = 5,
        kTargetSoC             = 6,
        kAddedEnergy           = 7,
    };

    enum class CounterTag : uint8_t
    {
        kCount = 1,
        kSize  = 2,
    };

private:
    // The array itself has a control byte and an end-of-array marker.
    static constexpr size_t kArrayOverHead = 2;

    EvseTargetIteratorImpl * mEvseTargetsIterator = nullptr;
    PersistentStorageDelegate * mpTargetStore     = nullptr;
};

using EvseTargetIterator = CommonIterator<EvseTargetEntry>;
class EvseTargetIteratorImpl : public EvseTargetIterator
{
public:
    EvseTargetIteratorImpl(EvseTargetsDelegate & aDelegate) : mDelegate(aDelegate)
    {
        mTargetEntryIndex = 0;
        mTargetEntryVector.clear();
    }
    size_t Count() override;
    bool Next(EvseTargetEntry & entry) override;
    void Release() override;
    CHIP_ERROR Load();

private:
    EvseTargetsDelegate & mDelegate;
    size_t mTargetEntryIndex = 0;
    std::vector<EvseTargetEntry> mTargetEntryVector;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
