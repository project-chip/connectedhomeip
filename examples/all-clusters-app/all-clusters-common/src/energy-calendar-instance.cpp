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

void CalendarProviderInstance::Init(void)
{    
    cluster = new EnergyCalendarServer(content.EndpointId, 0xff, this);
}

void CalendarProviderInstance::SetDefault(void)
{
    uint32_t date = get_current_utc();
    uint32_t time = date % kOneDay;
    date -= time;

    Structs::DayStruct::Type day = { .Date = {}, 
        .Transitions = {
            { .TransitionTime = 0, .PriceTier = 10}
            { .TransitionTime = 1000, .PriceTier = 20}
        },
        .CalendarID = 123 };

    Structs::PeakPeriodStruct::Type peak = { .Severity = 3, .PeakPeriod = 100, .StartTime = 2000, .EndTime = 2200 };

    content.CalendarID.Value() = 1;
    content.Name.Value() = "Test";
    content.ProviderID.Value() = 123;
    content.EventID.Value() = 1;
    content.StartDate.Value() = date;
    //content.CalendarPeriods.Value() = ;
    //content.SpecialDays.Value() = ;
    content.CurrentDay.Value() = day;
    content.NextDay.Value() = day;
    context.CurrentTransition.Value() = day.Transitions[0];
    context.CurrentPeakPeriod.Value() = peak;
    context.NextPeakPeriod.Value() = peak;
}

CHIP_ERROR CalendarProviderInstance::LoadJson(char *name)
{
    return CHIP_IM_GLOBAL_STATUS(NotImplemented);  
}

void CalendarProviderInstance::UpdateDays(uint32_t time)
{

}

void CalendarProviderInstance::MoveToNextDay(void)
{

}

void CalendarProviderInstance::UpdatePeak(uint32_t time)
{

}

void CalendarProviderInstance::MoveToNextPeak(void)
{

}

CHIP_ERROR CalendarProviderInstance::GetCommonAttributes(EndpointId endpoint, 
        DataModel::Nullable<uint32_t> &CalendarID,
        DataModel::Nullable<std::string> &Name,
        DataModel::Nullable<uint32_t> ProviderID,
        DataModel::Nullable<uint32_t> EventID)
{
    CalendarID = content.CalendarID;
    Name = content.Name;
    ProviderID = content.ProviderID;
    EventID = content.EventID;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CalendarProviderInstance::GetCalendarPeriod(EndpointId endpoint, 
        DataModel::Nullable<uint32_t> &StartDate,
        DataModel::DecodableList<Structs::CalendarPeriod::Type> &CalendarPeriods)
{
    StartDate = content.StartDate;
    CalendarPeriods = content.CalendarPeriods;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CalendarProviderInstance::GetSpecialDays(EndpointId endpoint, 
        DataModel::DecodableList<Structs::DayStruct::Type> &SpecialDays)
{
    SpecialDays = content.SpecialDays;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CalendarProviderInstance::GetCurrentAndNextDays(EndpointId endpoint, 
        DataModel::Nullable<Structs::DayStruct::Type> &CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> &NextDay)
{
    CurrentDay = content.currentDay;
    NextDay = content.NextDay;

    return CHIP_NO_ERROR;
}
    
CHIP_ERROR CalendarProviderInstance::GetPeakPeriods(EndpointId endpoint, 
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> &CurrentPeakPeriod,
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> &NextPeakPeriod)
{
    CurrentPeakPeriod = content.CurrentPeakPeriod;
    NextPeakPeriod = content.NextPeakPeriod;

    return CHIP_NO_ERROR;
}


DataModel::Nullable<Structs::DayStruct::Type> CalendarProviderInstance::GetDay(uint32_t date)
{
    auto days_iterator = context.SpecialDays.begin();
    while (days_iterator.Next())
    {
        auto & day = days_iterator.GetValue();
        if (day == currentDate)
        {
            return day;
        }
    }

    TransitionDayOfWeekBitmap week_day = GetWeekDate(date);

    auto period_iterator = context.CalendarPeriods.begin();
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

#if 0
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

#endif

bool CalendarProviderInstance::CheckPeriods(DataModel::DecodableList<Structs::CalendarPeriod::Type> periods)
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

bool CalendarProviderInstance::CheckSpecialDays(DataModel::DecodableList<Structs::DayStruct::Type> days)
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

bool CalendarProviderInstance::CheckDay(const Structs::DayStruct::Type &day)
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
