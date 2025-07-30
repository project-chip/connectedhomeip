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
#include <csetjmp>

#include <app/clusters/commodity-tariff-server/CommodityTariffConsts.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Attributes;
using namespace chip::app::Clusters::CommodityTariff::Structs;
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

void CommodityTariffInstance::Shutdown()
{
    Instance::Shutdown();
}

CHIP_ERROR CommodityTariffDelegate::LoadTariffData(const TariffDataSamples::TariffDataSet &data)
{/*
    const std::map<std::string, std::function<CHIP_ERROR(const Json::Value &)>> generic_tariff_items = {
        { "DayPatterns",
          [this](const Json::Value & v) { return JSON_Utilities::DayPatterns_LoadDataSample(v, this->GetDayPatterns_MgmtObj()); } },
        { "IndividualDays",
          [this](const Json::Value & v) {
              return JSON_Utilities::IndividualDays_LoadDataSample(v, this->GetIndividualDays_MgmtObj());
          } },
        { "CalendarPeriods",
          [this](const Json::Value & v) {
              return JSON_Utilities::CalendarPeriods_LoadDataSample(v, this->GetCalendarPeriods_MgmtObj());
          } },
    };*/

    CHIP_ERROR err = CHIP_NO_ERROR;

    const std::map<bool, std::string> required_tariff_items = {
        {data.StartDate.IsNull(), "TariffUnit"},
        {data.StartDate.IsNull(),  "StartDate"},
        {data.TariffInfo.IsNull(), "TariffInfo"},
        {data.DayEntries.IsNull(), "DayEntries"},
        {data.TariffComponents.IsNull(), "TariffComponents"},
        {data.TariffPeriods.IsNull(), "TariffPeriods"},
    };

    for (const auto & item : required_tariff_items)
    {
        if (!item.first)
        {
            continue;
        }

        ChipLogError(NotSpecified, "Invalid tariff data: the mandatory field \"%s\"is not present", item.second.c_str());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
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

    assert(!UpdCtx.DayEntryKeyIDs.empty());
    assert(!UpdCtx.TariffComponentKeyIDs.empty());

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
        if (!UpdCtx.TariffComponentKeyIDs.count(item))
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

    if (GetIndividualDays_MgmtObj().IsValid() && (GetIndividualDays_MgmtObj().GetNewValueData() != nullptr))
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

    if (GetCalendarPeriods_MgmtObj().IsValid() && (GetCalendarPeriods_MgmtObj().GetNewValueData() != nullptr))
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

    return true;
}

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

CommodityTariffDelegate::CommodityTariffDelegate() {}
