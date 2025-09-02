/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <CommodityTariffInstance.h>
#include <cstdint>
#include <unordered_set>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Attributes;
using namespace chip::app::Clusters::CommodityTariff::Structs;
using namespace chip::app::CommodityTariffAttrsDataMgmt;
using namespace CommodityTariffConsts;

using chip::Protocols::InteractionModel::Status;

using TariffInformationStructType = DataModel::Nullable<TariffInformationStruct::Type>;
using TariffPeriodStructType      = TariffPeriodStruct::Type;
using DayEntryStructType          = DayEntryStruct::Type;
using TariffComponentStructType   = TariffComponentStruct::Type;
using DayStructType               = DayStruct::Type;
using DayPatternStructType        = DayPatternStruct::Type;
using CalendarPeriodStructType    = CalendarPeriodStruct::Type;

CHIP_ERROR CommodityTariffInstance::Init()
{
    return Instance::Init();
}

uint32_t CommodityTariffInstance::GetCurrentTimestamp()
{
    return TimestampNow + TestTimeOverlay;
}

void CommodityTariffInstance::ScheduleTariffTimeUpdate()
{
    DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Milliseconds32(kTimerPollIntervalInSec * 1000),
        [](System::Layer *, void * context) { static_cast<CommodityTariffInstance *>(context)->TariffTimeUpdCb(); }, this);
}

void CommodityTariffInstance::TariffTimeUpdCb()
{
    GetDelegate()->TryToactivateDelayedTariff(TimestampNow);
    TimestampNow += kTimerPollIntervalInSec;
    TariffTimeAttrsSync();
    ScheduleTariffTimeUpdate();
}

void CommodityTariffInstance::ActivateTariffTimeTracking(uint32_t timestamp)
{
    TimestampNow = timestamp;

    ScheduleTariffTimeUpdate();
}

void CommodityTariffInstance::TariffTimeTrackingSetOffset(uint32_t offset)
{
    if (offset)
    {
        TestTimeOverlay += offset;
        return;
    }

    TestTimeOverlay = 0;
}

void CommodityTariffInstance::Shutdown()
{
    Instance::Shutdown();
}

