/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "energy-calendar-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyCalendar;
using namespace chip::app::Clusters::EnergyCalendar::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyCalendar {

constexpr uint32_t kOneDay = 24 * 60 * 60;

static TransitionDayOfWeekBitmap GetWeekDate(uint32_t date)
{
    tm calendarTime{};
    localtime_r(&date, &calendarTime);
    return (TransitionDayOfWeekBitmap)(calendarTime.tm_wday);
}

EnergyCalendarContent::EnergyCalendarContent()
{
    endpoint = 0;
    currentDate = 0;
}

EnergyCalendarContent::EnergyCalendarContent(EndpointId endpoint)
{
    this->endpoint = endpoint;
    currentDate = 0;
}

DataModel::Nullable<Structs::DayStruct::Type> EnergyCalendarContent::GetDay(uint32_t date)
{
    auto days_iterator = SpecialDays.begin();
    while (days_iterator.Next())
    {
        auto & day = days_iterator.GetValue();
        if (day == currentDate)
        {
            return day;
        }
    }

    TransitionDayOfWeekBitmap week_day = GetWeekDate(date);

    auto period_iterator = CalendarPeriods.begin();
    while (period_iterator.Next())
    {
        auto & period = period_iterator.GetValue();
        if (period.StartDate < date)
            continue;
        auto days_iterator = period.Days.begin();
        while (days_iterator.Next())
        {
            auto & day = days_iterator.GetValue();
            if ((day.DaysOfWeek & week_day != 0) || (day.Date == date))
            {
                return day;
            }
        }
    }

    return DataModel::Nullable<Structs::DayStruct::Type>();
}

CHIP_ERROR EnergyCalendarContent::UpdateDateRelativeAttributes()
{
    uint32_t date = get_current_utc();
    //uint32_t time = date % kOneDay;
    //date -= time;

    if ((currentDate <= date) && (date < currentDate + kOneDay))
    {
        return CHIP_NO_ERROR;
    }

    currentDate = date;
    CurrentDay = nullptr;
    NextDay = nullptr;
    CurrentTransition = nullptr;
    //CurrentPeakPeriod = nullptr;
    //NextPeakPeriod = nullptr;

    CurrentDay = EnergyCalendarContent::GetDay(date);
    if (!CurrentDay.IsNull())
    {
        auto &day = CurrentDay.Value();
        currentDate = day.Date;
        auto transition_iterator = day.Transitions.begin();
        uint32_t next_tr_time = kOneDay;

        while (transition_iterator.Next())
        {
            auto & transition = transition_iterator.GetValue();
            auto tr_time = transition.TransitionTime;
            if (tr_time <= time && (CurrentTransition == nullptr || CurrentTransition.TransitionTime < tr_time))
            {
                CurrentTransition = transition;
            }
            if ((time > tr_time) && (time < next_tr_time))
            {
                next_tr_time = time;
            }
        }

        //CurrentPeakPeriod.Value() = DayToPeak(day);
    }

    NextDay = EnergyCalendarContent::GetDay(date + kOneDay);
    //if (!NextDay.IsNull())
    //{
    //    NexytPeakPeriod.Value() = DayToPeak(NextDay.Value());
    //}

    return CHIP_NO_ERROR;
}

bool EnergyCalendarContent::CheckPeriods(DataModel::DecodableList<Structs::CalendarPeriod::Type> periods)
{
    uint32_t date = 0;
    auto period_iterator = periods.begin();
    while (period_iterator.Next())
    {
        auto & period = period_iterator.GetValue();
        if (period.StartDate)
        {
            if (period.StartDate < date)
            {
                return false;
            }
            date = period.StartDate;
        }

        auto days_iterator = period.Days.begin();
        while (days_iterator.Next())
        {
            auto & day = days_iterator.GetValue();
            if (!CheckDay(day))
            {
                return false;
            }
        }
    }

    return true;
}

bool EnergyCalendarContent::CheckSpecialDays(DataModel::DecodableList<Structs::DayStruct::Type> days)
{
    uint32_t date = 0;
    while (days_iterator.Next())
    auto days_iterator = days.begin();
    {
        auto & day = days_iterator.GetValue();
        if (day.DaysOfWeek || day.CalendarID || !CheckDay(day) || !(day.date) || day.date > date)
        {
            return false;
        }
        date = day.date;
    }

    return true;
}

bool EnergyCalendarContent::CheckDay(const Structs::DayStruct::Type &day)
{
    if (day.DaysOfWeek && day.Date) || (!day.DaysOfWeek && !day.Date))
    {
        return false;
    }

    if (size(day.Transition)== 0)
    {
        return false;
    }

    uint32_t time = 0;
    auto transition_iterator day.Transitions.begin();
    while (transition_iterator.Next())
    {
        auto & transition = transition_iterator.GetValue();
        auto tr_time = transition.TransitionTime;
        if (tr_time < time)
        {
            return false;
        }
        time = tr_time;
    }

    return true;
}

bool EnergyCalendarServer::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

DataModel::Nullable<Structs::PeakPeriodStruct::Type> EnergyCalendarServer::DayToPeak(DataModel::Nullable<Structs::DayStruct::Type> &day)
{
    if (GetPeakPeriod == nullptr || day.IsNull) {
        return DataModel::Nullable<Structs::PeakPeriodStruct::Type>();
    }
    return (*GetPeakPeriod)(day.Value().Date);
}

// AttributeAccessInterface
CHIP_ERROR EnergyCalendarServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case CalendarID::Id:
        return aEncoder.Encode(content[endpointIndex].CalendarID);
    case Name::Id:
        return aEncoder.Encode(content[endpointIndex].Name);
    case ProviderID::Id:
        return aEncoder.Encode(content[endpointIndex].ProviderID);
    case EventID::Id:
        return aEncoder.Encode(content[endpointIndex].EventID);
    case StartDate::Id:
        return aEncoder.Encode(content[endpointIndex].StartDate);
    case TimeReference::Id:
        return aEncoder.Encode(content[endpointIndex].TimeReference);
    case CalendarPeriods::Id:
        return aEncoder.Encode(content[endpointIndex].CalendarPeriods);
    case SpecialDays::Id:
        return aEncoder.Encode(content[endpointIndex].SpecialDays);
    /* Date relative attributes */
    case CurrentDay::Id:
        UpdateDateRelativeAttributes();
        return aEncoder.Encode(content[endpointIndex].CurrentDay);
    case NextDay::Id:
        UpdateDateRelativeAttributes();
        return aEncoder.Encode(content[endpointIndex].NextDay);
    case CurrentTransition::Id:
        UpdateDateRelativeAttributes();
        return aEncoder.Encode(content[endpointIndex].CurrentTransition);
    case CurrentPeakPeriod::Id:
    {
        UpdateDateRelativeAttributes();
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> peak = DayToPeak(content[endpointIndex].CurrentDay);
        return aEncoder.Encode(peak);
        //return aEncoder.Encode(content[endpointIndex].CurrentPeakPeriod);
    }
    case NextPeakPeriod::Id:
    {
        UpdateDateRelativeAttributes();
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> peak = DayToPeak(content[endpointIndex].NextDay);
        return aEncoder.Encode(peak);
        //return aEncoder.Encode(content[endpointIndex].NextPeakPeriod);
    }
    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyCalendarServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    default:
        // Unknown attribute; return error.  None of the other attributes for
        // this cluster are writable, so should not be ending up in this code to         
        // start with.
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

void EnergyCalendarServer::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    //switch (handlerContext.mRequestPath.mCommandId)
    //{
    //}
    return;
}

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterEnergyCalendarPluginServerInitCallback() {}
