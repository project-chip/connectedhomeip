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

#include <fstream>

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

static constexpr const char * default_tariff_data = "./commodity-tariff/DefaultTariff.json";




static bool LoadJsonFile(const char * aFname, Json::Value &jsonValue)
{
    bool is_ok = false;
    std::ifstream ifs;    
    Json::CharReaderBuilder builder;
    Json::String errs;

    ifs.open(aFname);

    if (!ifs.good())
    {
        ChipLogError(NotSpecified,
             "AllClusters App: Error open file %s", aFname);
        goto exit;
    }

    if (!parseFromStream(builder, ifs, &jsonValue, &errs)) {
        ChipLogError(NotSpecified,
             "AllClusters App: Error parsing JSON file %s with error %s:", aFname, errs.c_str());
        goto exit;
    }

    if (jsonValue.empty() || !jsonValue.isObject())
    {
        ChipLogError(NotSpecified, "JSON dtat %s", aFname);
        goto exit;
    }

    is_ok = true;

exit:
    return is_ok;
}

CHIP_ERROR CommodityTariffInstance::Init()
{
    return Instance::Init();
}

void CommodityTariffInstance::Shutdown()
{
    Instance::Shutdown();
}

CommodityTariffDelegate::CommodityTariffDelegate()
{
    Json::Value json_root;

    VerifyOrDieWithMsg(LoadJsonFile(default_tariff_data, json_root), AppServer, "Unable to load default tariff file");
    LoadTariffData(json_root);
    mTariffData.LoadJson(json_root);
}

/*
CommodityTariffDelegate::~CommodityTariffDelegate()
{
   // TODO - free allocated space
}*/

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

/** -------------------Primary attrs data--------------------- **/