bool CommodityTariffDelegate::TariffDataUpd_CrossValidator(TariffUpdateCtx & UpdCtx)
{
    bool DayEntriesData_is_available = false;

    if (!GetTariffInfo_MgmtObj().IsValid())
    {
        ChipLogError(NotSpecified, "TariffInfo not present!");
        return false;
    }
    else if (!GetDayEntries_MgmtObj().IsValid())
    {
        ChipLogError(NotSpecified, "DayEntries not present!");
        return false;
    }
    else if (!GetTariffComponents_MgmtObj().IsValid())
    {
        ChipLogError(NotSpecified, "TariffComponents not present!");
        return false;
    }
    else if (!GetTariffPeriods_MgmtObj().IsValid())
    {
        ChipLogError(NotSpecified, "TariffPeriods not present!");
        return false;
    }

    if (GetStartDate_MgmtObj().HasNewValue())
    {
        UpdCtx.TariffStartTimestamp = GetStartDate_MgmtObj().GetNewValue().Value();
    }

    assert(!UpdCtx.DayEntryKeyIDs.empty());
    assert(!UpdCtx.TariffComponentKeyIDsFeatureMap.empty());

    assert(!UpdCtx.TariffPeriodsDayEntryIDs.empty());        // Something went wrong if TariffPeriods has no DayEntries IDs
    assert(!UpdCtx.TariffPeriodsTariffComponentIDs.empty()); // Something went wrong if TariffPeriods has no TariffComponents IDs

    // Checks that all DayEntryIDs in Tariff Periods are in main DayEntries list:
    for (const auto & item : UpdCtx.TariffPeriodsDayEntryIDs)
    {
        if (!UpdCtx.DayEntryKeyIDs.count(item))
        {
            return false; // The item not found in original list
        }
    }

    // Checks that all TariffComponentIDs in Tariff Periods are in main TariffComponents list:
    for (const auto & item : UpdCtx.TariffPeriodsTariffComponentIDs)
    {
        if (UpdCtx.TariffComponentKeyIDsFeatureMap.find(item) == UpdCtx.TariffComponentKeyIDsFeatureMap.end())
        {
            return false; // The item not found in original list
        }
    }

    if (GetDayPatterns_MgmtObj().IsValid())
    {
        assert(!UpdCtx.DayPatternKeyIDs.empty());
        assert(!UpdCtx.DayPatternsDayEntryIDs.empty()); // Something went wrong if DP has no DE IDs

        // Checks that all DP_DEs are in main DE list:
        for (const auto & item : UpdCtx.DayPatternsDayEntryIDs)
        {
            if (!UpdCtx.DayEntryKeyIDs.count(item))
            {
                return false; // The item not found in original list
            }
        }
    }

    if (GetIndividualDays_MgmtObj().IsValid() && (GetIndividualDays_MgmtObj().HasNewValue()))
    {

        assert(!UpdCtx.IndividualDaysDayEntryIDs.empty()); // Something went wrong if IndividualDays has no DE IDs

        // Checks that all ID_DE_IDs are in main DE list:
        for (const auto & item : UpdCtx.IndividualDaysDayEntryIDs)
        {
            if (!UpdCtx.DayEntryKeyIDs.count(item))
            {
                return false; // The item not found in original list
            }

            if (UpdCtx.DayPatternsDayEntryIDs.count(item))
            {
                return false; // If same item from ID list has found in DP list
            }
        }

        DayEntriesData_is_available = true;
    }

    if (GetCalendarPeriods_MgmtObj().IsValid() && (GetCalendarPeriods_MgmtObj().HasNewValue()))
    {
        assert(!UpdCtx.CalendarPeriodsDayPatternIDs.empty()); // Something went wrong if CP has no DP IDs

        // Checks that all ID_DE_IDs are in main DE list:
        for (const auto & item : UpdCtx.CalendarPeriodsDayPatternIDs)
        {
            if (!UpdCtx.DayPatternKeyIDs.count(item))
            {
                return false; // The item not found in original list
            }
        }

        DayEntriesData_is_available = true;
    }

    if (!DayEntriesData_is_available)
    {
        ChipLogError(NotSpecified, "Both IndividualDays and CalendarPeriods are not present!");
        return false;
    }

    const auto & tariffPeriods    = GetTariffPeriods_MgmtObj().GetNewValue().Value();
    const auto & dayEntries       = GetDayEntries_MgmtObj().GetNewValue().Value();
    const auto & tariffComponents = GetTariffComponents_MgmtObj().GetNewValue().Value();

    // Create lookup maps with const correctness
    std::map<uint32_t, const Structs::DayEntryStruct::Type *> dayEntriesMap;
    std::map<uint32_t, const Structs::TariffComponentStruct::Type *> tariffComponentsMap;

    CommodityTariffAttrsDataMgmt::ListToMap<Structs::DayEntryStruct::Type, &Structs::DayEntryStruct::Type::dayEntryID>(
        dayEntries, dayEntriesMap);

    CommodityTariffAttrsDataMgmt::ListToMap<Structs::TariffComponentStruct::Type,
                                            &Structs::TariffComponentStruct::Type::tariffComponentID>(tariffComponents,
                                                                                                      tariffComponentsMap);

    struct DeStartDurationPair
    {
        uint16_t startTime;
        uint16_t duration;

        bool operator==(const DeStartDurationPair & other) const
        {
            return startTime == other.startTime && duration == other.duration;
        }
    };

    // Hash function for StartDurationPair
    struct DeStartDurationPairHash
    {
        size_t operator()(const DeStartDurationPair & p) const { return (static_cast<size_t>(p.startTime) << 16) | p.duration; }
    };

    for (const auto & period : tariffPeriods)
    {
        const auto & deIDs = period.dayEntryIDs;
        const auto & tcIDs = period.tariffComponentIDs;

        // Validate Day Entries
        std::unordered_set<DeStartDurationPair, DeStartDurationPairHash> seenStartDurationPairs;

        for (const uint32_t deID : deIDs)
        {
            // Check if DE exists in original context
            if (UpdCtx.DayEntryKeyIDs.count(deID) == 0)
            {
                return false; // Item not found in original list
            }

            // Safe lookup with bounds checking
            const auto dayEntryIt = dayEntriesMap.find(deID);
            if (dayEntryIt == dayEntriesMap.end())
            {
                return false; // Day entry not found in map
            }

            const auto * dayEntry = dayEntryIt->second;

            DeStartDurationPair pair;
            pair.startTime = dayEntry->startTime;
            pair.duration =
                dayEntry->duration.HasValue() ? dayEntry->duration.Value() : CommodityTariffConsts::kDayEntryDurationLimit;

            // Check for duplicates
            if (!seenStartDurationPairs.insert(pair).second)
            {
                ChipLogError(NotSpecified, "Duplicated startTime/duration (%u/%u) in  DayEntries of same TariffPeriod",
                             pair.startTime, pair.duration);
                return false; // Found duplicate startTime/duration combination
            }
        }

        // Validate Tariff Components
        std::unordered_map<uint32_t, std::unordered_set<int64_t>> seenFeatureThresholdPairs;

        for (const uint32_t tcID : tcIDs)
        {
            // Check if TC exists in original context
            const auto featureIt = UpdCtx.TariffComponentKeyIDsFeatureMap.find(tcID);
            if (featureIt == UpdCtx.TariffComponentKeyIDsFeatureMap.end())
            {
                return false; // Item not found in original list
            }
            // Safe lookup with bounds checking
            const auto tariffComponentIt = tariffComponentsMap.find(tcID);
            if (tariffComponentIt == tariffComponentsMap.end())
            {
                return false; // Tariff component not found in map
            }

            const auto * tariffComponent = tariffComponentIt->second;
            const uint32_t featureID     = featureIt->second;

            // Skip if threshold is null or featureID is 0
            if (tariffComponent->threshold.IsNull() ||
                (tariffComponent->predicted.HasValue() && (tariffComponent->predicted.Value() == true)) || featureID == 0)
            {
                continue;
            }

            const int64_t thresholdValue = tariffComponent->threshold.Value();

            // Find or create the set for this feature
            auto & thresholdSet = seenFeatureThresholdPairs[featureID];

            // Check for duplicate threshold for this feature
            if (!thresholdSet.insert(thresholdValue).second)
            {
                ChipLogError(NotSpecified, "Duplicated threshold value %" PRIi64 "per feature %" PRIu32 "of same TariffPeriod",
                             thresholdValue, featureID);
                return false; // Found duplicate feature/threshold combination
            }
        }
    }
    return true;
}

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

CommodityTariffDelegate::CommodityTariffDelegate() {}
