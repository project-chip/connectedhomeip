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
#pragma once

#include <json/json.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/data-model/Nullable.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/CodeUtils.h>
#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>

#include "CommodityTariffDataClasses.h"

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

using chip::Protocols::InteractionModel::Status;

constexpr bool operator!=(const Globals::Structs::CurrencyStruct::Type & lhs, const Globals::Structs::CurrencyStruct::Type & rhs)
{
    return ((lhs.currency != rhs.currency) || (lhs.decimalPoints != rhs.decimalPoints));
}

#define X(attrName, attrType) \
class attrName##DataClass : public CTC_BaseDataClass<attrType> { \
public: \
    attrName##DataClass(attrType& aValueStorage) \
        : CTC_BaseDataClass<attrType>(aValueStorage) { mValue = aValueStorage; }\
    ~attrName##DataClass() override = default; \
    CHIP_ERROR LoadFromJson(const Json::Value& json) override ;/*{ return CHIP_NO_ERROR; }*/ \
protected:    \
    CHIP_ERROR UpdateValue(const attrType& aValue) override { \
        mValue = aValue;        \
        return CHIP_NO_ERROR; }; \
    void CleanupValue() override {}; \
};
// Generate all classes
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES_STUBS
#undef X

#define X(attrName, attrType) \
class attrName##DataClass : public CTC_BaseDataClass<attrType> { \
public: \
    attrName##DataClass(attrType& aValueStorage) \
        : CTC_BaseDataClass<attrType>(aValueStorage) { mValue = aValueStorage; }\
    ~attrName##DataClass() override = default; \
protected:    \
    CHIP_ERROR UpdateValue(const attrType& aValue) override { \
        mValue = aValue;        \
        return CHIP_NO_ERROR; }; \
    void CleanupValue() override {}; \
};
// Generate all classes
COMMODITY_TARIFF_CURRENT_ATTRIBUTES_STUBS
#undef X

class CommodityTariffPrimaryData {
private:
// 1. First declare storage
#define X(attrName, attrType) \
    attrType m##attrName;
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

public:
// 2. Then declare DataClass objects initialized with storage
#define X(attrName, attrType) \
    attrName##DataClass attrName{m##attrName};
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

    CommodityTariffPrimaryData() = default;
    ~CommodityTariffPrimaryData() = default;

    CHIP_ERROR LoadJson(const Json::Value& root);
    bool IsValid() const { return true; }
};

class CommodityTariffCurrentData {
private:
// 1. First declare storage
#define X(attrName, attrType) \
    attrType m##attrName;
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

public:
#define X(attrName, attrType) \
    attrName##DataClass attrName{m##attrName};
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

    CommodityTariffCurrentData() = default;
    ~CommodityTariffCurrentData() = default;
};

// --- Tariff Data Updater ---
class TariffDataUpdater
{
public:
    using Callback = std::function<void(const CommodityTariffPrimaryData&)>;
    TariffDataUpdater(Callback cb, const Json::Value & json ) : callback(cb) { 
        if ( CHIP_NO_ERROR != LoadJson(json) )
        {
            ChipLogError(NotSpecified, "EGW-CTC: Invalid JSON data");
        }
        else
        {
            callback(mTariffData);
        }
    };
    ~TariffDataUpdater() = default; 
private:
    CommodityTariffPrimaryData mTariffData;
    Callback callback;

    CHIP_ERROR LoadJson(const Json::Value & root) { 
        return mTariffData.LoadJson(root);
    };
};

class CommodityTariffDelegate : public CommodityTariff::Delegate
{
    std::unique_ptr<TariffDataUpdater> updater;

public:
    CommodityTariffDelegate();
    ~CommodityTariffDelegate() = default;

    Protocols::InteractionModel::Status GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId,
                                                        Structs::DayEntryStruct::Type & aDayEntry) override;

    Protocols::InteractionModel::Status GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                                   DataModel::Nullable<chip::CharSpan> & label,
                                                                   DataModel::List<const uint32_t> & dayEntryIDs,
                                                                   Structs::TariffComponentStruct::Type & aTariffComponent) override;

#define X(attrName, attrType) \
    attrType& Get##attrName() override { return mTariffData.attrName.GetValue(); }
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

#define X(attrName, attrType) \
    attrType& Get##attrName() override { return mCurrentData.attrName.GetValue(); }
COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

private:
    CommodityTariffPrimaryData mTariffData;
    CommodityTariffCurrentData mCurrentData;

    // Tariff data Attributes
    //CHIP_ERROR SetTariffInfo(const DataModel::Nullable<Structs::TariffInformationStruct::Type>*);
    //CHIP_ERROR SetTariffUnit(Globals::TariffUnitEnum);
    //CHIP_ERROR SetStartDate(const DataModel::Nullable<epoch_s>);
    //CHIP_ERROR SetDefaultRandomizationOffset(const DataModel::Nullable<int16_t>);
    //CHIP_ERROR SetDefaultRandomizationType(const DataModel::Nullable<DayEntryRandomizationTypeEnum>);
    //// List Setters (replace entire lists)
    //CHIP_ERROR SetCalendarPeriods(const DataModel::List<Structs::CalendarPeriodStruct::Type>*);
    //CHIP_ERROR SetDayPatterns(const DataModel::List<Structs::DayPatternStruct::Type>*);
    //CHIP_ERROR SetIndividualDays(const DataModel::List<Structs::DayStruct::Type>*);
    //CHIP_ERROR SetDayEntries(const DataModel::List<Structs::DayEntryStruct::Type>*);
    //CHIP_ERROR SetTariffPeriods(const DataModel::List<Structs::TariffPeriodStruct::Type>*);
    //CHIP_ERROR SetTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
    //// Secondary Attributes
    //CHIP_ERROR SetCurrentDay(const DataModel::Nullable<Structs::DayStruct::Type>*);
    //CHIP_ERROR SetNextDay(const DataModel::Nullable<Structs::DayStruct::Type>*);
    //CHIP_ERROR SetCurrentDayEntry(const DataModel::Nullable<Structs::DayEntryStruct::Type>*);
    //CHIP_ERROR SetNextDayEntry(const DataModel::Nullable<Structs::DayEntryStruct::Type>*);
    //CHIP_ERROR SetCurrentDayEntryDate(const DataModel::Nullable<epoch_s>);
    //CHIP_ERROR SetNextDayEntryDate(const DataModel::Nullable<epoch_s>);
    //CHIP_ERROR SetCurrentTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
    //CHIP_ERROR SetNextTariffComponents(const DataModel::List<Structs::TariffComponentStruct::Type>*);
    // Generate setters
    #define X(attrName, attrType) \
        CHIP_ERROR Set##attrName(const attrType& newValue) override { \
            if (mTariffData.attrName.Update(newValue)) { \
                ChipLogProgress(NotSpecified, "EGW-CTC: The attr %s updated", #attrName);\
                ReportAttributeChange(Attributes::attrName::Id); \
            } \
            return CHIP_NO_ERROR; \
        }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
    #undef X

    #define X(attrName, attrType) \
        CHIP_ERROR Set##attrName(const attrType& newValue) override { \
            if (mCurrentData.attrName.Update(newValue)) { \
                ReportAttributeChange(Attributes::attrName::Id); \
            } \
            return CHIP_NO_ERROR; \
        }
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
    #undef X

    void ReportAttributeChange(uint16_t attributeId) override {
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, attributeId);
    };

    void LoadTariffData(const Json::Value & value)
    {
        auto cb = [this](const CommodityTariffPrimaryData& data) { this->TariffDataUpdaterCb(data); };
        updater = std::make_unique<TariffDataUpdater>(cb, value);
    }

    void TariffDataUpdaterCb(const CommodityTariffPrimaryData& newData) {
        if (!newData.IsValid())
        {
            ChipLogError(NotSpecified, "EGW-CTC: Tariff data rejected due to inconsistencies");
        }
        else
        {
            UpdateTariffAttributes(newData);
            ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data applied");
            //updater.reset();
        }
    }

    void UpdateTariffAttributes(const CommodityTariffPrimaryData& newData) 
    {
#define X(attrName, attrType) \
        Set##attrName(newData.attrName.GetValue());
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X
    };
    void UpdateCurrentAttrs();
};

class CommodityTariffInstance : public Instance
{
public:
    CommodityTariffInstance(EndpointId aEndpointId, CommodityTariffDelegate & aDelegate, Feature aFeature) :
        CommodityTariff::Instance(aEndpointId, aDelegate, aFeature)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    CommodityTariffInstance(const CommodityTariffInstance &)             = delete;
    CommodityTariffInstance(const CommodityTariffInstance &&)            = delete;
    CommodityTariffInstance & operator=(const CommodityTariffInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();
    CHIP_ERROR AppInit();

    CommodityTariffDelegate * GetDelegate() { return mDelegate; };

private:
    CommodityTariffDelegate * mDelegate;
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
 