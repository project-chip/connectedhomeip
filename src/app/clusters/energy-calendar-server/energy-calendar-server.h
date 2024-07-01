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

#include <list>
#include <string>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyCalendar {

// Spec-defined constants
constexpr uint32_t kMaximumNameLenght            = 12;
constexpr uint32_t kMinimumCalendarPeriodsLength = 1;
constexpr uint32_t kMaximumCalendarPeriodsLength = 4;
constexpr uint32_t kMaximumSpecialDaysLength     = 50;

constexpr int kNumSupportedEndpoints = 1;

/** @brief
 * CalendarProvider is interface of the Calendar Provider
 */
class CalendarProvider
{
public:
    CalendarProvider(EndpointId ep) : _endpoint(ep) {}
    virtual ~CalendarProvider() = default;

    EndpointId endpoint() const { return _endpoint; };

    CHIP_ERROR SetCommonAttributes(DataModel::Nullable<uint32_t> CalendarID, DataModel::Nullable<CharSpan> Name,
                                   DataModel::Nullable<uint32_t> ProviderID, DataModel::Nullable<uint32_t> EventID);

    CHIP_ERROR SetCalendarPeriod(DataModel::Nullable<uint32_t> StartDate,
                                 DataModel::List<Structs::CalendarPeriodStruct::Type> CalendarPeriods);

    CHIP_ERROR SetSpecialDays(DataModel::List<Structs::DayStruct::Type> SpecialDays);

    CHIP_ERROR SetCurrentAndNextDays(DataModel::Nullable<Structs::DayStruct::Type> CurrentDay,
                                     DataModel::Nullable<Structs::DayStruct::Type> NextDay);

    CHIP_ERROR SetPeakPeriods(DataModel::Nullable<Structs::PeakPeriodStruct::Type> CurrentPeakPeriod,
                              DataModel::Nullable<Structs::PeakPeriodStruct::Type> NextPeakPeriod);

    CHIP_ERROR UpdateDays(void);

    virtual CHIP_ERROR GetDays(EndpointId ep, DataModel::Nullable<Structs::DayStruct::Type> & CurrentDay,
                               DataModel::Nullable<Structs::DayStruct::Type> & NextDay) = 0;

    DataModel::Nullable<uint32_t> GetCalendarID(void) { return _calendarID; }
    DataModel::Nullable<CharSpan> GetName(void) { return _name; }
    DataModel::Nullable<uint32_t> GetProviderID(void) { return _providerID; }
    DataModel::Nullable<uint32_t> GetEventID(void) { return _eventID; }
    DataModel::Nullable<uint32_t> GetStartDate(void) { return _startDate; }
    DataModel::List<Structs::CalendarPeriodStruct::Type> GetCalendarPeriods(void) { return _calendarPeriods; }
    DataModel::List<Structs::DayStruct::Type> GetSpecialDays(void) { return _specialDays; }
    DataModel::Nullable<Structs::DayStruct::Type> GetCurrentDay(void) { return _currentDay; }
    DataModel::Nullable<Structs::DayStruct::Type> GetNextDay(void) { return _nextDay; }
    // DataModel::Nullable<Structs::TransitionStruct::Type> GetCurrentTransition(_void);
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> GetCurrentPeakPeriod(void) { return _currentPeakPeriod; }
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> GetNextPeakPeriod(void) { return _nextPeakPeriod; }

private:
    EndpointId _endpoint;

    DataModel::Nullable<uint32_t> _calendarID;
    DataModel::Nullable<CharSpan> _name;
    DataModel::Nullable<uint32_t> _providerID;
    DataModel::Nullable<uint32_t> _eventID;
    DataModel::Nullable<uint32_t> _startDate;
    DataModel::List<Structs::CalendarPeriodStruct::Type> _calendarPeriods;
    DataModel::List<Structs::DayStruct::Type> _specialDays;
    DataModel::Nullable<Structs::DayStruct::Type> _currentDay;
    DataModel::Nullable<Structs::DayStruct::Type> _nextDay;
    // Structs::TransitionStruct::Type _currentTransition;
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> _currentPeakPeriod;
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> _nextPeakPeriod;
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

    CHIP_ERROR AddCalendarProvider(CalendarProvider * provider);

    //(...)
    // Attributes
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    // Commands
    // void InvokeCommand(HandlerContext & ctx) override;

private:
    BitMask<Feature> feature;
    CalendarProvider * calendars[kNumSupportedEndpoints] = { 0 };

    void UpdateCurrentAttrs(void);
    CalendarProvider * GetProvider(EndpointId ep);
    DataModel::Nullable<Structs::TransitionStruct::Type> GetTransition(EndpointId ep);

    static void MidnightTimerCallback(chip::System::Layer *, void * callbackContext);
};

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip
