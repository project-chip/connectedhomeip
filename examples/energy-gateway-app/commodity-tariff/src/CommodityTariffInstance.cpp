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

CommodityTariffInstance::CommodityTariffInstance()
{

}

/*
CommodityTariffInstance::~CommodityTariffInstance()
{
   // TODO - free allocated space
}*/

Status CommodityTariffInstance::GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId, DayEntryStructType & aDayEntry)
{
    return Status::Success;
}

Status CommodityTariffInstance::GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                           DataModel::Nullable<chip::CharSpan> & label,
                                                           DataModel::List<const uint32_t> & dayEntryIDs,
                                                           TariffComponentStructType & aTariffComponent)
{
    return Status::Success;
}

/** -------------------Primary attrs data--------------------- **/

CHIP_ERROR CommodityTariffPrimaryData::LoadJson(const Json::Value& root)
{
    const std::map<std::string, std::function<CHIP_ERROR(const Json::Value&)>> required_tariff_items = {
        {"TariffUnit",          [this](const Json::Value& v){ return TariffUnit.LoadFromJson(v); }},
        {"StartDate",           [this](const Json::Value& v){ return StartDate.LoadFromJson(v); }},
        {"TariffInfo",          [this](const Json::Value& v){ return TariffInfo.LoadFromJson(v); }},
        {"DayEntries",          [this](const Json::Value& v){ return DayEntries.LoadFromJson(v); }},
        {"TariffComponents",    [this](const Json::Value& v){ return TariffComponents.LoadFromJson(v); }},
        {"TariffPeriods",       [this](const Json::Value& v){ return TariffPeriods.LoadFromJson(v); }}
    };

    const std::map<std::string, std::function<CHIP_ERROR(const Json::Value&)>> generic_tariff_items = {
        {"DefaultRandomizationOffset", [this](const Json::Value& v){ return DefaultRandomizationOffset.LoadFromJson(v); }},
        {"DefaultRandomizationType",   [this](const Json::Value& v){ return DefaultRandomizationType.LoadFromJson(v); }},
        {"DayPatterns",                [this](const Json::Value& v){ return DayPatterns.LoadFromJson(v); }},
        {"IndividualDays",             [this](const Json::Value& v){ return IndividualDays.LoadFromJson(v); }},
        {"CalendarPeriods",            [this](const Json::Value& v){ return CalendarPeriods.LoadFromJson(v); }},
    };

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (const auto& item : required_tariff_items) {
        auto key = item.first;
        if ( root.isMember(key) )
        {
            Json::Value value = root.get(key, Json::Value());

            err = item.second(value);

            if ( err != CHIP_NO_ERROR )
            {
                ChipLogError(NotSpecified,
                    "Invalid tariff data: Invalid value in field \"%s\"", key.c_str());
                break;
            }
        }
        else
        {
            ChipLogError(NotSpecified,
                "Invalid tariff data: the mandatory field \"%s\"is not present", key.c_str());
            err = CHIP_ERROR_INVALID_ARGUMENT;
            break;
        }


    }

    if ( err != CHIP_NO_ERROR)
    {
        return err;
    }

    /* Additional fields */    
    for (const auto& item : generic_tariff_items) {
        auto key = item.first;

        if ( root.isMember(key) )
        {
            Json::Value value = root.get(key, Json::Value());
            err = item.second(value);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified,
                    "Invalid tariff data: unable to parse value of the field \"%s\"", key.c_str());
                break;
            }
        }
    }

    return err;
}

// Specialized implementations for complex types

// TariffUnitDataClass
CHIP_ERROR TariffUnitDataClass::LoadFromJson(const Json::Value & json) {
    if (json.isUInt())
    {
        mValue.SetNonNull(static_cast<Globals::TariffUnitEnum>(json.asUInt()));
    }
    return CHIP_NO_ERROR;
}

// StartDateDataClass
CHIP_ERROR StartDateDataClass::LoadFromJson(const Json::Value & json) {
    if (json.isUInt())
    {
        mValue.SetNonNull(static_cast<uint32_t>(json.asUInt()));
    }
    return CHIP_NO_ERROR;
}

// DefaultRandomizationOffsetDataClass
CHIP_ERROR DefaultRandomizationOffsetDataClass::LoadFromJson(const Json::Value & json) {
    if (json.isUInt())
    {
        mValue.SetNonNull(static_cast<int16_t>(json.asUInt()));
    }
    return CHIP_NO_ERROR;
}

