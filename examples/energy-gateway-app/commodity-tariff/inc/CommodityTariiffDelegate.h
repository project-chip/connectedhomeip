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
#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>
#include <app/clusters/commodity-tariff-server/CommodityTariffAttrAccessors.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

using chip::Protocols::InteractionModel::Status;

// --- Tariff Data Updater ---
class TariffDataUpdater
{
public:
    using Callback = std::function<void(const CommodityTariffPrimaryData&)>;
    TariffDataUpdater(Callback cb) : callback(cb) {}; 
    ~TariffDataUpdater(); 


    CHIP_ERROR LoadJson(Json::Value & root);
private:
    using ValidationRule = std::function<bool(const auto&)>;
    CommodityTariffPrimaryData TariffData;
    std::map<AttributeType, ValidationRule> validationRules;
    Callback callback;
    
    template<typename T>
    T JsonTo(const JSON& json) { /* Conversion logic */ }
    
    template<typename T>
    bool Validate(const T& data) { /* Validation logic */ }
};

class CommodityTariffDelegate : public CommodityTariff::Delegate
{
    std::unique_ptr<TariffDataUpdater> updater;

public:
    CommodityTariffDelegate();
    ~CommodityTariffDelegate() = default;
private:
    // Attributes storage

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
            auto& current = mTariffData.attrName; \
            if (AttributeAccessor<attrType>::HasChanged(current, newValue)) { \
                AttributeAccessor<attrType>::Update(current, newValue); \
                ReportAttributeChange(Attributes::attrName::Id); \
            } \
            return CHIP_NO_ERROR; \
        }
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
    #undef X

    #define X(attrName, attrType) \
        CHIP_ERROR Set##attrName(const attrType& newValue) override { \
            auto& current = mCurrentData.attrName; \
            if (AttributeAccessor<attrType>::HasChanged(current, newValue)) { \
                AttributeAccessor<attrType>::Update(current, newValue); \
                ReportAttributeChange(Attributes::attrName::Id); \
            } \
            return CHIP_NO_ERROR; \
        }
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
    #undef X

    void HandleNewTariffData(Json::Value & value)
    {
        auto cb = [this](const CommodityTariffPrimaryData& data) { this->TariffDataUpdaterCb(data); };
        auto updater = std::make_unique<TariffDataUpdater>(cb);
        updater->LoadJson(value);
    }

    void TariffDataUpdaterCb(const CommodityTariffPrimaryData& newData) {
        UpdateTariffAttributes(newData);
        updater.reset();
    }

    void UpdateTariffAttributes(const CommodityTariffPrimaryData& newData);
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

    CommodityTariffDelegate * GetDelegate() { return mDelegate; };

private:
    CommodityTariffDelegate * mDelegate;
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
 