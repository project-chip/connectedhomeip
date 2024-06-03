/*
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

#include <list>
#include <string>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyCalendar {

// Spec-defined constraints
constexpr uint32_t kMaximumNameLenght         = 12;
constexpr uint32_t kMinimumCalendarPeriodsLength = 1;
constexpr uint32_t kMaximumCalendarPeriodsLength = 4;
constexpr uint32_t kMaximumSpecialDaysLength = 50;

enum class SetAttributes : uint8_t
{
    kSetCurrentDay = 0x01,
    kSetNextDay = 0x02,
    kSetCurrentTransition = 0x04,
    kSetCurrentPeakPeriod = 0x08
    kSetNextPeakPeriod = 0x10

    kSetPeakPeriodStatus = 0x20;
    kSetPeakPeriodStartTime = 0x40;
    kSetPeakPeriodEndTime = 0x80;
};

/** @brief
 * EnergyCalendarContent has the stateful attributes of the cluster: its endpoint
 * and attributes
 */
class EnergyCalendarContent
{
public:
    EndpointId endpoint;

    // Attribute List
    DataModel::Nullable<uint32_t> CalendarID;   /* Attributes::CalendarID:Id */ 
    DataModel::Nullable<std::string> Name;      /* Attributes::Name:Id */ 
    DataModel::Nullable<uint32_t> ProviderID;   /* Attributes::ProviderID:Id */ 
    DataModel::Nullable<uint32_t> EventID;      /* Attributes::EventID:Id */ 
    DataModel::Nullable<uint32_t> StartDate;    /* Attributes::StartDate:Id */ 
    DataModel::DecodableList<Structs::CalendarPeriod::Type> CalendarPeriods; /* Attributes::CalendarPeriods:Id */ 
    DataModel::DecodableList<Structs::DayStruct::Type> SpecialDays; /* Attributes::SpecialDays:Id */ 
    DataModel::Nullable<Structs::DayStruct::Type> CurrentDay; /* Attributes::CurrentDay:Id */ 
    DataModel::Nullable<Structs::DayStruct::Type> NextDay;    /* Attributes::NextDay:Id */ 
    DataModel::Nullable<Structs::TransitionStruct::Type> CurrentTransition; /* Attributes::CurrentTransition:Id */ 
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> CurrentPeakPeriod; /* Attributes::CurrentPeakPeriod:Id */ 
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> NextPeakPeriod; /* Attributes::NextPeakPeriod:Id */ 

    EnergyCalendarContent(EndpointId endpoint);
    EnergyCalendarContent();

    DataModel::Nullable<Structs::DayStruct::Type> GetDay(uint32_t date);
    CHIP_ERROR UpdateDateRelativeAttributes();
    bool CheckPeriods(DataModel::DecodableList<Structs::CalendarPeriod::Type> periods);
    bool CheckSpecialDays(DataModel::DecodableList<Structs::DayStruct::Type> days);
    bool CheckDay(Structs::DayStruct::Type &day);

private:
    uint32_t currentDate;
};

/** @brief
 * EnergyCalendarServer implements both Attributes and Commands
 */
class EnergyCalendarServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    EnergyCalendarServer();
    //(...)
    // Attributes
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Commands
    void InvokeCommand(HandlerContext & ctx) override;

    // Attribute storage
    EnergyCalendarContent content[kNumSupportedEndpoints];
    //EnergyCalendarContent* content = nullptr;
    //(...)
};

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip
