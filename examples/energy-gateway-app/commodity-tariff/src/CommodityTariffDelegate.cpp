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

// Specialized implementations for complex types

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

CHIP_ERROR TariffPeriodItemDataClass::LoadFromJson(const Json::Value& json) {
    TariffPeriodStructType tempValue;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Parse label
    err = ParseLabelFromJson(json, tempValue);
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

CHIP_ERROR TariffPeriodItemDataClass::ParseLabelFromJson(
    const Json::Value& json, TariffPeriodStructType& output)
{
    Json::Value value = json.get("Label", Json::Value());

    if (value.empty() || !value.isString()) {
        output.label.SetNull();
        return CHIP_NO_ERROR;
    }

    const std::string str = value.asString();
    if (str.size() >= kDefaultStringValuesMaxBufLength) {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (!SpanCopier<char>::Copy(chip::CharSpan(str.data(), str.size()),
                               output.label,
                               kDefaultStringValuesMaxBufLength - 1)) {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TariffPeriodItemDataClass::ParseIDsFromJson(
    const Json::Value& json, const char* fieldName,
    DataModel::List<const uint32_t>& output)
{
    Json::Value value = json.get(fieldName, Json::Value());

    if (value.empty() || !value.isArray()) {
        output = DataModel::List<const uint32_t>();
        return CHIP_NO_ERROR;
    }

    if (value.size() > kMaxIDsEntries) {
        return CHIP_ERROR_INVALID_LIST_LENGTH;
    }

    std::vector<uint32_t> ids;
    ids.reserve(value.size());

    for (const auto& id : value) {
        if (!id.isUInt()) {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ids.push_back(id.asUInt());
    }

    if (!SpanCopier<uint32_t>::Copy(
        chip::Span<const uint32_t>(ids.data(), ids.size()),
        output,
        kMaxIDsEntries))
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
    if (!period.label.IsNull()) {
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
    DayEntryStructType tempValue;
    CHIP_ERROR err = ParseFromJson(json, tempValue);

    if (err == CHIP_NO_ERROR && IsValid(tempValue)) {
        CleanupValue();
        mValue = tempValue;
    }

    return err;
}

CHIP_ERROR DayEntryItemDataClass::ParseFromJson(const Json::Value& json, DayEntryStructType& output) {
    // Required fields
    if (!json.isMember("dayEntryID") || !json["dayEntryID"].isUInt()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    output.dayEntryID = json["dayEntryID"].asUInt();

    if (!json.isMember("startTime") || !json["startTime"].isUInt()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    output.startTime = static_cast<uint16_t>(json["startTime"].asUInt());

    // Optional fields
    if (json.isMember("duration") && json["duration"].isUInt()) {
        output.duration = MakeOptional(static_cast<uint16_t>(json["duration"].asUInt()));
    }

    if (json.isMember("randomizationOffset") && json["randomizationOffset"].isInt()) {
        output.randomizationOffset = MakeOptional(static_cast<int16_t>(json["randomizationOffset"].asInt()));
    }

    if (json.isMember("randomizationType") && json["randomizationType"].isUInt()) {
        output.randomizationType = MakeOptional(static_cast<DayEntryRandomizationTypeEnum>(json["randomizationType"].asUInt()));
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR TariffComponentItemDataClass::LoadFromJson(const Json::Value& json) {
    TariffComponentStructType tempValue;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Required field
    if (!json.isMember("tariffComponentID") || !json["tariffComponentID"].isUInt()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    tempValue.tariffComponentID = json["tariffComponentID"].asUInt();

    // Optional fields
    if (json.isMember("price") && json["price"].isObject()) {
        // Parse price struct...
    }

    if (json.isMember("label") && json["label"].isString()) {
        err = ParseLabelFromJson(json["label"], tempValue.label);
        SuccessOrExit(err);
    }

    // Parse other optional fields...

    // Validate and commit
    VerifyOrExit(IsValid(tempValue), err = CHIP_ERROR_INVALID_ARGUMENT);
    CleanupValue();
    mValue = tempValue;

exit:
    if (err != CHIP_NO_ERROR) {
        CleanupTariffComponent(tempValue);
    }
    return err;
}

CHIP_ERROR TariffComponentItemDataClass::ParseLabelFromJson(
    const Json::Value& json, 
    chip::Optional<DataModel::Nullable<chip::CharSpan>>& output) 
{
    if (!json.isString()) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    const std::string str = json.asString();
    if (str.size() >= kDefaultStringValuesMaxBufLength) {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    DataModel::Nullable<chip::CharSpan> newLabel;
    if (!SpanCopier<char>::Copy(
        chip::CharSpan(str.data(), str.size()),
        newLabel,
        kDefaultStringValuesMaxBufLength - 1)) 
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    output.SetValue(newLabel);
    return CHIP_NO_ERROR;
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