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

typedef uint32_t epoch_s;

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
    // Get attribute methods
    virtual DataModel::Nullable<Structs::TariffInformationStruct::Type> & GetTariffInfo()        = 0;
    virtual Globals::TariffUnitEnum GetTariffUnit()                                              = 0;
    virtual DataModel::Nullable<epoch_s> GetStartDate()                                          = 0;
    virtual DataModel::Nullable<int16_t> GetDefaultRandomizationOffset()                         = 0;
    virtual DataModel::Nullable<DayEntryRandomizationTypeEnum> GetDefaultRandomizationType()     = 0;
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
    virtual DataModel::Nullable<epoch_s> GetCurrentDayEntryDate()                                = 0;
    virtual DataModel::Nullable<epoch_s> GetNextDayEntryDate()                                   = 0;
    virtual DataModel::List<Structs::TariffComponentStruct::Type> & GetCurrentTariffComponents() = 0;
    virtual DataModel::List<Structs::TariffComponentStruct::Type> & GetNextTariffComponents()    = 0;

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
