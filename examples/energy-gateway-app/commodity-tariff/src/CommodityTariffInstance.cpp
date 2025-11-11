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
#include <cinttypes>

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
using namespace chip::System;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

using chip::Protocols::InteractionModel::Status;

using TariffInformationStructType = DataModel::Nullable<TariffInformationStruct::Type>;
using TariffPeriodStructType      = TariffPeriodStruct::Type;
using DayEntryStructType          = DayEntryStruct::Type;
using TariffComponentStructType   = TariffComponentStruct::Type;
using DayStructType               = DayStruct::Type;
using DayPatternStructType        = DayPatternStruct::Type;
using CalendarPeriodStructType    = CalendarPeriodStruct::Type;

// ============================================================================
// Existing Tariff Implementation
// ============================================================================

void CommodityTariffInstance::ScheduleTariffTimeUpdate()
{
    constexpr auto pollInterval = kTimerPollIntervalInSec * 1_s;

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(
        pollInterval, [](System::Layer *, void * context) { static_cast<CommodityTariffInstance *>(context)->TariffTimeUpdCb(); },
        this);
}

void CommodityTariffInstance::CancelTariffTimeUpdate()
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().CancelTimer(
        [](System::Layer *, void * context) { static_cast<CommodityTariffInstance *>(context)->TariffTimeUpdCb(); }, this);
}

void CommodityTariffInstance::TariffTimeUpdCb()
{
    uint32_t currentTimestamp = 0;

    if (CHIP_NO_ERROR == GetClock_MatterEpochS(currentTimestamp))
    {
        GetDelegate()->TryToactivateDelayedTariff(currentTimestamp);
    }

    TariffTimeAttrsSync();
    ScheduleTariffTimeUpdate();
}

CHIP_ERROR CommodityTariffInstance::Init()
{
    ScheduleTariffTimeUpdate();
    return Instance::Init();
}

void CommodityTariffInstance::Shutdown()
{
    CancelTariffTimeUpdate();
    Instance::Shutdown();
}

void CommodityTariffDelegate::TariffDataUpdate(uint32_t aNowTimestamp)
{

    TariffUpdateCtx UpdCtx = {
        .blockMode = static_cast<BlockModeEnum>(0),
        .TariffStartTimestamp =
            static_cast<StartDateDataClass &>(GetMgmtObj(CommodityTariffAttrTypeEnum::kStartDate)).GetNewValue(),
        .mFeature              = mFeature,
        .TariffUpdateTimestamp = aNowTimestamp
    };
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (CHIP_NO_ERROR != (err = TariffDataUpd_Init(UpdCtx)))
    {
        ChipLogError(AppServer, "EGW-CTC: New tariff data rejected due to internal inconsistencies");
    }
    else if (CHIP_NO_ERROR != (err = TariffDataUpd_CrossValidator(UpdCtx)))
    {
        ChipLogError(AppServer, "EGW-CTC: New tariff data rejected due to some cross-fields inconsistencies");
    }
    else
    {
        if (!UpdCtx.TariffStartTimestamp.IsNull() && (UpdCtx.TariffStartTimestamp.Value() > UpdCtx.TariffUpdateTimestamp))
        {
            DelayedTariffUpdateIsActive = true;
            return;
        }
    }
    TariffDataUpd_Finish(err == CHIP_NO_ERROR);
}

