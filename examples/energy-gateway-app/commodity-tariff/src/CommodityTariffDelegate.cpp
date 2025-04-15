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

 #include <CommodityTariffDelegate.h>
 #include <app/EventLogging.h>
 #include <app/reporting/reporting.h>
 
 #include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>
 
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
 
 CHIP_ERROR CommodityTariffInstance::Init()
 {
     return Instance::Init();
 }
 
 void CommodityTariffInstance::Shutdown()
 {
     Instance::Shutdown();
 }
 /*
 CommodityTariffDelegate::CommodityTariffDelegate()
 {
    // TODO - set default values for attributes 
 }
 
 CommodityTariffDelegate::~CommodityTariffDelegate()
 {
    // TODO - free allocated space
 }
*/

Status CommodityTariffDelegate::GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId, Structs::DayEntryStruct::Type & aDayEntry)
{
    return Status::Success;
}

Status CommodityTariffDelegate::GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                           DataModel::Nullable<chip::CharSpan> & label,
                                                           DataModel::List<const uint32_t> & dayEntryIDs,
                                                           Structs::TariffComponentStruct::Type & aTariffComponent)
{
    return Status::Success;
}

 // --------------- Internal Attribute Set APIs
 /*
 CHIP_ERROR SetTariffInfo(const DataModel::Nullable<Structs::TariffInformationStruct::Type>& newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic



    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, TariffInfo::Id;);
    }

    return err;
}


CHIP_ERROR SetTariffUnit(Globals::TariffUnitEnum newValue);
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, TariffInfo::Id);
    }

    return err;
}

CHIP_ERROR SetStartDate(const DataModel::Nullable<epoch_s> newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, StartDate::Id);
    }

    return err;
}

CHIP_ERROR SetDefaultRandomizationOffset(const DataModel::Nullable<int16_t> newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, DefaultRandomizationOffset::Id);
    }

    return err;
}

CHIP_ERROR SetDefaultRandomizationType(const DataModel::Nullable<DayEntryRandomizationTypeEnum> newValue)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, DefaultRandomizationType::Id);
    }

    return err;
}

// List Setters (replace entire lists)
CHIP_ERROR SetCalendarPeriods(const DataModel::List<Structs::CalendarPeriodStruct::Type>* newList)
{
    bool is_changed = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO - some value change logic

    if ( (err == CHIP_NO_ERROR) && is_changed)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, CalendarPeriods::Id);
    }

    return err;
}

CHIP_ERROR SetDayPatterns(const DataModel::List<Structs::DayPatternStruct::Type>*);
CHIP_ERROR SetIndividualDays(const DataModel::List<Structs::DayStruct::Type>*);
CHIP_ERROR SetDayEntries(const DataModel::List<Structs::DayEntryStruct::Type>*);
CHIP_ERROR SetTariffPeriods(const DataModel::List<Structs::TariffPeriodStruct::Type>*);
CHIP_ERROR SetTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);

// Secondary Attributes
CHIP_ERROR SetCurrentDay(const DataModel::Nullable<Structs::DayStruct::Type>*);
CHIP_ERROR SetNextDay(const DataModel::Nullable<Structs::DayStruct::Type>*);
CHIP_ERROR SetCurrentDayEntry(const DataModel::Nullable<Structs::DayEntryStruct::Type>*);
CHIP_ERROR SetNextDayEntry(const DataModel::Nullable<Structs::DayEntryStruct::Type>*);
CHIP_ERROR SetCurrentDayEntryDate(const DataModel::Nullable<epoch_s>);
CHIP_ERROR SetNextDayEntryDate(const DataModel::Nullable<epoch_s>);
CHIP_ERROR SetCurrentTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
CHIP_ERROR SetNextTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
*/

// Specialized implementations for complex types

// TariffInfoDataClass
void TariffInfoDataClass::UpdateValue(const DataModel::Nullable<Structs::TariffInformationStruct::Type>& aValue)
{
    if (!aValue.IsNull())
    {
        Structs::TariffInformationStruct::Type tempValue;

        const auto& val = aValue.Value();
        const auto& tariffLabel_val =  val.tariffLabel.Value();
        const auto& providerName_val = val.providerName.Value();

        if (!val.tariffLabel.IsNull())
        {
            memmove(mTariffLabelValueBuffer ,tariffLabel_val.data(), tariffLabel_val.size());
            tempValue.tariffLabel = CharSpan(mTariffLabelValueBuffer, tariffLabel_val.size());
        }
        else
        {
            tempValue.tariffLabel.SetNull();
        }

        if (!val.providerName.IsNull())
        {
            memmove(mProviderNameValueBuffer, providerName_val.data(), providerName_val.size());
            tempValue.providerName = CharSpan(mProviderNameValueBuffer, providerName_val.size());
        }
        else
        {
            tempValue.providerName.SetNull();
        }

        if (val.currency.HasValue())
        {
            const auto& currencyNullable = val.currency.Value();
            if (!currencyNullable.IsNull())
            {
                const auto& currencyValue = currencyNullable.Value();
                tempValue.currency = MakeOptional(CurrencyStruct::Type{
                    static_cast<uint16_t>(currencyValue.currency),
                    currencyValue.decimalPoints
                });
            }
        }
        else{
            tempValue.currency.Value().SetNull();
        }

        if (!val.blockMode.IsNull())
        {
            tempValue.blockMode = val.blockMode;
        }
        else
        {
            tempValue.blockMode.SetNull();
        }

        mValue.SetNonNull(tempValue);
    }
    else
    {
        mValue.SetNull();
    }
}