// DefaultRandomizationTypeDataClass
CHIP_ERROR DefaultRandomizationTypeDataClass::LoadFromJson(const Json::Value & json) {
    if (json.isUInt())
    {
        mValue.SetNonNull(static_cast<DayEntryRandomizationTypeEnum>(json.asUInt()));
    }
    return CHIP_NO_ERROR;
}

// TariffInfoDataClass
CHIP_ERROR TariffInfoDataClass::UpdateValue(const TariffInformationStructType& aValue)
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
            tempValue.currency.ClearValue();
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

    return CHIP_NO_ERROR;
}

CHIP_ERROR TariffInfoDataClass::LoadFromJson(const Json::Value & json)
{
    Structs::TariffInformationStruct::Type tempValue;
    Json::Value value;

    if (!json.isObject()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

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

        tempValue.currency =MakeOptional(DataModel::Nullable<Globals::Structs::CurrencyStruct::Type>(tmp_cur));
    }

    if (json.isMember("blockMode"))
    {
        tempValue.blockMode = static_cast<BlockModeEnum>(json["blockMode"].asUInt());
    }

    if (IsValid(tempValue))
    {
        mValue.SetNonNull(tempValue);
    }
    return CHIP_NO_ERROR;
}

bool TariffInfoDataClass::IsValid(const TariffInformationStructType& tariffInfo) const {

    if (tariffInfo.IsNull())
    {
        return false;
    }

    const auto& tariffInfo_val = tariffInfo.Value();
    // Validate label length if present
    if (tariffInfo_val.tariffLabel.IsNull()) {
        return false;
    }
    // TODO - add other conditions
    
    return true;
}

// TariffPeriodItemDataClass

CHIP_ERROR TariffPeriodItemDataClass::UpdateValue(const TariffPeriodStructType& aValue) {
    TariffPeriodStructType tempValue;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Label
    if (!aValue.label.IsNull()) {
        const auto& labelSpan = aValue.label.Value();
        err = SpanCopier<char>::Copy(labelSpan, tempValue.label, kDefaultStringValuesMaxBufLength - 1)
            ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
        SuccessOrExit(err);
    }

    // DayEntryIDs
    if (!aValue.dayEntryIDs.empty()) {
        err = SpanCopier<uint32_t>::Copy(
            chip::Span<const uint32_t>(aValue.dayEntryIDs.data(), aValue.dayEntryIDs.size()),
            tempValue.dayEntryIDs,
            kMaxIDsEntries)
            ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
        SuccessOrExit(err);
    }

    // TariffComponentIDs
    if (!aValue.tariffComponentIDs.empty()) {
        err = SpanCopier<uint32_t>::Copy(
            chip::Span<const uint32_t>(aValue.tariffComponentIDs.data(), aValue.tariffComponentIDs.size()),
            tempValue.tariffComponentIDs,
            kMaxIDsEntries)
            ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
        SuccessOrExit(err);
    }

    // Commit changes
    CleanupValue();
    mValue = tempValue;

exit:
    if (err != CHIP_NO_ERROR) {
        CleanupTariffPeriod(tempValue);
    }

    return err;
}

