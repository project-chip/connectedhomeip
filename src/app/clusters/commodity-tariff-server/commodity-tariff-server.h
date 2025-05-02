/**
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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>

#include "CommodityTariffAttrsDataClassesTemplate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

typedef uint32_t epoch_s;

/**
 * @def COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
 * @brief Macro defining Primary attributes for Commodity Tariff
 * 
 * Primary attributes are elements that can only be changed by a special trigger from the updater 
 * when new tariff data has been obtained. These represent the fundamental tariff configuration.
 * 
 * The macro defines the following attributes:
 * - TariffUnit: The unit of measurement for the tariff (nullable enum)
 * - StartDate: The starting date of the tariff (nullable uint32)
 * - DefaultRandomizationOffset: Default offset for randomization (nullable int16)
 * - DefaultRandomizationType: Default type of randomization (nullable enum)
 * - CalendarPeriods: List of calendar period structures
 * - DayPatterns: List of day pattern structures
 * - IndividualDays: List of individual day structures
 * - DayEntries: List of day entry structures
 * - TariffPeriods: List of tariff period structures
 * - TariffComponents: List of tariff component structures
 */
#define COMMODITY_TARIFF_PRIMARY_ATTRIBUTES \
    X(TariffInfo,                   DataModel::Nullable<Structs::TariffInformationStruct::Type>) \
    X(TariffUnit,                   DataModel::Nullable<Globals::TariffUnitEnum>) \
    X(StartDate,                    DataModel::Nullable<uint32_t>) \
    X(DefaultRandomizationOffset,   DataModel::Nullable<int16_t>) \
    X(DefaultRandomizationType,     DataModel::Nullable<DayEntryRandomizationTypeEnum>) \
    X(DayEntries,                   DataModel::List<Structs::DayEntryStruct::Type>)  \
    X(TariffComponents,             DataModel::List<Structs::TariffComponentStruct::Type>) \
    X(TariffPeriods,                DataModel::List<Structs::TariffPeriodStruct::Type>) \
    X(DayPatterns,                  DataModel::List<Structs::DayPatternStruct::Type>) \
    X(IndividualDays,               DataModel::Nullable<DataModel::List<Structs::DayStruct::Type>>) \
    X(CalendarPeriods,              DataModel::Nullable<DataModel::List<Structs::CalendarPeriodStruct::Type>>)

/**
 * @def COMMODITY_TARIFF_CURRENT_ATTRIBUTES
 * @brief Macro defining Current attributes for Commodity Tariff
 * 
 * Current attributes change internally depending on the current time context. These represent
 * the dynamically changing state of the tariff system.
 * 
 * The macro defines the following attributes:
 * - CurrentDay: The current day structure (nullable)
 * - NextDay: The next day structure (nullable)
 * - CurrentDayEntry: The current day entry structure (nullable)
 * - NextDayEntry: The next day entry structure (nullable)
 * - CurrentDayEntryDate: The date of current day entry (nullable uint32)
 * - NextDayEntryDate: The date of next day entry (nullable uint32)
 * - CurrentTariffComponents: List of current tariff component structures
 * - NextTariffComponents: List of next tariff component structures
 */
#define COMMODITY_TARIFF_CURRENT_ATTRIBUTES \
    X(CurrentDay,                   DataModel::Nullable<Structs::DayStruct::Type>) \
    X(NextDay,                      DataModel::Nullable<Structs::DayStruct::Type>) \
    X(CurrentDayEntry,              DataModel::Nullable<Structs::DayEntryStruct::Type>) \
    X(NextDayEntry,                 DataModel::Nullable<Structs::DayEntryStruct::Type>) \
    X(CurrentDayEntryDate,          DataModel::Nullable<uint32_t>) \
    X(NextDayEntryDate,             DataModel::Nullable<uint32_t>) \
    X(CurrentTariffComponents,      DataModel::List<Structs::TariffComponentStruct::Type>) \
    X(NextTariffComponents,         DataModel::Nullable<DataModel::List<Structs::TariffComponentStruct::Type>>)

/**
 * @def X(attrName, attrType)
 * @brief Macro generating attribute-specific data management classes
 * 
 * For each attribute in COMMODITY_TARIFF_PRIMARY_ATTRIBUTES, this macro generates a specialized
 * data management class that inherits from CTC_BaseDataClass. These classes provide:
 * - Type-safe storage and access to attribute values
 * - Change detection and validation mechanisms
 * - Memory management for complex types
 * 
 * The generated classes follow this pattern:
 * @code
 * class attrName##DataClass : public CTC_BaseDataClass<attrType> {
 * public:
 *     attrName##DataClass(attrType& aValueStorage) : CTC_BaseDataClass<attrType>(aValueStorage) {}
 *     ~attrName##DataClass() = default;
 * };
 * @endcode
 * 
 * @see CTC_BaseDataClass for base functionality
 */
#define X(attrName, attrType) \
class attrName##DataClass : public CTC_BaseDataClass<attrType> { \
public: \
    attrName##DataClass(attrType& aValueStorage) \
        : CTC_BaseDataClass<attrType>(aValueStorage) {} \
};
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

class CommodityTariffPrimaryData {
private:
// 1. First declare storage
#define X(attrName, attrType) \
    attrType m##attrName;
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

public:
/**
 * @def X(attrName, attrType)
 * @brief Macro generating attribute-specific data management objects
 */
#define X(attrName, attrType) \
    attrName##DataClass attrName{m##attrName};
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

    CommodityTariffPrimaryData() = default;
    virtual ~CommodityTariffPrimaryData() = default;

    //CHIP_ERROR LoadJson(const Json::Value& root);
    bool IsValid() const { return __is_valid(); }

protected:
    virtual bool __is_valid() const { return true; };
};

class CommodityTariffDataProvider
{
public:
    CommodityTariffDataProvider()          = default;
    virtual ~CommodityTariffDataProvider() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }


    virtual Protocols::InteractionModel::Status GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId,
                                                                Structs::DayEntryStruct::Type & aDayEntry) = 0;

    virtual Protocols::InteractionModel::Status GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                                           DataModel::Nullable<chip::CharSpan> & label,
                                                                           DataModel::List<const uint32_t> & dayEntryIDs,
                                                                           Structs::TariffComponentStruct::Type & aTariffComponent) = 0;

                              
    void TariffDataUpdate(const CommodityTariffPrimaryData& newData)
    {
        if (!newData.IsValid())
        {
            ChipLogError(NotSpecified, "EGW-CTC: Tariff data rejected due to inconsistencies");
        }
        else
        {
    #define X(attrName, attrType) \
            Set##attrName(newData.attrName.GetValue());
        COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
    #undef X
            ChipLogProgress(NotSpecified, "EGW-CTC: Tariff data applied");
        }
    }

/**
 * @brief Standard getter methods for Primary attributes
 * 
 * These methods provide access to the Primary tariff attributes stored in mTariffData.
 * 
 * The following getters are generated (one for each attribute in COMMODITY_TARIFF_PRIMARY_ATTRIBUTES):
 * @code
 * attrType& Get<attrName>()
 * @endcode
 * Where:
 * - <attrName> is the name of each Primary attribute
 * - Returns a reference to the attribute's value
 */
#define X(attrName, attrType) \
    attrType& Get##attrName() { return mTariffData.attrName.GetValue(); }
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X

/**
 * @brief Standard getter methods for Current attributes
 * 
 * These methods provide access to the Current tariff attributes stored in mCurrentData.
 */
#define X(attrName, attrType) \
    attrType& Get##attrName() { return m##attrName; }
COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

private:
    /*  */
    CommodityTariffPrimaryData mTariffData;

#define X(attrName, attrType) \
    attrType m##attrName;
COMMODITY_TARIFF_CURRENT_ATTRIBUTES
#undef X

/**
 * @brief Standard setter methods for Primary attributes
 **/
#define X(attrName, attrType) \
CHIP_ERROR Set##attrName(const attrType& newValue) { \
    if (mTariffData.attrName.Update(newValue)) { \
        ChipLogProgress(NotSpecified, "EGW-CTC: The attr %s updated", #attrName);\
        MatterReportingAttributeChangeCallback(mEndpointId, CommodityTariff::Id, Attributes::attrName::Id); \
    } \
    return CHIP_NO_ERROR; \
}
COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
#undef X


    void UpdateCurrentAttrs();
    static void MidnightTimerCallback(chip::System::Layer *, void * callbackContext);
protected:
    EndpointId mEndpointId = 0;
};

class CommodityTariffServer : public AttributeAccessInterface,
                              public CommandHandlerInterface
{
public:
    CommodityTariffServer(EndpointId aEndpointId, CommodityTariffDataProvider & aDelegate, BitMask<Feature> aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }

    ~CommodityTariffServer() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

private:
    CommodityTariffDataProvider & mDelegate;
    BitMask<Feature> mFeature;

    // Internal Application API to set attribute values
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;

    void HandleGetDayEntry(HandlerContext & ctx, const Commands::GetDayEntry::DecodableType & commandData);
    void HandleGetTariffComponent(HandlerContext & ctx, const Commands::GetTariffComponent::DecodableType & commandData);
};

} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
