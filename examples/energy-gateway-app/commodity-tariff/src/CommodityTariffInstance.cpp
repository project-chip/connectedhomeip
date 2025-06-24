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

#include "CommodityTariffJsonParser.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Attributes;
using namespace chip::app::Clusters::CommodityTariff::Structs;

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

namespace JSON_Utilities {

// TariffUnitDataClass
CHIP_ERROR TariffUnit_LoadFromJson(const Json::Value & json, TariffUnitDataClass & MgmtObj)
{
    if (json.isUInt())
    {
        CHIP_ERROR err = MgmtObj.CreateNewValue(0);
        if (CHIP_NO_ERROR == err)
        {
            *(MgmtObj.GetNewValue()) = static_cast<Globals::TariffUnitEnum>(json.asUInt());
            MgmtObj.MarkAsAssigned();
        }

        return err;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// StartDateDataClass
CHIP_ERROR StartDate_LoadFromJson(const Json::Value & json, StartDateDataClass & MgmtObj)
{
    if (json.isUInt())
    {
        CHIP_ERROR err = MgmtObj.CreateNewValue(0);
        if (CHIP_NO_ERROR == err)
        {
            *(MgmtObj.GetNewValue()) = static_cast<uint32_t>(json.asUInt());
            MgmtObj.MarkAsAssigned();
        }

        return err;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// DefaultRandomizationOffsetDataClass
CHIP_ERROR DefaultRandomizationOffset_LoadFromJson(const Json::Value & json, DefaultRandomizationOffsetDataClass & MgmtObj)
{
    if (json.isUInt())
    {
        CHIP_ERROR err = MgmtObj.CreateNewValue(0);
        if (CHIP_NO_ERROR == err)
        {
            *(MgmtObj.GetNewValue()) = static_cast<int16_t>(json.asInt());
            MgmtObj.MarkAsAssigned();
        }

        return err;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// DefaultRandomizationTypeDataClass
CHIP_ERROR DefaultRandomizationType_LoadFromJson(const Json::Value & json, DefaultRandomizationTypeDataClass & MgmtObj)
{
    if (json.isUInt())
    {
        CHIP_ERROR err = MgmtObj.CreateNewValue(0);
        if (CHIP_NO_ERROR == err)
        {
            *(MgmtObj.GetNewValue()) = (static_cast<DayEntryRandomizationTypeEnum>(json.asUInt()));
            MgmtObj.MarkAsAssigned();
        }

        return err;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

// Specialization for TariffInformationStruct
template <>
CHIP_ERROR
JsonParser<TariffInformationStruct::Type, TariffInfoDataClass, false>::ParseFromJson(const Json::Value & json,
                                                                                     TariffInformationStruct::Type & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (json.isMember("TariffLabel"))
    {
        if (CHIP_NO_ERROR != ( err = ParseLabelFromJson(json["TariffLabel"], output.tariffLabel)) )
        {
            return err;
        }
    }
    else
    {
        output.tariffLabel.SetNull();
    }
    if (json.isMember("ProviderName"))
    {
        if (CHIP_NO_ERROR != (err = ParseLabelFromJson(json["ProviderName"], output.providerName)) )
        {
            return err;
        }
    }
    else
    {
        output.providerName.SetNull();
    }
    if (json.isMember("Currency"))
    {
        CurrencyStruct::Type tmp_cur;
        tmp_cur.currency      = static_cast<uint16_t>(json["Currency"]["Currency"].asUInt());
        tmp_cur.decimalPoints = static_cast<uint8_t>(json["Currency"]["DecimalPoints"].asUInt());
        output.currency       = MakeOptional(DataModel::Nullable<Globals::Structs::CurrencyStruct::Type>(tmp_cur));
    }
    if (json.isMember("blockMode"))
    {
        output.blockMode = static_cast<BlockModeEnum>(json["blockMode"].asUInt());
    }

    return err;
}

// TariffInfoDataClass
CHIP_ERROR TariffInfo_LoadFromJson(const Json::Value & json, TariffInfoDataClass & MgmtObj)
{
    return JsonParser<TariffInformationStruct::Type, TariffInfoDataClass, false>::LoadFromJson(json, MgmtObj);
}

// TariffPeriodItemDataClass
template <>
CHIP_ERROR JsonParser<TariffPeriodStructType, TariffPeriodsDataClass>::ParseFromJson(const Json::Value & json,
                                                                                     TariffPeriodStructType & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ParseLabelFromJson(json.get("Label", Json::Value()), output.label);
    SuccessOrExit(err);

    // Parse DayEntryIDs
    err = ParseIDArray(json.get("DayEntryIDs", Json::Value()), output.dayEntryIDs, CommodityTariffAttrsDataMgmt::kDayEntriesAttrMaxLength);
    SuccessOrExit(err);

    // Parse TariffComponentIDs
    err = ParseIDArray(json.get("TariffComponentIDs", Json::Value()), output.tariffComponentIDs, CommodityTariffAttrsDataMgmt::kTariffComponentsAttrMaxLength);
    SuccessOrExit(err);

exit:
    return err;
}

// TariffPeriodsDataClass

CHIP_ERROR TariffPeriods_LoadFromJson(const Json::Value & json, TariffPeriodsDataClass & MgmtObj)
{
    return JsonParser<TariffPeriodStructType, TariffPeriodsDataClass>::LoadFromJson(json, MgmtObj, CommodityTariffAttrsDataMgmt::kTariffPeriodsAttrMaxLength);
}

// DayEntriesDataClass
template <>
CHIP_ERROR JsonParser<DayEntryStructType, DayEntriesDataClass>::ParseFromJson(const Json::Value & json, DayEntryStructType & output)
{
    // Required fields check
    auto checkRequired = [&](const std::string & key, auto & dest, auto converter) -> CHIP_ERROR {
        if (!json.isMember(key) || !json[key].isUInt())
            return CHIP_ERROR_INVALID_ARGUMENT;
        dest = converter(json[key].asUInt());
        return CHIP_NO_ERROR;
    };

    // Optional fields check
    auto checkOptional = [&](const std::string & key, auto & dest, auto converter) {
        if (json.isMember(key) && json[key].isUInt())
        {
            dest = MakeOptional(converter(json[key].asUInt()));
        }
    };

    CHIP_ERROR err = CHIP_NO_ERROR;

    if ((err = checkRequired("DayEntryID", output.dayEntryID, [](auto v) { return v; })) == CHIP_NO_ERROR &&
        (err = checkRequired("StartTime", output.startTime, [](auto v) { return static_cast<uint16_t>(v); })) == CHIP_NO_ERROR)
    {
        checkOptional("Duration", output.duration, [](auto v) { return static_cast<uint16_t>(v); });
        checkOptional("RandomizationOffset", output.randomizationOffset, [](auto v) { return static_cast<int16_t>(v); });
        checkOptional("RandomizationType", output.randomizationType,
                      [](auto v) { return static_cast<DayEntryRandomizationTypeEnum>(v); });
    }

    return err;
}

CHIP_ERROR DayEntries_LoadFromJson(const Json::Value & json, DayEntriesDataClass & MgmtObj)
{
    return JsonParser<DayEntryStructType, DayEntriesDataClass>::LoadFromJson(json, MgmtObj, kDayEntriesAttrMaxLength);
}

// TariffComponentsDataClass
template <>
CHIP_ERROR JsonParser<TariffComponentStructType, TariffComponentsDataClass>::ParseFromJson(const Json::Value & json,
                                                                                           TariffComponentStructType & output)
{
    DataModel::Nullable<chip::CharSpan> tempLabel;

    output.tariffComponentID = 0;
    output.threshold.SetNull();
    tempLabel.SetNull();

    const std::map<std::string, std::function<void(const Json::Value &)>> handlers = {
        { "TariffComponentID", [&output](const Json::Value & v) { output.tariffComponentID = (v.isUInt() ? (v.asUInt()) : 0); } },
        { "Threshold", [&output](const Json::Value & v) { output.threshold         = (v.isUInt() ? (v.asUInt()) : 0); } },
        { "Label", [&tempLabel](const Json::Value & v) { ParseLabelFromJson(v, tempLabel); } },

        { "FriendlyCredit",
          [&output](const Json::Value & v) { output.friendlyCredit                 = MakeOptional((v.isBool() ? (v.asBool()) : 0)); } },
        { "Predicted", [&output](const Json::Value & v) { output.predicted         = MakeOptional((v.isBool() ? (v.asBool()) : 0)); } },
    };

    if (!json.isMember("TariffComponentID"))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & item : handlers)
    {
        auto key = item.first;

        if (json.isMember(key))
        {
            Json::Value value = json.get(key, Json::Value());
            item.second(value);
        }
    }

    if (json.isMember("Price"))
    {
        const Json::Value & priceJson = json["Price"];
        TariffPriceStruct::Type tmp_price;
        
        if (priceJson.isMember("PriceType"))
        {
            tmp_price.priceType = static_cast<Globals::TariffPriceTypeEnum>(priceJson["PriceType"].asUInt());
        }
        else
        {
            // The PriceType is mandatory field
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (priceJson.isMember("Price"))
        {
            if (priceJson["Price"].isDouble())
            {
                double priceValue = priceJson["Price"].asDouble();
                tmp_price.price = MakeOptional(static_cast<int64_t>(priceValue * 100));
            }
            else if (priceJson["Price"].isInt64())
            {
                tmp_price.price = MakeOptional(static_cast<int64_t>(priceJson["Price"].asInt64()));
            }
            else
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (priceJson.isMember("PriceLevel"))
        {

            if (priceJson["PriceLevel"].isUInt())
            {
                tmp_price.priceLevel = MakeOptional(static_cast<int16_t>(priceJson["PriceLevel"].asUInt()));
            }
            else
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        output.price = MakeOptional(tmp_price);          
    }

    if (json.isMember("AuxiliaryLoad"))
    {
        const Json::Value & auxLoadJson = json["AuxiliaryLoad"];
        AuxiliaryLoadSwitchSettingsStruct::Type auxiliaryLoad;
        
        if (auxLoadJson.isMember("Number"))
        {
            auxiliaryLoad.number = static_cast<uint8_t>(auxLoadJson["Number"].asUInt());
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (auxLoadJson.isMember("RequiredState"))
        {
            auxiliaryLoad.requiredState = static_cast<AuxiliaryLoadSettingEnum>(auxLoadJson["RequiredState"].asUInt());
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        output.auxiliaryLoad = MakeOptional(auxiliaryLoad);          
    }

    if (json.isMember("PeakPeriod"))
    {
        const Json::Value & PeakPeriodJson = json["PeakPeriod"];
        PeakPeriodStruct::Type peakPeriod;
        
        if (PeakPeriodJson.isMember("Severity"))
        {
            peakPeriod.severity = static_cast<PeakPeriodSeverityEnum>(PeakPeriodJson["Severity"].asUInt());
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (PeakPeriodJson.isMember("PeakPeriod"))
        {
            peakPeriod.peakPeriod = static_cast<uint16_t>(PeakPeriodJson["PeakPeriod"].asUInt());
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        output.peakPeriod = MakeOptional(peakPeriod);          
    }

    if (json.isMember("PowerThreshold"))
    {
        const Json::Value & powerThresholdJson = json["PowerThreshold"];
        PowerThresholdStruct::Type powerThreshold;
        
        if (powerThresholdJson.isMember("PowerThreshold"))
        {
            if (powerThresholdJson["PowerThreshold"].isUInt())
            {
                powerThreshold.powerThreshold = MakeOptional(static_cast<int64_t>(powerThresholdJson["PowerThreshold"].asUInt()));
            }
            else
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (powerThresholdJson.isMember("ApparentPowerThreshold"))
        {
            if (powerThresholdJson["ApparentPowerThreshold"].isUInt())
            {
                powerThreshold.apparentPowerThreshold = MakeOptional(static_cast<int64_t>(powerThresholdJson["ApparentPowerThreshold"].asUInt()));
            }
            else
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        if (powerThresholdJson.isMember("PowerThresholdSource"))
        {
            if (powerThresholdJson["PowerThresholdSource"].isUInt())
            {
                powerThreshold.powerThresholdSource = MakeNullable(static_cast<PowerThresholdSourceEnum>(powerThresholdJson["PowerThresholdSource"].asUInt()));
            }
            else
            {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
        }

        output.powerThreshold = MakeOptional(powerThreshold);          
    }

    if (!tempLabel.IsNull())
    {
        output.label = MakeOptional(tempLabel);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TariffComponents_LoadFromJson(const Json::Value & json, TariffComponentsDataClass & MgmtObj)
{
    return JsonParser<TariffComponentStructType, TariffComponentsDataClass>::LoadFromJson(json, MgmtObj,
                                                                                          CommodityTariffAttrsDataMgmt::kTariffComponentsAttrMaxLength);
}

// DayPatternsDataClass
template <>
CHIP_ERROR JsonParser<DayPatternStructType, DayPatternsDataClass>::ParseFromJson(const Json::Value & json,
                                                                                 DayPatternStructType & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Helper function to parse hex string
    auto parseHexString = [](const std::string & hexStr) -> std::optional<uint8_t> {
        if (!hexStr.empty() && hexStr.size() >= 3 && 
            hexStr[0] == '0' && (hexStr[1] != 'x' || hexStr[1] != 'X')) {
            return static_cast<uint8_t>(std::stoul(hexStr.substr(2), nullptr, 16));
        }

        return std::nullopt;
    };

    // Required fields check with hex string support
    auto checkRequired = [&](const std::string & key, auto & dest, auto converter) -> CHIP_ERROR {
        if (!json.isMember(key)) {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (key == "DaysOfWeek" && json[key].isString()) {
            // Special handling for hex string
            auto hexValue = parseHexString(json[key].asString());
            if (!hexValue) {
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            dest = converter(*hexValue);
        } 
        else if (json[key].isUInt()) {
            // Original numeric handling
            dest = converter(json[key].asUInt());
        }
        else {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        
        return CHIP_NO_ERROR;
    };

    err = checkRequired("DayPatternID", output.dayPatternID, [](auto v) { return v; });
    SuccessOrExit(err);

    err = checkRequired("DaysOfWeek", output.daysOfWeek, [](auto v) {
        return chip::BitMask<DayPatternDayOfWeekBitmap>(static_cast<std::underlying_type_t<DayPatternDayOfWeekBitmap>>(v));
    });
    SuccessOrExit(err);

    err = ParseIDArray(json.get("DayEntryIDs", Json::Value()), output.dayEntryIDs, CommodityTariffAttrsDataMgmt::kDayEntriesAttrMaxLength);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR DayPatterns_LoadFromJson(const Json::Value & json, DayPatternsDataClass & MgmtObj)
{
    return JsonParser<DayPatternStructType, DayPatternsDataClass>::LoadFromJson(json, MgmtObj, CommodityTariffAttrsDataMgmt::kDayPatternsAttrMaxLength);
}

// IndividualDaysDataClass
template <>
CHIP_ERROR JsonParser<DayStructType, IndividualDaysDataClass>::ParseFromJson(const Json::Value & json,
                                                                                 DayStructType & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Required fields check
    auto checkRequired = [&](const std::string & key, auto & dest, auto converter) -> CHIP_ERROR {
        if (!json.isMember(key) || !json[key].isUInt())
            return CHIP_ERROR_INVALID_ARGUMENT;
        dest = converter(json[key].asUInt());
        return CHIP_NO_ERROR;
    };

    err = checkRequired("Date", output.date, [](auto v) { return static_cast<uint32_t>(v);; });
    SuccessOrExit(err);

    err = checkRequired("DayType", output.dayType, [](auto v) {
        return static_cast<DayTypeEnum>(v);
    });
    SuccessOrExit(err);

    err = ParseIDArray(json.get("DayEntryIDs", Json::Value()), output.dayEntryIDs, CommodityTariffAttrsDataMgmt::kDayEntriesAttrMaxLength);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR IndividualDays_LoadFromJson(const Json::Value & json, IndividualDaysDataClass & MgmtObj)
{
    return JsonParser<DayStructType, IndividualDaysDataClass>::LoadFromJson(json, MgmtObj, CommodityTariffAttrsDataMgmt::kIndividualDaysAttrMaxLength);
}

// CalendarPeriodsDataClass
template <>
CHIP_ERROR JsonParser<CalendarPeriodStructType, CalendarPeriodsDataClass>::ParseFromJson(const Json::Value & json,
                                                                                 CalendarPeriodStructType & output)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t startDateVal = 0;

    if (json.isMember("StartDate"))
    {
        if (json["StartDate"].isUInt())
        {
            startDateVal = json["StartDate"].asUInt();

            if (startDateVal > 0)
            {
                output.startDate.SetNonNull(static_cast<uint32_t>(startDateVal));
            }
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (startDateVal == 0)
    {
        output.startDate.SetNull();
    }

    err = ParseIDArray(json.get("DayPatternIDs", Json::Value()), output.dayPatternIDs, CommodityTariffAttrsDataMgmt::kCalendarPeriodItemMaxDayPatternIDs);
    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR CalendarPeriods_LoadFromJson(const Json::Value & json, CalendarPeriodsDataClass & MgmtObj)
{
    return JsonParser<CalendarPeriodStructType, CalendarPeriodsDataClass>::LoadFromJson(json, MgmtObj, CommodityTariffAttrsDataMgmt::kCalendarPeriodsAttrMaxLength);
}
} // namespace JSON_Utilities

CHIP_ERROR CommodityTariffDelegate::LoadTariffData(const Json::Value & root)
{
    const std::map<std::string, std::function<CHIP_ERROR(const Json::Value &)>> required_tariff_items = {
        { "TariffUnit",
          [this](const Json::Value & v) { return JSON_Utilities::TariffUnit_LoadFromJson(v, this->GetTariffUnit_MgmtObj()); } },
        { "StartDate",
          [this](const Json::Value & v) { return JSON_Utilities::StartDate_LoadFromJson(v, this->GetStartDate_MgmtObj()); } },
        { "TariffInfo",
          [this](const Json::Value & v) { return JSON_Utilities::TariffInfo_LoadFromJson(v, this->GetTariffInfo_MgmtObj()); } },
        { "DayEntries",
          [this](const Json::Value & v) { return JSON_Utilities::DayEntries_LoadFromJson(v, this->GetDayEntries_MgmtObj()); } },
        { "TariffComponents",
          [this](const Json::Value & v) {
              return JSON_Utilities::TariffComponents_LoadFromJson(v, this->GetTariffComponents_MgmtObj());
          } },
        { "TariffPeriods",
          [this](const Json::Value & v) {
              return JSON_Utilities::TariffPeriods_LoadFromJson(v, this->GetTariffPeriods_MgmtObj());
          } }
    };

    const std::map<std::string, std::function<CHIP_ERROR(const Json::Value &)>> generic_tariff_items = {
        { "DefaultRandomizationOffset",
          [this](const Json::Value & v) {
              return JSON_Utilities::DefaultRandomizationOffset_LoadFromJson(v, this->GetDefaultRandomizationOffset_MgmtObj());
          } },
        { "DefaultRandomizationType",
          [this](const Json::Value & v) {
              return JSON_Utilities::DefaultRandomizationType_LoadFromJson(v, this->GetDefaultRandomizationType_MgmtObj());
          } },
        { "DayPatterns",
          [this](const Json::Value & v) { return JSON_Utilities::DayPatterns_LoadFromJson(v, this->GetDayPatterns_MgmtObj()); } },
        { "IndividualDays",
          [this](const Json::Value & v) {
              return JSON_Utilities::IndividualDays_LoadFromJson(v, this->GetIndividualDays_MgmtObj());
          } },
        { "CalendarPeriods",
          [this](const Json::Value & v) {
              return JSON_Utilities::CalendarPeriods_LoadFromJson(v, this->GetCalendarPeriods_MgmtObj());
          } },
    };

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (const auto & item : required_tariff_items)
    {
        auto key = item.first;
        if (root.isMember(key))
        {
            Json::Value value = root.get(key, Json::Value());

            err = item.second(value);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Invalid tariff data: Invalid value in field \"%s\"", key.c_str());
                break;
            }
        }
        else
        {
            ChipLogError(NotSpecified, "Invalid tariff data: the mandatory field \"%s\"is not present", key.c_str());
            err = CHIP_ERROR_INVALID_ARGUMENT;
            break;
        }
    }

    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    /* Additional fields */
    for (const auto & item : generic_tariff_items)
    {
        auto key = item.first;

        if (root.isMember(key))
        {
            Json::Value value = root.get(key, Json::Value());
            err               = item.second(value);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Invalid tariff data: unable to parse value of the field \"%s\"", key.c_str());
                break;
            }
        }
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

    if ( GetIndividualDays_MgmtObj().IsValid() && (GetIndividualDays_MgmtObj().GetNewValue() != nullptr) )
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

    if ( GetCalendarPeriods_MgmtObj().IsValid() && ( GetCalendarPeriods_MgmtObj().GetNewValue() != nullptr) )
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
    
    if(!DayEntriesData_is_available)
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