CHIP_ERROR CommodityTariffPrimaryData::LoadJson(const Json::Value& root)
{
    const std::map<std::string, std::function<CHIP_ERROR(const Json::Value&)>> required_tariff_items = {
        {"TariffUnit",          [this](const Json::Value& v){ return TariffUnit.LoadFromJson(v); }},
        {"StartDate",           [this](const Json::Value& v){ return StartDate.LoadFromJson(v); }},
        {"TariffLabel",         [this](const Json::Value& v){ return TariffInfo.LoadFromJson(v); }},
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

            if (value.isArray())
            {
                err = item.second(value);
            }
            else
            {
                item.second(root);
            }
        }
        else
        {
            ChipLogError(NotSpecified,
                "Invalid tariff data: the mandatory field \"%s\"is not present", key.c_str());
            err = CHIP_ERROR_INVALID_ARGUMENT;
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

static bool ParseIDArray(const Json::Value& json, DataModel::List<const uint32_t>& output, size_t maxSize) {
    if (json.empty() || !json.isArray() || json.size() > maxSize) return false;
    
    std::vector<uint32_t> ids;
    ids.reserve(json.size());

    for (const auto& id : json) {
        if (id.isUInt()) ids.push_back(id.asUInt());
    }

    bool is_success = SpanCopier<uint32_t>::Copy(
        chip::Span<const uint32_t>(ids.data(), ids.size()),
        output,
        maxSize
    );
    ids.clear();
    return is_success;
}

// Specialized implementations for complex types

// TariffUnitDataClass
CHIP_ERROR TariffUnitDataClass::LoadFromJson(const Json::Value & json) {
    return CHIP_NO_ERROR;
}

// StartDateDataClass
CHIP_ERROR StartDateDataClass::LoadFromJson(const Json::Value & json) {
    return CHIP_NO_ERROR;
}

// DefaultRandomizationOffsetDataClass
CHIP_ERROR DefaultRandomizationOffsetDataClass::LoadFromJson(const Json::Value & json) {
    return CHIP_NO_ERROR;
}

// DefaultRandomizationTypeDataClass
CHIP_ERROR DefaultRandomizationTypeDataClass::LoadFromJson(const Json::Value & json) {
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

        tempValue.currency.Value().SetNonNull(tmp_cur);
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

bool TariffInfoDataClass::IsValid(const TariffInformationStructType& period) const {

    if (period.IsNull())
    {
        return false;
    }

    const auto& period_val = period.Value();
    // Validate label length if present
    if (!period_val.tariffLabel.IsNull()) {
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

CHIP_ERROR TariffPeriodItemDataClass::LoadFromJson(const Json::Value& json) {
    TariffPeriodStructType tempValue;
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ParseLabelFromJson(json.get("Label", Json::Value()), tempValue.label);
    SuccessOrExit(err);

    // Parse DayEntryIDs
    err = ParseIDsFromJson(json, "DayEntryIDs", tempValue.dayEntryIDs);
    SuccessOrExit(err);

    // Parse TariffComponentIDs
    err = ParseIDsFromJson(json, "TariffComponentIDs", tempValue.tariffComponentIDs);
    SuccessOrExit(err);

    // Validate final structure
    VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Commit changes
    CleanupValue();
    mValue = tempValue;

exit:
    if (err != CHIP_NO_ERROR) {
        CleanupTariffPeriod(tempValue);
    }
    return err;
}

CHIP_ERROR TariffPeriodItemDataClass::ParseIDsFromJson(
    const Json::Value& json, const char* fieldName,
    DataModel::List<const uint32_t>& output)
{
    Json::Value value = json.get(fieldName, Json::Value());

    if (!ParseIDArray(value, output, kMaxIDsEntries))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void TariffPeriodItemDataClass::CleanupTariffPeriod(TariffPeriodStructType& period) {
    if (!period.label.IsNull() && period.label.Value().data()) {
        chip::Platform::MemoryFree(const_cast<char*>(period.label.Value().data()));
    }
    if (!period.dayEntryIDs.empty() && period.dayEntryIDs.data()) {
        chip::Platform::MemoryFree(const_cast<uint32_t*>(period.dayEntryIDs.data()));
    }
    if (!period.tariffComponentIDs.empty() && period.tariffComponentIDs.data() ) {
        chip::Platform::MemoryFree(const_cast<uint32_t*>(period.tariffComponentIDs.data()));
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
CHIP_ERROR TariffPeriodsDataClass::UpdateValue(const TariffPeriodsList& aValue) {
    if (aValue.size() > kMaxPeriods) {
        mValue = TariffPeriodsList();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto* buffer = static_cast<Structs::TariffPeriodStruct::Type*>(
        chip::Platform::MemoryCalloc(aValue.size(), sizeof(Structs::TariffPeriodStruct::Type))
    );

    if (!buffer) {
        mValue = TariffPeriodsList();
        return CHIP_ERROR_NO_MEMORY;
    }

    // Initialize with placement new
    for (size_t i = 0; i < aValue.size(); i++) {
        new (&buffer[i]) Structs::TariffPeriodStruct::Type();
    }

    TariffPeriodsList tempList(
        chip::Span<Structs::TariffPeriodStruct::Type>(buffer, aValue.size())
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

    auto* buffer = static_cast<Structs::TariffPeriodStruct::Type*>(
        chip::Platform::MemoryCalloc(json.size(), sizeof(Structs::TariffPeriodStruct::Type))
    );

    if (!buffer) {
        return CHIP_ERROR_NO_MEMORY;
    }

    TariffPeriodsList tempList(
        chip::Span<Structs::TariffPeriodStruct::Type>(buffer, json.size())
    );

    CHIP_ERROR overallErr = CHIP_NO_ERROR;

    for (Json::ArrayIndex i = 0; i < json.size(); ++i) {
        TariffPeriodItemDataClass period(tempList[i]);
        CHIP_ERROR err = period.LoadFromJson(json[i]);

        if (err != CHIP_NO_ERROR) {
            overallErr = err;
            // Cleanup what we've allocated so far
            for (Json::ArrayIndex j = 0; j < i; ++j) {
                TariffPeriodItemDataClass::CleanupTariffPeriod(tempList[j]);
            }
            break;
        }
    }

    if (overallErr == CHIP_NO_ERROR) {
        CleanupValue(); // Cleanup old data
        mValue = tempList;
    } else {
        chip::Platform::MemoryFree(buffer);
    }

    return overallErr;
}

void TariffPeriodsDataClass::CleanupValue() {
    if (!mValue.empty()) {
        for (auto& period : mValue) {
            TariffPeriodItemDataClass::CleanupTariffPeriod(period);
        }
        chip::Platform::MemoryFree(const_cast<Structs::TariffPeriodStruct::Type*>(mValue.data()));
        mValue = TariffPeriodsList();
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
    VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    CleanupValue();
    mValue = tempValue;

exit:
    return err;
}

CHIP_ERROR DayEntryItemDataClass::LoadFromJson(const Json::Value& json) {
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

    DayEntryStructType tempValue;
    CHIP_ERROR err;
    if ((err = checkRequired("DayEntryID", tempValue.dayEntryID, [](auto v){ return v; })) == CHIP_NO_ERROR &&
        (err = checkRequired("StartTime", tempValue.startTime, [](auto v){ return static_cast<uint16_t>(v); })) == CHIP_NO_ERROR)
    {
        checkOptional("Duration", tempValue.duration, [](auto v){ return static_cast<uint16_t>(v); });
        checkOptional("RandomizationOffset", tempValue.randomizationOffset, [](auto v){ return static_cast<int16_t>(v); });
        checkOptional("RandomizationType", tempValue.randomizationType, [](auto v){ return static_cast<DayEntryRandomizationTypeEnum>(v); });
    }

    if (err == CHIP_NO_ERROR && IsValid(tempValue)) {
        CleanupValue();
        mValue = tempValue;
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
    // No dynamic memory in basic DayEntryStruct
}

// DayEntriesDataClass

CHIP_ERROR DayEntriesDataClass::UpdateValue(const DayEntryListType& aValue) {
    if (aValue.size() > kMaxDayEntries) {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    auto* buffer = static_cast<Structs::DayEntryStruct::Type*>(
        chip::Platform::MemoryCalloc(aValue.size(), sizeof(Structs::DayEntryStruct::Type))
    );

    if (!buffer) {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Copy data
    for (size_t i = 0; i < aValue.size(); i++) {
        new (&buffer[i]) Structs::DayEntryStruct::Type(aValue[i]);
    }

    CleanupValue();
    mValue = DayEntryListType(chip::Span<Structs::DayEntryStruct::Type>(buffer, aValue.size()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DayEntriesDataClass::LoadFromJson(const Json::Value& json) {
    if (!json.isArray()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (json.size() > kMaxDayEntries) {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    std::vector<Structs::DayEntryStruct::Type> tempEntries;
    tempEntries.reserve(json.size());

    for (const auto& entryJson : json) {
        Structs::DayEntryStruct::Type entry;
        DayEntryItemDataClass item(entry);
        CHIP_ERROR err = item.LoadFromJson(entryJson);
        if (err != CHIP_NO_ERROR) {
            return err;
        }
        tempEntries.push_back(entry);
    }

    return UpdateValue(DayEntryListType(tempEntries.data(), tempEntries.size()));
}

void DayEntriesDataClass::CleanupValue() {
    if (!mValue.empty()) {
        chip::Platform::MemoryFree(const_cast<Structs::DayEntryStruct::Type*>(mValue.data()));
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
    VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);

    CleanupValue();
    mValue = tempValue;

exit:
    if (err != CHIP_NO_ERROR) {
        CleanupTariffComponent(tempValue);
    }
    return err;
}

//static CHIP_ERROR ParsePriceFromJson

CHIP_ERROR TariffComponentItemDataClass::LoadFromJson(const Json::Value& json) {
    CHIP_ERROR err = CHIP_NO_ERROR;    
    TariffComponentStructType tempValue;
    DataModel::Nullable<chip::CharSpan> tempLabel;

    tempValue.tariffComponentID = 0;
    tempLabel.SetNull();
    tempValue.threshold.SetNull();

    const std::map<std::string, std::function<void(const Json::Value&)>> handlers = {
        {"TariffComponentID",   [&tempValue](const Json::Value& v){ tempValue.tariffComponentID = (v.isUInt() ? (v.asUInt()): 0); }},
        {"Threshold",           [&tempValue](const Json::Value& v){ tempValue.threshold = (v.isUInt() ? (v.asUInt()): 0); }},
        {"Label",               [&tempLabel](const Json::Value& v){ ParseLabelFromJson(v, tempLabel); }},

        {"FriendlyCredit",      [&tempValue](const Json::Value& v){ tempValue.friendlyCredit = MakeOptional((v.isBool() ? (v.asBool()): 0)); }},
        {"Predicted",           [&tempValue](const Json::Value& v){ tempValue.predicted = MakeOptional((v.isBool() ? (v.asBool()): 0)); }},
    
        {"Price",               [](const Json::Value& v){ }},
        {"AuxiliaryLoad",       [](const Json::Value& v){ }},
        {"PeakPeriod",          [](const Json::Value& v){ }},
        {"PowerThreshold",      [](const Json::Value& v){ }},
    };

    for (const auto& item : handlers) {
        auto key = item.first;

        if ( json.isMember(key) )
        {
            Json::Value value = json.get(key, Json::Value());
            item.second(value);
        }
    }

    tempValue.label = MakeOptional(tempLabel);

    VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);
    CleanupValue();

    mValue = tempValue;

exit:
    if (err != CHIP_NO_ERROR) {
        CleanupTariffComponent(tempValue);
    }
    return err;
}

void TariffComponentItemDataClass::CleanupTariffComponent(TariffComponentStructType& component) {
    if (component.label.HasValue() && !component.label.Value().IsNull()) {
        chip::Platform::MemoryFree(const_cast<char*>(component.label.Value().Value().data()));
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

    auto* buffer = static_cast<Structs::TariffComponentStruct::Type*>(
        chip::Platform::MemoryCalloc(aValue.size(), sizeof(Structs::TariffComponentStruct::Type))
    );

    if (!buffer) {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    for (size_t i = 0; i < aValue.size(); i++) {
        new (&buffer[i]) Structs::TariffComponentStruct::Type();
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
        mValue = TariffComponentsListType(chip::Span<Structs::TariffComponentStruct::Type>(buffer, aValue.size()));
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

    std::vector<Structs::TariffComponentStruct::Type> tempComponents;
    tempComponents.reserve(json.size());

    for (const auto& compJson : json) {
        Structs::TariffComponentStruct::Type component;
        TariffComponentItemDataClass item(component);
        CHIP_ERROR err = item.LoadFromJson(compJson);
        if (err != CHIP_NO_ERROR) {
            return err;
        }
        tempComponents.push_back(component);
    }

    return UpdateValue(TariffComponentsListType(tempComponents.data(), tempComponents.size()));
}

void TariffComponentsDataClass::CleanupValue() {
    if (!mValue.empty()) {
        for (auto& component : mValue) {
            TariffComponentItemDataClass::CleanupTariffComponent(component);
        }
        chip::Platform::MemoryFree(const_cast<Structs::TariffComponentStruct::Type*>(mValue.data()));
        mValue = TariffComponentsListType();
    }
}

//DayPatternsDataClass
CHIP_ERROR DayPatternsDataClass::LoadFromJson(const Json::Value& json) {
    return CHIP_NO_ERROR;
}

//IndividualDaysDataClass
CHIP_ERROR IndividualDaysDataClass::LoadFromJson(const Json::Value& json) {
    return CHIP_NO_ERROR;
}

//CalendarPeriodsDataClass
CHIP_ERROR CalendarPeriodsDataClass::LoadFromJson(const Json::Value& json) {
    return CHIP_NO_ERROR;
}

/** -------------------Current attrs data--------------------- **/

//CurrentDayEntryDataClass

//NextDayEntryDateDataClass

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}