bool TariffInfoDataClass::LoadFromJson(const Json::Value & json)
{
    Structs::TariffInformationStruct::Type tempValue;
    Json::Value value;

    if (json.isMember("TariffLabel"))
    {
        value = json.get("TariffLabel", Json::Value());
        strcpy(mTariffLabelValueBuffer, value.asCString());
        tempValue.tariffLabel = chip::CharSpan::fromCharString(mTariffLabelValueBuffer);
    }
    else
    {
        tempValue.tariffLabel.SetNull();
    }

    if (json.isMember("ProviderName"))
    {
        value = json.get("ProviderName", Json::Value());
        strcpy(mProviderNameValueBuffer, value.asCString());
        tempValue.providerName = chip::CharSpan::fromCharString(mProviderNameValueBuffer);
    }
    else
    {
        tempValue.providerName.SetNull();
    }


    if (json.isMember("Currency"))
    {
        CurrencyStruct::Type tmp_cur;
        tmp_cur.currency = static_cast<uint16_t>(json["Currency"]["Currency"].asUInt());
        tmp_cur.decimalPoints = static_cast<uint8_t>(json["Currency"]["DecimalPoints"].asUInt());

        tempValue.currency.Value().SetNonNull(tmp_cur);
    }

    if (json.isMember("blockMode"))
    {
        tempValue.blockMode = static_cast<BlockModeEnum>(json["blockMode"].asUInt());
    }

    return IsValid(tempValue);
}

/*
// DayEntriesDataClass
template <>
bool DayEntriesDataClass::UpdateValue(const DataModel::List<Structs::DayEntryStruct::Type>* aValue)
    {
    for (const auto &entry : aValue)
    {
        if (!ValidationHelpers::ValidateDayEntryTime(entry.startTime))
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (entry.duration.HasValue() && !ValidationHelpers::ValidateDuration(entry.duration.Value()))
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

// DayPatternsDataClass
template <>
bool DayPatternsDataClass::UpdateValue(const DataModel::List<Structs::DayPatternStruct::Type>* aValue)
    {
    for (const auto &pattern : aValue)
    {
        if (pattern.daysOfWeek == 0 || pattern.daysOfWeek > 0x7F)
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (pattern.dayEntryIDs.size() == 0 || pattern.dayEntryIDs.size() > 96)
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

// IndividualDaysDataClass
template <>
bool IndividualDaysDataClass::UpdateValue(const DataModel::List<Structs::DayStruct::Type>* aValue)
    {
    for (const auto &day : aValue)
    {
        if (day.dayEntryIDs.size() == 0 || day.dayEntryIDs.size() > 96)
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

// CalendarPeriodsDataClass
template <>
bool CalendarPeriodsDataClass::UpdateValue(const DataModel::List<Structs::CalendarPeriodStruct::Type>* aValue)
    {
    for (const auto &period : aValue)
    {
        if (period.dayPatternIDs.size() == 0 || period.dayPatternIDs.size() > 7)
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

// TariffPeriodsDataClass
template <>
bool TariffPeriodsDataClass::UpdateValue(const DataModel::List<Structs::TariffPeriodStruct::Type>* aValue)
    {
    for (const auto &period : aValue)
    {
        if (!ValidationHelpers::ValidateStringLength(period.label, 128))
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (period.dayEntryIDs.size() == 0 || period.dayEntryIDs.size() > 20)
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (period.tariffComponentIDs.size() == 0 || period.tariffComponentIDs.size() > 20)
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

// TariffComponentsDataClass
template <>
bool TariffComponentsDataClass::UpdateValue(const DataModel::List<Structs::TariffComponentStruct::Type>* aValue)
    {
    for (const auto &component : aValue)
    {
        if (component.label.HasValue() && 
            !ValidationHelpers::ValidateStringLength(component.label.Value(), 128))
    {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

// CurrentDayDataClass and NextDayDataClass share the same validation
template <>
bool CurrentDayDataClass::UpdateValue(const DataModel::Nullable<Structs::DayStruct::Type>* aValue)
    {
    if (aValue->IsNull())
    {
        return CTC_BaseDataClass::UpdateValue(aValue);
    }
    const auto &day = aValue->Value();
    if (day.dayEntryIDs.size() == 0 || day.dayEntryIDs.size() > 96)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

template <>
bool NextDayDataClass::UpdateValue(const DataModel::Nullable<Structs::DayStruct::Type>* aValue)
    {
    return CurrentDayDataClass::UpdateValue(aValue);
}

// CurrentDayEntryDataClass and NextDayEntryDataClass share the same validation
template <>
bool CurrentDayEntryDataClass::UpdateValue(const DataModel::Nullable<Structs::DayEntryStruct::Type>* aValue)
    {
    if (aValue->IsNull())
    {
        return CTC_BaseDataClass::UpdateValue(aValue);
    }
    const auto &entry = aValue->Value();
    if (!ValidationHelpers::ValidateDayEntryTime(entry.startTime))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (entry.duration.HasValue() && !ValidationHelpers::ValidateDuration(entry.duration.Value()))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CTC_BaseDataClass::UpdateValue(aValue);
}

template <>
bool NextDayEntryDataClass::UpdateValue(const DataModel::Nullable<Structs::DayEntryStruct::Type>* aValue)
    {
    return CurrentDayEntryDataClass::UpdateValue(aValue);
}

// CurrentTariffComponentsDataClass and NextTariffComponentsDataClass share the same validation
template <>
bool CurrentTariffComponentsDataClass::UpdateValue(const DataModel::List<Structs::TariffComponentStruct::Type>* aValue)
    {
    return TariffComponentsDataClass::UpdateValue(aValue);
}

template <>
bool NextTariffComponentsDataClass::UpdateValue(const DataModel::List<Structs::TariffComponentStruct::Type>* aValue)
    {
    return TariffComponentsDataClass::UpdateValue(aValue);
}
*/