CHIP_ERROR CommodityTariffDelegate::TariffDataUpd_Init(TariffUpdateCtx & UpdCtx)
{
    for (uint8_t iter = 0; iter < CommodityTariffAttrTypeEnum::kAttrMax; iter++)
    {
        CommodityTariffAttrTypeEnum attr = static_cast<CommodityTariffAttrTypeEnum>(iter);
        auto & mgmtObj                   = GetMgmtObj(attr);
        CHIP_ERROR err                   = mgmtObj.UpdateBegin(&UpdCtx);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "EGW-CTC: UpdateBegin failed for attribute %d: %" CHIP_ERROR_FORMAT, iter, err.Format());
            return err;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommodityTariffDelegate::TariffDataUpd_CrossValidator(TariffUpdateCtx & UpdCtx)
{
    bool DayEntriesData_is_available = false;

    if (!GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffInfo).IsValid())
    {
        ChipLogError(AppServer, "TariffInfo management object is not present or invalid");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (!GetMgmtObj(CommodityTariffAttrTypeEnum::kDayEntries).IsValid())
    {
        ChipLogError(AppServer, "DayEntries management object is not present or invalid");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (!GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffComponents).IsValid())
    {
        ChipLogError(AppServer, "TariffComponents management object is not present or invalid");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (!GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffPeriods).IsValid())
    {
        ChipLogError(AppServer, "TariffPeriods management object is not present or invalid");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (GetMgmtObj(CommodityTariffAttrTypeEnum::kStartDate).HasNewValue())
    {
        UpdCtx.TariffStartTimestamp =
            static_cast<StartDateDataClass &>(GetMgmtObj(CommodityTariffAttrTypeEnum::kStartDate)).GetNewValue().Value();
    }

    // Checks that all DayEntryIDs in Tariff Periods are in main DayEntries list:
    for (const auto & item : UpdCtx.TariffPeriodsDayEntryIDs)
    {
        if (!UpdCtx.DayEntryKeyIDs.count(item))
        {
            ChipLogError(AppServer, "DayEntry ID%" PRIu32 "referenced from TariffPeriods doesn't exist in main DayEntries list",
                         item);
            return CHIP_ERROR_KEY_NOT_FOUND; // The item not found in original list
        }
    }

    // Checks that all TariffComponentIDs in Tariff Periods are in main TariffComponents list:
    for (const auto & item : UpdCtx.TariffPeriodsTariffComponentIDs)
    {
        if (UpdCtx.TariffComponentKeyIDsFeatureMap.find(item) == UpdCtx.TariffComponentKeyIDsFeatureMap.end())
        {
            ChipLogError(AppServer,
                         "TariffComponent ID%" PRIu32 "referenced from TariffPeriods doesn't exist in main TariffComponents list",
                         item);
            return CHIP_ERROR_KEY_NOT_FOUND; // The item not found in original list
        }
    }

    if (GetMgmtObj(CommodityTariffAttrTypeEnum::kDayPatterns).IsValid())
    {
        // Checks that all DP_DEs are in main DE list:
        for (const auto & item : UpdCtx.DayPatternsDayEntryIDs)
        {
            if (!UpdCtx.DayEntryKeyIDs.count(item))
            {
                ChipLogError(AppServer, "DayEntry ID%" PRIu32 "referenced from DayPatterns doesn't exist in main DayEntries list",
                             item);
                return CHIP_ERROR_KEY_NOT_FOUND; // The item not found in original list
            }
        }
    }

    if (GetMgmtObj(CommodityTariffAttrTypeEnum::kIndividualDays).IsValid() &&
        (GetMgmtObj(CommodityTariffAttrTypeEnum::kIndividualDays).HasNewValue()))
    {
        // Checks that all ID_DE_IDs are in main DE list:
        for (const auto & item : UpdCtx.IndividualDaysDayEntryIDs)
        {
            if (!UpdCtx.DayEntryKeyIDs.count(item))
            {
                ChipLogError(AppServer,
                             "DayEntry ID%" PRIu32 "referenced from IndividualDays doesn't exist in main DayEntries list", item);
                return CHIP_ERROR_KEY_NOT_FOUND; // The item not found in original list
            }

            if (UpdCtx.DayPatternsDayEntryIDs.count(item))
            {
                ChipLogError(AppServer, "DayEntry ID%" PRIu32 "is duplicated - found in both IndividualDays and DayPatterns lists",
                             item);
                return CHIP_ERROR_DUPLICATE_KEY_ID; // If same item from ID list has found in DP list
            }
        }

        DayEntriesData_is_available = true;
    }

    if (GetMgmtObj(CommodityTariffAttrTypeEnum::kCalendarPeriods).IsValid() &&
        (GetMgmtObj(CommodityTariffAttrTypeEnum::kCalendarPeriods).HasNewValue()))
    {
        // Checks that all DayPatternIDs are in main DayPattern list:
        for (const auto & item : UpdCtx.CalendarPeriodsDayPatternIDs)
        {
            if (!UpdCtx.DayPatternKeyIDs.count(item))
            {
                ChipLogError(AppServer,
                             "DayPattern ID%" PRIu32 "referenced from CalendarPeriods doesn't exist in main DayPatterns list",
                             item);
                return CHIP_ERROR_KEY_NOT_FOUND; // The item not found in original list
            }
        }

        DayEntriesData_is_available = true;
    }

    if (!DayEntriesData_is_available)
    {
        ChipLogError(AppServer, "Both IndividualDays and CalendarPeriods are not present or have no valid data");
        return CHIP_ERROR_INVALID_DATA_LIST;
    }

    const auto & tariffPeriods =
        static_cast<TariffPeriodsDataClass &>(GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffPeriods)).GetNewValue().Value();
    const auto & dayEntries =
        static_cast<DayEntriesDataClass &>(GetMgmtObj(CommodityTariffAttrTypeEnum::kDayEntries)).GetNewValue().Value();
    const auto & tariffComponents =
        static_cast<TariffComponentsDataClass &>(GetMgmtObj(CommodityTariffAttrTypeEnum::kTariffComponents)).GetNewValue().Value();

    // Create lookup maps with const correctness
    std::unordered_map<uint32_t, const Structs::DayEntryStruct::Type *> dayEntriesMap;
    std::unordered_map<uint32_t, const Structs::TariffComponentStruct::Type *> tariffComponentsMap;

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
                ChipLogError(AppServer, "DayEntry ID%" PRIu32 "doesn't exist in validation context", deID);
                return CHIP_ERROR_KEY_NOT_FOUND; // Item not found in original list
            }

            const auto dayEntryIt = dayEntriesMap.find(deID);
            if (dayEntryIt == dayEntriesMap.end())
            {
                ChipLogError(AppServer, "Unable to find DayEntry with ID%" PRIu32 "in the parsed data map", deID);
                return CHIP_ERROR_KEY_NOT_FOUND; // Day entry not found in map
            }

            const auto * dayEntry = dayEntryIt->second;

            DeStartDurationPair pair;
            pair.startTime = dayEntry->startTime;
            pair.duration =
                dayEntry->duration.HasValue() ? dayEntry->duration.Value() : CommodityTariffConsts::kDayEntryDurationLimit;

            // Check for duplicates
            if (!seenStartDurationPairs.insert(pair).second)
            {
                ChipLogError(AppServer,
                             "Duplicate startTime/duration combination (%u/%u) found in DayEntries of the same TariffPeriod",
                             pair.startTime, pair.duration);
                return CHIP_ERROR_DUPLICATE_KEY_ID; // Found duplicate startTime/duration combination
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
                ChipLogError(AppServer, "TariffComponent ID%" PRIu32 "not found in validation context feature map", tcID);
                return CHIP_ERROR_KEY_NOT_FOUND; // Item not found in original list
            }
            const auto tariffComponentIt = tariffComponentsMap.find(tcID);
            if (tariffComponentIt == tariffComponentsMap.end())
            {
                ChipLogError(AppServer, "Unable to find TariffComponent with ID%" PRIu32 "in the parsed data map", tcID);
                return CHIP_ERROR_KEY_NOT_FOUND; // Tariff component not found in map
            }

            const auto * tariffComponent = tariffComponentIt->second;
            const uint32_t featureID     = featureIt->second;

            // Skip if threshold is null or featureID is 0
            if (tariffComponent->threshold.IsNull() || tariffComponent->predicted.ValueOr(false) || featureID == 0)
            {
                continue;
            }

            const int64_t thresholdValue = tariffComponent->threshold.Value();

            // Find or create the set for this feature
            auto & thresholdSet = seenFeatureThresholdPairs[featureID];

            // Check for duplicate threshold for this feature
            if (!thresholdSet.insert(thresholdValue).second)
            {
                ChipLogError(NotSpecified,
                             "Duplicated threshold value among TCs for the 0x%" PRIx32 " feature in the same tariff period",
                             featureID);
                return CHIP_ERROR_DUPLICATE_KEY_ID; // Found duplicate feature/threshold combination
            }
        }
    }

    return CHIP_NO_ERROR;
}