static CHIP_ERROR ParseLabelFromJson(
    const Json::Value& value,
    DataModel::Nullable<chip::CharSpan>& outLabel
)
{   
    // Check if the value exists and is a string
    if (value.isNull()) {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    if (!value.isString()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    chip::CharSpan newLabel;

    CHIP_ERROR err = StrToSpan::Copy(
            value.asString(),
            newLabel,
            kDefaultStringValuesMaxBufLength - 1
        );

    if(err == CHIP_NO_ERROR)
    {
        outLabel.SetNonNull(newLabel);
    }
    else {
        outLabel.SetNull();
        //StrToSpan::Release(newLabel);
    }

    return err;
}

static CHIP_ERROR ParseIDArray(const Json::Value& value, DataModel::List<const uint32_t>& output, size_t maxSize) {
    if (value.empty() || !value.isArray() || value.size() > maxSize)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    
    std::vector<uint32_t> ids;
    ids.reserve(value.size());

    for (const auto& id : value) {
        if (id.isUInt()) ids.push_back(id.asUInt());
    }

    bool is_success = SpanCopier<uint32_t>::Copy(
        chip::Span<const uint32_t>(ids.data(), ids.size()),
        output,
        maxSize
    );
    ids.clear();

    if (!is_success)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TariffPeriodItemDataClass::ParseFromJson(const Json::Value& json, TariffPeriodStructType& output) {
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ParseLabelFromJson(json.get("Label", Json::Value()), output.label);
    SuccessOrExit(err);

    // Parse DayEntryIDs
    err = ParseIDArray(json.get("DayEntryIDs", Json::Value()), output.dayEntryIDs, kMaxIDsEntries);
    SuccessOrExit(err);

    // Parse TariffComponentIDs
    err = ParseIDArray(json.get("TariffComponentIDs", Json::Value()), output.tariffComponentIDs, kMaxIDsEntries);
    SuccessOrExit(err);

exit:
    return err;
}

void TariffPeriodItemDataClass::CleanupTariffPeriod(TariffPeriodStructType& period) {
    if (!period.label.IsNull() && period.label.Value().data()) {
        auto& tmp_label = period.label;
        chip::Platform::MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        tmp_label.SetNull();
    }
    if (!period.dayEntryIDs.empty() && period.dayEntryIDs.data()) {
        chip::Platform::MemoryFree(const_cast<uint32_t*>(period.dayEntryIDs.data()));
        period.dayEntryIDs = DataModel::List<uint32_t>();
    }
    if (!period.tariffComponentIDs.empty() && period.tariffComponentIDs.data() ) {
        chip::Platform::MemoryFree(const_cast<uint32_t*>(period.tariffComponentIDs.data()));
        period.tariffComponentIDs = DataModel::List<uint32_t>();
    }
}

bool TariffPeriodItemDataClass::IsValid(const TariffPeriodStructType& period) const {
    // Validate label length if present
    if (period.label.IsNull()) {
        return false;
    }
    
    // Validate list sizes
    if (period.dayEntryIDs.empty()  || period.tariffComponentIDs.empty()) {
        return false;
    }

    // Validate list contents
    for (auto id : period.dayEntryIDs) {
        if (id == 0) return false; // Example validation
    }
    
    return true;
}

// TariffPeriodsDataClass
CHIP_ERROR TariffPeriodsDataClass::UpdateValue(const TariffPeriodsListType& aValue) {
    if (aValue.size() > kMaxPeriods) {
        mValue = TariffPeriodsListType();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto* buffer = static_cast<TariffPeriodStructType*>(
        chip::Platform::MemoryCalloc(aValue.size(), sizeof(TariffPeriodStructType))
    );

    if (!buffer) {
        mValue = TariffPeriodsListType();
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize with placement new
    for (size_t i = 0; i < aValue.size(); i++) {
        new (&buffer[i]) TariffPeriodStructType();
    }

    TariffPeriodsListType tempList(
        chip::Span<TariffPeriodStructType>(buffer, aValue.size())
    );

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (size_t i = 0; i < aValue.size(); i++) {
        TariffPeriodItemDataClass period(tempList[i]);
        if (!period.Update(aValue[i])) {
            err = CHIP_ERROR_NO_MEMORY;
            // Cleanup what we've allocated so far
            for (size_t j = 0; j < i; ++j) {
                TariffPeriodItemDataClass::CleanupTariffPeriod(tempList[j]);
            }
            break;
        }
    }

    if (err == CHIP_NO_ERROR) {
        CleanupValue();
        mValue = tempList;
    } else {
        chip::Platform::MemoryFree(buffer);
    }

    return err;
}

CHIP_ERROR TariffPeriodsDataClass::LoadFromJson(const Json::Value& json) {
    if (!json.isArray()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (json.size() > kMaxPeriods) {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    std::vector<TariffPeriodStructType> tempList;
    tempList.reserve(json.size());

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (const auto& entryJson : json) {
        TariffPeriodStructType entry;
        err = TariffPeriodItemDataClass::ParseFromJson(entryJson, entry);
        if ( err != CHIP_NO_ERROR)
        {
            break;
        }
        tempList.push_back(entry);
    }

    if (err == CHIP_NO_ERROR) {
        CleanupValue();

        auto* buffer = static_cast<TariffPeriodStructType*>(
            chip::Platform::MemoryCalloc(tempList.size(), sizeof(TariffPeriodStructType))
        );
        if (!buffer) {
            return CHIP_ERROR_NO_MEMORY;
        }
        // Copy data
        std::copy(tempList.begin(), tempList.end(), buffer);
        mValue = TariffPeriodsListType(buffer, tempList.size());
    }
    else
    {
        for (auto& entry : tempList) {
            TariffPeriodItemDataClass::CleanupTariffPeriod(entry);
        }
        tempList.clear();    
    }

    return err;
}

void TariffPeriodsDataClass::CleanupValue() {
    if (!mValue.empty()) {
        for (auto& period : mValue) {
            TariffPeriodItemDataClass::CleanupTariffPeriod(period);
        }
        chip::Platform::MemoryFree(const_cast<TariffPeriodStructType*>(mValue.data()));
        mValue = TariffPeriodsListType();
    }
}

// DayEntryItemDataClass

CHIP_ERROR DayEntryItemDataClass::UpdateValue(const DayEntryStructType& aValue) {
    DayEntryStructType tempValue;
    CHIP_ERROR err = CHIP_NO_ERROR;

    tempValue.dayEntryID = aValue.dayEntryID;
    tempValue.startTime = aValue.startTime;
    tempValue.duration = aValue.duration;
    tempValue.randomizationOffset = aValue.randomizationOffset;
    tempValue.randomizationType = aValue.randomizationType;

    // Validate before committing
    //VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    CleanupValue();
    mValue = tempValue;

    return err;
}

CHIP_ERROR DayEntryItemDataClass::ParseFromJson(const Json::Value& json, DayEntryStructType& output) {
    // Required fields check
    auto checkRequired = [&](const std::string& key, auto& dest, auto converter) -> CHIP_ERROR {
        if (!json.isMember(key) || !json[key].isUInt())
            return CHIP_ERROR_INVALID_ARGUMENT;
        dest = converter(json[key].asUInt());
        return CHIP_NO_ERROR;
    };

    // Optional fields check
    auto checkOptional = [&](const std::string& key, auto& dest, auto converter) {
        if (json.isMember(key) && json[key].isUInt()) {
            dest = MakeOptional(converter(json[key].asUInt()));
        }
    };

    CHIP_ERROR err = CHIP_NO_ERROR;

    if ((err = checkRequired("DayEntryID", output.dayEntryID, [](auto v){ return v; })) == CHIP_NO_ERROR &&
        (err = checkRequired("StartTime", output.startTime, [](auto v){ return static_cast<uint16_t>(v); })) == CHIP_NO_ERROR)
    {
        checkOptional("Duration", output.duration, [](auto v){ return static_cast<uint16_t>(v); });
        checkOptional("RandomizationOffset", output.randomizationOffset, [](auto v){ return static_cast<int16_t>(v); });
        checkOptional("RandomizationType", output.randomizationType, [](auto v){ return static_cast<DayEntryRandomizationTypeEnum>(v); });
    }

    return err;
}

bool DayEntryItemDataClass::IsValid(const DayEntryStructType& entry) const {
    // Validate startTime (max 1499 = 24h59m in minutes)
    if (entry.startTime > 1499) {
        return false;
    }

    // Validate duration if present
    if (entry.duration.HasValue() && entry.duration.Value() > 1440) {
        return false;
    }

    return true;
}

void DayEntryItemDataClass::CleanupDayEntry(DayEntryStructType& entry) {
    entry = DayEntryStructType();
}

// DayEntriesDataClass

CHIP_ERROR DayEntriesDataClass::UpdateValue(const DayEntryListType& aValue) {
    if (aValue.size() > kMaxDayEntries) {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    auto* buffer = static_cast<DayEntryStructType*>(
        chip::Platform::MemoryCalloc(aValue.size(), sizeof(DayEntryStructType))
    );

    if (!buffer) {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Copy data
    for (size_t i = 0; i < aValue.size(); i++) {
        new (&buffer[i]) DayEntryStructType(aValue[i]);
    }

    CleanupValue();
    mValue = DayEntryListType(chip::Span<DayEntryStructType>(buffer, aValue.size()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DayEntriesDataClass::LoadFromJson(const Json::Value& json) {
    if (!json.isArray()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (json.size() > kMaxDayEntries) {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    std::vector<DayEntryStructType> tempList;
    tempList.reserve(json.size());

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (const auto& entryJson : json) {
        DayEntryStructType entry;
        err = DayEntryItemDataClass::ParseFromJson(entryJson, entry);
        if (err != CHIP_NO_ERROR)
        {
           break;
        }
        tempList.push_back(entry);
    }

    if (err == CHIP_NO_ERROR) {
        CleanupValue();

        auto* buffer = static_cast<DayEntryStructType*>(
            chip::Platform::MemoryCalloc(tempList.size(), sizeof(DayEntryStructType))
        );
        if (!buffer) {
            return CHIP_ERROR_NO_MEMORY;
        }
        // Copy data
        std::copy(tempList.begin(), tempList.end(), buffer);
        mValue = DayEntryListType(buffer, tempList.size());
    }
    else
    {
        tempList.clear();        
    }

    return err;
}

void DayEntriesDataClass::CleanupValue() {
    if (!mValue.empty()) {
        chip::Platform::MemoryFree(const_cast<DayEntryStructType*>(mValue.data()));
        mValue = DayEntryListType();
    }
}

// TariffComponentItemDataClass

CHIP_ERROR TariffComponentItemDataClass::UpdateValue(const TariffComponentStructType& aValue) {
    TariffComponentStructType tempValue;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Copy basic fields
    tempValue.tariffComponentID = aValue.tariffComponentID;
    tempValue.price = aValue.price;
    tempValue.friendlyCredit = aValue.friendlyCredit;
    tempValue.auxiliaryLoad = aValue.auxiliaryLoad;
    tempValue.peakPeriod = aValue.peakPeriod;
    tempValue.powerThreshold = aValue.powerThreshold;
    tempValue.threshold = aValue.threshold;
    tempValue.predicted = aValue.predicted;

    // Handle label separately (may need memory allocation)
    if (aValue.label.HasValue()) {
        const auto& labelNullable = aValue.label.Value();
        if (!labelNullable.IsNull()) {
            const auto& labelSpan = labelNullable.Value();
            DataModel::Nullable<chip::CharSpan> newLabel;
            err = SpanCopier<char>::Copy(labelSpan, newLabel, kDefaultStringValuesMaxBufLength - 1)
                ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
            SuccessOrExit(err);
            tempValue.label.SetValue(newLabel);
        } else {
            DataModel::Nullable<chip::CharSpan> nullLabel;
            nullLabel.SetNull();
            tempValue.label.SetValue(nullLabel);
        }
    } else {
        tempValue.label.ClearValue();
    }

    // Validate before committing
    //VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    CleanupValue();
    mValue = tempValue;

exit:
    if (err != CHIP_NO_ERROR) {
        CleanupTariffComponent(tempValue);
    }
    return err;
}

//static CHIP_ERROR ParsePriceFromJson

CHIP_ERROR TariffComponentItemDataClass::ParseFromJson(const Json::Value& json, TariffComponentStructType& output) {
    DataModel::Nullable<chip::CharSpan> tempLabel;

    output.tariffComponentID = 0;
    output.threshold.SetNull();
    tempLabel.SetNull();

    const std::map<std::string, std::function<void(const Json::Value&)>> handlers = {
        {"TariffComponentID",   [&output](const Json::Value& v){ output.tariffComponentID = (v.isUInt() ? (v.asUInt()): 0); }},
        {"Threshold",           [&output](const Json::Value& v){ output.threshold = (v.isUInt() ? (v.asUInt()): 0); }},
        {"Label",               [&tempLabel](const Json::Value& v){ ParseLabelFromJson(v, tempLabel); }},

        {"FriendlyCredit",      [&output](const Json::Value& v){ output.friendlyCredit = MakeOptional((v.isBool() ? (v.asBool()): 0)); }},
        {"Predicted",           [&output](const Json::Value& v){ output.predicted = MakeOptional((v.isBool() ? (v.asBool()): 0)); }},
    
        {"Price",               [](const Json::Value& v){ }},
        {"AuxiliaryLoad",       [](const Json::Value& v){ }},
        {"PeakPeriod",          [](const Json::Value& v){ }},
        {"PowerThreshold",      [](const Json::Value& v){ }},
    };

    if (!json.isMember("TariffComponentID"))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto& item : handlers) {
        auto key = item.first;

        if ( json.isMember(key) )
        {
            Json::Value value = json.get(key, Json::Value());
            item.second(value);
        }
    }

    if ( !tempLabel.IsNull() )
    {
        output.label = MakeOptional(tempLabel);        
    }

    return CHIP_NO_ERROR;
}

void TariffComponentItemDataClass::CleanupTariffComponent(TariffComponentStructType& component) {
    if (component.label.HasValue() && !component.label.Value().IsNull()) {
        auto& tmp_label = component.label.Value();
        chip::Platform::MemoryFree(const_cast<char*>(tmp_label.Value().data()));
        tmp_label.SetNull();
        component.label.ClearValue();
    }
}

bool TariffComponentItemDataClass::IsValid(const TariffComponentStructType& component) const {
    // Validate required field
    if (component.tariffComponentID == 0) {
        return false;
    }
    
    // Validate label if present
    if (component.label.HasValue()) {
        const auto& labelNullable = component.label.Value();
        if (!labelNullable.IsNull()) {
            const auto& labelSpan = labelNullable.Value();
            if (labelSpan.size() >= kDefaultStringValuesMaxBufLength) {
                return false;
            }
        }
    }
    
    return true;
}

// TariffComponentsDataClass

CHIP_ERROR TariffComponentsDataClass::UpdateValue(const TariffComponentsListType& aValue) {
    if (aValue.size() > kMaxComponents) {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    auto* buffer = static_cast<TariffComponentStructType*>(
        chip::Platform::MemoryCalloc(aValue.size(), sizeof(TariffComponentStructType))
    );

    if (!buffer) {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (size_t i = 0; i < aValue.size(); i++) {
        new (&buffer[i]) TariffComponentStructType();
        TariffComponentItemDataClass component(buffer[i]);
        if (!component.Update(aValue[i])) {
            err = CHIP_ERROR_NO_MEMORY;
            // Cleanup what we've allocated so far
            for (size_t j = 0; j < i; j++) {
                TariffComponentItemDataClass::CleanupTariffComponent(buffer[j]);
            }
            break;
        }
    }

    if (err == CHIP_NO_ERROR) {
        CleanupValue();
        mValue = TariffComponentsListType(chip::Span<TariffComponentStructType>(buffer, aValue.size()));
    } else {
        chip::Platform::MemoryFree(buffer);
    }

    return err;
}

CHIP_ERROR TariffComponentsDataClass::LoadFromJson(const Json::Value& json) {
    if (!json.isArray()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (json.size() > kMaxComponents) {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    std::vector<TariffComponentStructType> tempList;
    tempList.reserve(json.size());

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (const auto& entryJson : json) {
        TariffComponentStructType entry;
        err = TariffComponentItemDataClass::ParseFromJson(entryJson, entry);
        if (err != CHIP_NO_ERROR) {
            break;
        }
        tempList.push_back(entry);
    }

    if (err == CHIP_NO_ERROR) {
        CleanupValue();

        auto* buffer = static_cast<TariffComponentStructType*>(
            chip::Platform::MemoryCalloc(tempList.size(), sizeof(TariffComponentStructType))
        );
        if (!buffer) {
            return CHIP_ERROR_NO_MEMORY;
        }
        // Copy data
        std::copy(tempList.begin(), tempList.end(), buffer);
        mValue = TariffComponentsListType(buffer, tempList.size());
    }
    else
    {
        for (auto& entry : tempList) {
            TariffComponentItemDataClass::CleanupTariffComponent(entry);
        }
        tempList.clear();        
    }

    return err;
}

void TariffComponentsDataClass::CleanupValue() {
    if (!mValue.empty()) {
        for (auto& component : mValue) {
            TariffComponentItemDataClass::CleanupTariffComponent(component);
        }
        chip::Platform::MemoryFree(const_cast<TariffComponentStructType*>(mValue.data()));
        mValue = TariffComponentsListType();
    }
}

//IndividualDaysDataClass
CHIP_ERROR IndividualDaysDataClass::UpdateValue(const DayStructsListType& aValue) {
    return CHIP_NO_ERROR;
}

CHIP_ERROR IndividualDaysDataClass::LoadFromJson(const Json::Value& json) {
    return CHIP_NO_ERROR;
}

void IndividualDaysDataClass::CleanupValue() {
    mValue.SetNull();
}

//DayPatternsDataClass
CHIP_ERROR DayPatternsDataClass::UpdateValue(const DayPatternsListType& aValue) {
    return CHIP_NO_ERROR;
}

CHIP_ERROR DayPatternsDataClass::LoadFromJson(const Json::Value& json) {
    return CHIP_NO_ERROR;
}

void DayPatternsDataClass::CleanupValue() {

}

//CalendarPeriodsDataClass
CHIP_ERROR CalendarPeriodsDataClass::UpdateValue(const CalendarPeriodsListType& aValue) {
    return CHIP_NO_ERROR;
}

CHIP_ERROR CalendarPeriodsDataClass::LoadFromJson(const Json::Value& json) {
    return CHIP_NO_ERROR;
}

void CalendarPeriodsDataClass::CleanupValue() {
    mValue.SetNull();
}
/** -------------------Current attrs data--------------------- **/

//CurrentDayEntryDataClass

//NextDayEntryDateDataClass

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}