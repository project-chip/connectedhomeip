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
#include <StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyCalendar {

// Spec-defined constants
constexpr uint32_t kMaximumNameLenght         = 12;
constexpr uint32_t kMinimumCalendarPeriodsLength = 1;
constexpr uint32_t kMaximumCalendarPeriodsLength = 4;
constexpr uint32_t kMaximumSpecialDaysLength = 50;

constexpr uint32_t kNumSupportedEndpoints = 1;

/** @brief
 * CalendarProvider is interface of the Calendar Provider
 */
class CalendarProvider
{
public:
    CalendarProvider(EndpointId ep) : _endpoint(ep) {}
    ~CalendarProvider() = default;

    EndpointId endpoint() const { return _endpoint; };

    Protocols::InteractionModel::Status SetCommonAttributes(DataModel::Nullable<uint32_t> CalendarID,
        DataModel::Nullable<std::string> Name, DataModel::Nullable<uint32_t> ProviderID,
        DataModel::Nullable<uint32_t> EventID);

    Protocols::InteractionModel::Status SetCalendarPeriod(DataModel::Nullable<uint32_t> StartDate,
        DataModel::DecodableList<Structs::CalendarPeriodStruct::Type> CalendarPeriods);

    Protocols::InteractionModel::Status SetSpecialDays(DataModel::DecodableList<Structs::DayStruct::Type> SpecialDays);

    Protocols::InteractionModel::Status SetCurrentAndNextDays(DataModel::Nullable<Structs::DayStruct::Type> CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> NextDay);
    
    Protocols::InteractionModel::Status SetPeakPeriods(DataModel::Nullable<Structs::PeakPeriodStruct::Type> CurrentPeakPeriod,
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> NextPeakPeriod);

    Protocols::InteractionModel::Status UpdateDays(void);

    virtual Protocols::InteractionModel::Status GetDays(EndpointId ep, DataModel::Nullable<Structs::DayStruct::Type> &CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> &NextDay) = 0;

    DataModel::Nullable<uint32_t> GetCalendarID(void) { return calendarID; }
    DataModel::Nullable<std::string> GetName(void) { return name; }
    DataModel::Nullable<uint32_t> GetProviderID(void) { return providerID; }
    DataModel::Nullable<uint32_t> GetEventID(void) { return eventID; }
    DataModel::Nullable<uint32_t> GetStartDate(void) { return startDate; }
    DataModel::DecodableList<Structs::CalendarPeriodStruct::Type> GetCalendarPeriods(void) { return calendarPeriods; }
    DataModel::DecodableList<Structs::DayStruct::Type> GetSpecialDays(void) { return specialDays; }
    DataModel::Nullable<Structs::DayStruct::Type> GetCurrentDay(void) { return currentDay; }
    DataModel::Nullable<Structs::DayStruct::Type> GetNextDay(void) { return nextDay; }
    //DataModel::Nullable<Structs::TransitionStruct::Type> GetCurrentTransition(void);
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> GetCurrentPeakPeriod(void) { return currentPeakPeriod; }
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> GetNextPeakPeriod(void) { return nextPeakPeriod; }

private:
    EndpointId _endpoint;

    DataModel::Nullable<uint32_t> calendarID;
    DataModel::Nullable<std::string> name;
    DataModel::Nullable<uint32_t> providerID;
    DataModel::Nullable<uint32_t> eventID;
    DataModel::Nullable<uint32_t> startDate;
    DataModel::DecodableList<Structs::CalendarPeriodStruct::Type> calendarPeriods;
    DataModel::DecodableList<Structs::DayStruct::Type> specialDays;
    DataModel::Nullable<Structs::DayStruct::Type> currentDay;
    DataModel::Nullable<Structs::DayStruct::Type> nextDay;
    //Structs::TransitionStruct::Type currentTransition;
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> currentPeakPeriod;
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> nextPeakPeriod;
};

/** @brief
 * EnergyCalendarServer implements both Attributes and Commands
 */
class EnergyCalendarServer : public AttributeAccessInterface
{
public:
    EnergyCalendarServer();
    EnergyCalendarServer(Feature aFeature);

    bool HasFeature(Feature aFeature) const;

    Protocols::InteractionModel::Status AddCalendarProvider(CalendarProvider *provider);

    //(...)
    // Attributes
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Commands
    //void InvokeCommand(HandlerContext & ctx) override;

private:
    BitMask<Feature> feature;
    CalendarProvider *calendars[kNumSupportedEndpoints];

    void UpdateCurrentAttrs(void);
    CalendarProvider *GetProvider(EndpointId ep);
    DataModel::Nullable<Structs::TransitionStruct::Type> GetTransition(EndpointId ep);

    static void MidnightTimerCallback(chip::System::Layer *, void * callbackContext);
};

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip
