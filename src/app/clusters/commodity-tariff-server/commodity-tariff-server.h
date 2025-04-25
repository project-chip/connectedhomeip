/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {

// Centralized attribute declaration (Primary attrs)
#define COMMODITY_TARIFF_PRIMARY_ATTRIBUTES_STUBS \
    /*X(TariffInfo,                 DataModel::Nullable<Structs::TariffInformationStruct::Type>)*/ \
    X(TariffUnit,                   DataModel::Nullable<Globals::TariffUnitEnum>) \
    X(StartDate,                    DataModel::Nullable<uint32_t>) \
    X(DefaultRandomizationOffset,   DataModel::Nullable<int16_t>) \
    X(DefaultRandomizationType,     DataModel::Nullable<DayEntryRandomizationTypeEnum>)
    /*X(CalendarPeriods,              DataModel::List<Structs::CalendarPeriodStruct::Type>) \
    X(DayPatterns,                  DataModel::List<Structs::DayPatternStruct::Type>) \
    X(IndividualDays,               DataModel::List<Structs::DayStruct::Type>) \
    X(DayEntries,                 DataModel::List<Structs::DayEntryStruct::Type>) \
    X(TariffPeriods,                DataModel::List<Structs::TariffPeriodStruct::Type>) \
    X(TariffComponents,             DataModel::List<Structs::TariffComponentStruct::Type>)*/

#define COMMODITY_TARIFF_PRIMARY_ATTRIBUTES_DEV \
    X(TariffInfo,                   DataModel::Nullable<Structs::TariffInformationStruct::Type>) \
    X(TariffPeriods,                DataModel::List<Structs::TariffPeriodStruct::Type>) \
    X(DayEntries,                   DataModel::List<Structs::DayEntryStruct::Type>) \
    X(TariffComponents,             DataModel::List<Structs::TariffComponentStruct::Type>) \
    X(IndividualDays,               DataModel::List<Structs::DayStruct::Type>) \
    X(DayPatterns,                  DataModel::List<Structs::DayPatternStruct::Type>) \
    X(CalendarPeriods,              DataModel::List<Structs::CalendarPeriodStruct::Type>)

#define COMMODITY_TARIFF_PRIMARY_ATTRIBUTES \
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES_STUBS \
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES_DEV

// Centralized attribute declaration (Current attrs)
#define COMMODITY_TARIFF_CURRENT_ATTRIBUTES \
    X(CurrentDay,                   DataModel::Nullable<Structs::DayStruct::Type>) \
    X(NextDay,                      DataModel::Nullable<Structs::DayStruct::Type>) \
    X(CurrentDayEntry,              DataModel::Nullable<Structs::DayEntryStruct::Type>) \
    X(NextDayEntry,                 DataModel::Nullable<Structs::DayEntryStruct::Type>) \
    X(CurrentDayEntryDate,          DataModel::Nullable<uint32_t>) \
    X(NextDayEntryDate,             DataModel::Nullable<uint32_t>) \
    X(CurrentTariffComponents,      DataModel::List<Structs::TariffComponentStruct::Type>) \
    X(NextTariffComponents,         DataModel::List<Structs::TariffComponentStruct::Type>)

class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }


    virtual Protocols::InteractionModel::Status GetDayEntryById(DataModel::Nullable<uint32_t> aDayEntryId,
                                                                Structs::DayEntryStruct::Type & aDayEntry) = 0;

    virtual Protocols::InteractionModel::Status GetTariffComponentInfoById(DataModel::Nullable<uint32_t>  aTariffComponentId,
                                                                           DataModel::Nullable<chip::CharSpan> & label,
                                                                           DataModel::List<const uint32_t> & dayEntryIDs,
                                                                           Structs::TariffComponentStruct::Type & aTariffComponent) = 0;

    // ------------------------------------------------------------------
#if 0
    // Attribute  getters
    virtual DataModel::Nullable<Structs::TariffInformationStruct::Type> & GetTariffInfo()        = 0;
    virtual DataModel::Nullable<Globals::TariffUnitEnum> & GetTariffUnit()                       = 0;
    virtual DataModel::Nullable<uint32_t> & GetStartDate()                                       = 0;
    virtual DataModel::Nullable<int16_t> & GetDefaultRandomizationOffset()                       = 0;
    virtual DataModel::Nullable<DayEntryRandomizationTypeEnum> & GetDefaultRandomizationType()   = 0;
    virtual DataModel::List<Structs::CalendarPeriodStruct::Type> & GetCalendarPeriods()          = 0;
    virtual DataModel::List<Structs::DayPatternStruct::Type> & GetDayPatterns()                  = 0;
    virtual DataModel::List<Structs::DayStruct::Type> & GetIndividualDays()                      = 0;
    virtual DataModel::List<Structs::DayEntryStruct::Type> & GetDayEntries()                     = 0;
    virtual DataModel::List<Structs::TariffPeriodStruct::Type> & GetTariffPeriods()              = 0;
    virtual DataModel::List<Structs::TariffComponentStruct::Type> & GetTariffComponents()        = 0;
    virtual DataModel::Nullable<Structs::DayStruct::Type> & GetCurrentDay()                      = 0;
    virtual DataModel::Nullable<Structs::DayStruct::Type> & GetNextDay()                         = 0;
    virtual DataModel::Nullable<Structs::DayEntryStruct::Type> & GetCurrentDayEntry()            = 0;
    virtual DataModel::Nullable<Structs::DayEntryStruct::Type> & GetNextDayEntry()               = 0;
    virtual DataModel::Nullable<uint32_t> & GetCurrentDayEntryDate()                             = 0;
    virtual DataModel::Nullable<uint32_t> & GetNextDayEntryDate()                                = 0;
    virtual DataModel::List<Structs::TariffComponentStruct::Type> & GetCurrentTariffComponents() = 0;
    virtual DataModel::List<Structs::TariffComponentStruct::Type> & GetNextTariffComponents()    = 0;
#else
    #define X(attrName, attrType) \
        virtual attrType& Get##attrName() = 0;
    COMMODITY_TARIFF_PRIMARY_ATTRIBUTES
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
    #undef X
#endif

private:
    // Generate setters
    #define X(attrName, attrType) \
        virtual CHIP_ERROR Set##attrName(const attrType& newValue) { \
            return CHIP_NO_ERROR; \
        }
    COMMODITY_TARIFF_CURRENT_ATTRIBUTES
    #undef X

    // Helper function for change reporting
    virtual void ReportAttributeChange(uint16_t attributeId) = 0;

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface,
                              public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

private:
    Delegate & mDelegate;
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
