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

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

#include "energy-calendar-instance.h"

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
    time_t dt = date;
    localtime_r(&dt, &calendarTime);
    return (TransitionDayOfWeekBitmap)(calendarTime.tm_wday);
}

void CalendarProviderInstance::Init(void)
{    
}

static uint32_t GetCurrentDateTime(void)
{
    System::Clock::Timestamp time = System::SystemClock().GetMonotonicTimestamp();
    using cast = std::chrono::duration<std::uint64_t>;
    uint64_t msec = std::chrono::duration_cast< cast >(time).count();

    return static_cast<uint32_t>(msec / 1000);
}

void CalendarProviderInstance::SetDefault(void)
{
    uint32_t date = GetCurrentDateTime();
    uint32_t time = date % kOneDay;
    date -= time;

    Structs::DayStruct::Type day = { .date = {}, 
        .transitions = {
            { .transitionTime = 0, .PriceTier = 10},
            { .transitionTime = 1000, .PriceTier = 20}
        },
        .calendarID = chip::Optional<uint32_t>(123) };

    Structs::PeakPeriodStruct::Type peak = { .severity = PeakPeriodSeverityEnum::kHigh, .peakPeriod = 100, .startTime = 2000, .endTime = 2200 };

    DataModel::DecodableList<Structs::CalendarPeriodStruct::Type> calendarPeriods = {};
    DataModel::DecodableList<Structs::DayStruct::Type> specialDays = {};

    SetCommonAttributes(1, "Test", 123, 1);
    SetCalendarPeriod(date, calendarPeriods);
    SetSpecialDays(specialDays);
    SetCurrentAndNextDays(DataModel::MakeNullable(day), DataModel::MakeNullable(day));
    SetPeakPeriods(DataModel::MakeNullable(peak), DataModel::MakeNullable(peak));
}

CHIP_ERROR CalendarProviderInstance::LoadJson(Json::Value & root)
{
    DataModel::Nullable<uint32_t> calendarID;
    DataModel::Nullable<std::string> name;
    DataModel::Nullable<uint32_t> providerID;
    DataModel::Nullable<uint32_t> eventID;
    DataModel::Nullable<uint32_t> startDate;
    DataModel::DecodableList<Structs::DayStruct::Type> specialDays;
    DataModel::DecodableList<Structs::DayStruct::Type> currentDay;
    DataModel::DecodableList<Structs::DayStruct::Type> nextDay;
    DataModel::DecodableList<Structs::PeakPeriodStruct::Type> currentPeak;
    DataModel::DecodableList<Structs::PeakPeriodStruct::Type> nextPeak;

    auto value = root.get("CalendarID", Json::Value());
    if (!value.empty())
    {
        calendarID.SetNonNull(value.asInt());
    }

    auto value = root.get("Name", Json::Value());
    if (!value.empty())
    {
        name.SetNonNull(value.asString());
    }

    auto value = root.get("ProviderID", Json::Value());
    if (!value.empty())
    {
        providerID.SetNonNull(value.asInt());
    }

    auto value = root.get("EventID", Json::Value());
    if (!value.empty())
    {
        eventID.SetNonNull(value.asInt());
    }

    SetCommonAttributes(calendarID, name, providerID, eventID);

    auto value = root.get("StartDate", Json::Value());
    if (!value.empty())
    {
        startDate.SetNonNull(value.asInt());
    }

    if (root.hasValue("CalendarPeriods"))
    {
        Json::FastWriter writer;
        const std::string json = writer.write(root["CalendarPeriods"]);

        TLV::TLVReader reader;
        reader.Init(json);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
        ReturnErrorOnFailure(calendarPeriods.Decode(reader));
    }

    SetCalendarPeriod(date, calendarPeriods);

    if (root.hasValue("SpecialDays"))
    {
        Json::FastWriter writer;
        const std::string json = writer.write(root["SpecialDays"]);

        TLV::TLVReader reader;
        reader.Init(json);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
        ReturnErrorOnFailure(specialDays.Decode(reader));
    }

    SetSpecialDays(specialDays);

    if (root.hasValue("CurrentDay"))
    {
        Json::FastWriter writer;
        const std::string json = writer.write(root["CurrentDay"]);

        TLV::TLVReader reader;
        reader.Init(json);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        ReturnErrorOnFailure(currentDay.Decode(reader));
    }

    if (root.hasValue("NextDay"))
    {
        Json::FastWriter writer;
        const std::string json = writer.write(root["NextDay"]);

        TLV::TLVReader reader;
        reader.Init(json);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        ReturnErrorOnFailure(nextDay.Decode(reader));
    }

    SetCurrentAndNextDays(currentDay, nextDay);

    if (root.hasValue("CurrentPeak"))
    {
        Json::FastWriter writer;
        const std::string json = writer.write(root["CurrentPeak"]);

        TLV::TLVReader reader;
        reader.Init(json);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        ReturnErrorOnFailure(currentPeak.Decode(reader));
    }

    if (root.hasValue("NextPeak"))
    {
        Json::FastWriter writer;
        const std::string json = writer.write(root["NextPeak"]);

        TLV::TLVReader reader;
        reader.Init(json);
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
        ReturnErrorOnFailure(nextPeak.Decode(reader));
    }

    SetPeakPeriods(currentPeak, nextPeak);

    return CHIP_NO_ERROR;
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

Protocols::InteractionModel::Status CalendarProviderInstance::GetDays(EndpointId ep, DataModel::Nullable<Structs::DayStruct::Type> &CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> &NextDay)
{
    CurrentDay = content.currentDay;
    NextDay = content.NextDay;

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