void CommodityTariffDelegate::TariffDataUpd_Finish(bool is_success)
{
    AttributeId updatedAttrIds[CommodityTariffAttrTypeEnum::kAttrMax];
    size_t updatedCount = 0;

    for (uint8_t iter = 0; iter < CommodityTariffAttrTypeEnum::kAttrMax; iter++)
    {
        CommodityTariffAttrTypeEnum attr = static_cast<CommodityTariffAttrTypeEnum>(iter);
        auto & mgmtObj                   = GetMgmtObj(attr);
        if (mgmtObj.UpdateFinish(is_success))
        {
            updatedAttrIds[updatedCount++] = mgmtObj.GetAttrId();
        }
    }

    if (mTariffDataUpdatedCb != nullptr && updatedCount > 0)
    {
        ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data applied");
        if (mTariffDataUpdatedCb != nullptr)
        {
            mTariffDataUpdatedCb(false, updatedAttrIds, updatedCount);
        }
    }
    else
    {
        ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data does not change");
    }
}

void CommodityTariffDelegate::TryToactivateDelayedTariff(uint32_t now)
{
    if (!DelayedTariffUpdateIsActive)
    {
        return;
    }

    if (now >= static_cast<StartDateDataClass &>(GetMgmtObj(CommodityTariffAttrTypeEnum::kStartDate)).GetNewValue().Value())
    {
        TariffDataUpd_Finish(true);
        DelayedTariffUpdateIsActive = false;
    }
}

void CommodityTariffDelegate::CleanupTariffData()
{
    AttributeId updatedAttrIds[CommodityTariffAttrTypeEnum::kAttrMax];
    size_t updatedCount = 0;

    for (uint8_t iter = 0; iter < CommodityTariffAttrTypeEnum::kAttrMax; iter++)
    {
        CommodityTariffAttrTypeEnum attr = static_cast<CommodityTariffAttrTypeEnum>(iter);
        auto & mgmtObj                   = GetMgmtObj(attr);
        if (mgmtObj.Cleanup())
        {
            if (updatedCount < CommodityTariffAttrTypeEnum::kAttrMax)
            {
                updatedAttrIds[updatedCount++] = mgmtObj.GetAttrId();
            }
            else
            {
                ChipLogError(NotSpecified, "EGW-CTC: Too many cleaned up attributes");
                break;
            }
        }
    }

    if (mTariffDataUpdatedCb != nullptr && updatedCount > 0)
    {
        mTariffDataUpdatedCb(true, updatedAttrIds, updatedCount);
    }
}

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

CommodityTariffDelegate::CommodityTariffDelegate() {}
