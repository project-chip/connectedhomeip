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

constexpr uint32_t kOneDay = 24 * 60 * 60;

static TransitionDayOfWeekBitmap GetWeekDate(uint32_t date)
{
    tm calendarTime{};
    time_t dt = date;
    localtime_r(&dt, &calendarTime);
    return (TransitionDayOfWeekBitmap)(calendarTime.tm_wday);
}

static uint32_t GetCurrentDateTime(void)
{
    System::Clock::Timestamp time = System::SystemClock().GetMonotonicTimestamp();
    using cast = std::chrono::duration<std::uint64_t>;
    uint64_t msec = std::chrono::duration_cast< cast >(time).count();

    return static_cast<uint32_t>(msec / 1000);
}

chip::app::Clusters::EnergyCalendar::CalendarProviderInstance::~CalendarProviderInstance()
{
    FreeMemoryCalendarPeriodStructList(mCalendarPeriods);
    FreeMemoryDayStructList(mSpecialDays);
    FreeMemoryDayStruct(mCurrentDay);
    FreeMemoryDayStruct(mNextDay);
    if (!mName.IsNull())
    {
        chip::Platform::MemoryFree((void*)mName.Value().data());
        mName.SetNull();
    }
}

void CalendarProviderInstance::Init(void)
{
    SetDefault();
}

void CalendarProviderInstance::SetDefault(void)
{
    uint32_t date = GetCurrentDateTime();
    uint32_t time = date % kOneDay;
    date -= time;

    Structs::DayStruct::Type day = { .date = {}, 
        .transitions = {
//            { .transitionTime = 0, .PriceTier = 10},
//            { .transitionTime = 1000, .PriceTier = 20}
        },
        .calendarID = chip::Optional<uint32_t>(123) };

    Structs::TransitionStruct::Type *buffer = (Structs::TransitionStruct::Type*)chip::Platform::MemoryCalloc(2, sizeof(Structs::TransitionStruct::Type));

    day.transitions = Span<Structs::TransitionStruct::Type>(buffer, 2);
    buffer[0].transitionTime = 0;
    buffer[0].priceTier.SetValue(10);
    buffer[1].transitionTime = 1000;
    buffer[1].priceTier.SetValue(20);

    Structs::PeakPeriodStruct::Type peak = { .severity = PeakPeriodSeverityEnum::kHigh, .peakPeriod = 100, .startTime = 2000, .endTime = 2200 };

    //DataModel::List<Structs::CalendarPeriodStruct::Type> calendarPeriods = {};
    //DataModel::List<Structs::DayStruct::Type> specialDays = {};

    char *str = (char*)chip::Platform::MemoryAlloc(5);
    memcpy(str, "Test", 5);
    CharSpan nameString(str, 5);
    mName = MakeNullable(static_cast<CharSpan>(nameString));

    SetCommonAttributes(1, mName, 123, 1);
    SetCalendarPeriod(date, mCalendarPeriods);
    SetSpecialDays(mSpecialDays);
    SetCurrentAndNextDays(DataModel::MakeNullable(day), DataModel::MakeNullable(day));
    SetPeakPeriods(DataModel::MakeNullable(peak), DataModel::MakeNullable(peak));
}

CHIP_ERROR CalendarProviderInstance::LoadJson(Json::Value & root)
{
    DataModel::Nullable<uint32_t> calendarID;
    DataModel::Nullable<uint32_t> providerID;
    DataModel::Nullable<uint32_t> eventID;
    DataModel::Nullable<uint32_t> startDate;
    Structs::PeakPeriodStruct::Type currentPeak;
    Structs::PeakPeriodStruct::Type nextPeak;

    Json::Value value = root.get("CalendarID", Json::Value());
    if (!value.empty() && value.isInt())
    {
        calendarID.SetNonNull(value.asInt());
    }

    value = root.get("Name", Json::Value());
    if (!mName.IsNull())
    {
        chip::Platform::MemoryFree((void*)mName.Value().data());
        mName.SetNull();
    }
    if (!value.empty() && value.isString())
    {
        size_t len = value.asString().size()+1;
        char *str = (char*)chip::Platform::MemoryAlloc(len);
        memcpy(str, value.asCString(), len);
        CharSpan nameString(str, len);
        mName = MakeNullable(static_cast<CharSpan>(nameString));
    }

    value = root.get("ProviderID", Json::Value());
    if (!value.empty() && value.isInt())
    {
        providerID.SetNonNull(value.asInt());
    }

    value = root.get("EventID", Json::Value());
    if (!value.empty() && value.isInt())
    {
        eventID.SetNonNull(value.asInt());
    }

    SetCommonAttributes(calendarID, mName, providerID, eventID);

    value = root.get("StartDate", Json::Value());
    if (!value.empty() && value.isInt())
    {
        startDate.SetNonNull(value.asInt());
    }

    value = root.get("CalendarPeriods", Json::Value());
    FreeMemoryCalendarPeriodStructList(mCalendarPeriods);
    if (!value.empty() && value.isArray())
    {
        JsonToCalendarPeriodStructList(value, mCalendarPeriods);
    }
    SetCalendarPeriod(startDate, mCalendarPeriods);

    value = root.get("SpecialDays", Json::Value());
    FreeMemoryDayStructList(mSpecialDays);
    if (!value.empty() && value.isArray())
    {
        JsonToDayStructList(value, mSpecialDays);
    }
    SetSpecialDays(mSpecialDays);

    value = root.get("CurrentDay", Json::Value());
    FreeMemoryDayStruct(mCurrentDay);
    if (!value.empty())
    {
        JsonToDayStruct(value, mCurrentDay);
    }

    value = root.get("NextDay", Json::Value());
    FreeMemoryDayStruct(mNextDay);
    if (!value.empty())
    {
        JsonToDayStruct(value, mNextDay);
    }

    SetCurrentAndNextDays(mCurrentDay, mNextDay);

    value = root.get("CurrentPeak", Json::Value());
    if (!value.empty())
    {
        JsonToPeakPeriodStruct(value, currentPeak);
    }

    value = root.get("NextPeak", Json::Value());
    if (!value.empty())
    {
        JsonToPeakPeriodStruct(value, nextPeak);
    }

    SetPeakPeriods(currentPeak, nextPeak);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CalendarProviderInstance::GetDays(EndpointId ep, DataModel::Nullable<Structs::DayStruct::Type> &CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> &NextDay)
{
    CurrentDay = mCurrentDay;
    NextDay = mNextDay;

    return CHIP_NO_ERROR;
}

DataModel::Nullable<Structs::DayStruct::Type> CalendarProviderInstance::GetDay(uint32_t date)
{
    for (auto & day : mSpecialDays)
    {
        if (day.date.HasValue() && day.date.Value() == date)
        {
            return day;
        }
    }

    TransitionDayOfWeekBitmap week_day = GetWeekDate(date);

    for (auto & period : mCalendarPeriods)
    {
        if (!period.startDate.IsNull() && period.startDate.Value() < date)
            continue;
        for (auto & day : period.days)
        {
            if ((day.daysOfWeek.HasValue() && (day.daysOfWeek.Value().GetField(week_day))) ||
                (day.date.HasValue() && (day.date.Value() == date)))
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

bool CalendarProviderInstance::CheckPeriods()
{
    uint32_t date = 0;
    for (auto & period : mCalendarPeriods)
    {
        if (!period.startDate.IsNull())
        {
            if (period.startDate.Value() < date)
            {
                return false;
            }
            date = period.startDate.Value();
        }

        for (auto & day : period.days)
        {
            if (!CheckDay(day))
            {
                return false;
            }
        }
    }

    return true;
}

bool CalendarProviderInstance::CheckSpecialDays()
{
    uint32_t date = 0;
    for (auto & day : mSpecialDays)
    {
        if (!day.daysOfWeek.HasValue() || !day.calendarID.HasValue() || !CheckDay(day) || day.date.HasValue() || day.date.Value() > date)
        {
            return false;
        }
        date = day.date.Value();
    }

    return true;
}

bool CalendarProviderInstance::CheckDay(const Structs::DayStruct::Type &day)
{
    if ((day.daysOfWeek.HasValue() && day.date.HasValue()) || (!day.daysOfWeek.HasValue() && !day.date.HasValue()))
    {
        return false;
    }

    if (day.transitions.size()== 0)
    {
        return false;
    }

    uint32_t time = 0;
    for (auto & transition : day.transitions)
    {
        auto tr_time = transition.transitionTime;
        if (tr_time < time)
        {
            return false;
        }
        time = tr_time;
    }

    return true;
}

void CalendarProviderInstance::JsonToCalendarPeriodStruct(Json::Value & root, Structs::CalendarPeriodStruct::Type & value)
{
    Json::Value t = root.get("startDate", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.startDate.SetNonNull(t.asInt());
    }
    
    t = root.get("days", Json::Value());
    if (!t.empty() && t.isArray())
    {
        DataModel::List<Structs::DayStruct::Type> *days = (DataModel::List<Structs::DayStruct::Type>*)&value.days;
        JsonToDayStructList(t, *days);
    }
}

void CalendarProviderInstance::JsonToDayStruct(Json::Value & root, Structs::DayStruct::Type & value)
{
    Json::Value t = root.get("date", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.date.SetValue(t.asInt());
    }
    
    t = root.get("daysOfWeek", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.daysOfWeek.SetValue(chip::BitMask<TransitionDayOfWeekBitmap>((uint8_t)t.asInt()));
    }

    t = root.get("transitions", Json::Value());
    if (!t.empty() && t.isArray())
    {
        DataModel::List<Structs::TransitionStruct::Type> *transitions = (DataModel::List<Structs::TransitionStruct::Type>*)&value.transitions;
        JsonToTransitionStructList(t, *transitions);
    }

    t = root.get("calendarID", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.calendarID.SetValue(t.asInt());
    }
}

void CalendarProviderInstance::JsonToPeakPeriodStruct(Json::Value & root, Structs::PeakPeriodStruct::Type & value)
{
    Json::Value t = root.get("severity", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.severity = static_cast<PeakPeriodSeverityEnum>(t.asInt());
    }
    
    t = root.get("peakPeriod", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.peakPeriod = static_cast<uint16_t>(t.asInt());
    }

    t = root.get("startTime", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.startTime.SetNonNull(t.asInt());
    }
    
    t = root.get("endTime", Json::Value());
    if (!t.empty() && t.isInt())
    {
        value.endTime.SetNonNull(t.asInt());
    }
}

void CalendarProviderInstance::JsonToCalendarPeriodStructList(Json::Value & root,
                                                              DataModel::List<Structs::CalendarPeriodStruct::Type> & value)
{
    Structs::CalendarPeriodStruct::Type *buffer = (Structs::CalendarPeriodStruct::Type*)chip::Platform::MemoryCalloc(root.size(), sizeof(Structs::CalendarPeriodStruct::Type));

    value = Span<Structs::CalendarPeriodStruct::Type>(buffer, root.size());

    for (Json::ArrayIndex i = 0; i < root.size(); ++i)
    {
        Json::Value v = root[i];
        JsonToCalendarPeriodStruct(root[i], value[i]);
    }
}

void CalendarProviderInstance::JsonToDayStructList(Json::Value & root, DataModel::List<Structs::DayStruct::Type> & value)
{
    Structs::DayStruct::Type *buffer = (Structs::DayStruct::Type*)chip::Platform::MemoryCalloc(root.size(), sizeof(Structs::DayStruct::Type));

    value = Span<Structs::DayStruct::Type>(buffer, root.size());

    for (Json::ArrayIndex i = 0; i < root.size(); ++i)
    {
        Json::Value v = root[i];
        JsonToDayStruct(root[i], value[i]);
    }
}

void CalendarProviderInstance::JsonToTransitionStructList(Json::Value & root,
                                                          DataModel::List<Structs::TransitionStruct::Type> & value)
{
    Structs::TransitionStruct::Type *buffer = (Structs::TransitionStruct::Type*)chip::Platform::MemoryCalloc(root.size(), sizeof(Structs::TransitionStruct::Type));

    value = Span<Structs::TransitionStruct::Type>(buffer, root.size());

    for (Json::ArrayIndex i = 0; i < root.size(); ++i)
    {
        Json::Value v = root[i];

        Json::Value t = v.get("transitionTime", Json::Value());
        if (!t.empty() && t.isInt())
        {
            value[i].transitionTime = static_cast<uint16_t>(t.asInt());
        }
 
        t = v.get("priceTier", Json::Value());
        if (!t.empty() && t.isInt())
        {
            value[i].priceTier.SetValue(t.asInt());
        }
        
        t = v.get("friendlyCredit", Json::Value());
        if (!t.empty() && t.isBool())
        {
            value[i].friendlyCredit.SetValue(t.asBool());
        }
 
        t = v.get("auxiliaryLoad", Json::Value());
        if (!t.empty() && t.isInt())
        {
            value[i].auxiliaryLoad.SetValue(chip::BitMask<AuxiliaryLoadBitmap>((uint8_t)t.asInt()));
        }
    }
}

void CalendarProviderInstance::FreeMemoryDayStruct(Structs::DayStruct::Type & value)
{
    DataModel::List<const Structs::TransitionStruct::Type> tmp;
    std::swap(tmp, value.transitions);
    chip::Platform::MemoryFree((void*)tmp.data());
}

void CalendarProviderInstance::FreeMemoryDayStructList(DataModel::List<Structs::DayStruct::Type> & value)
{
    for (auto & item : value)
    {
        FreeMemoryDayStruct(item);
    }
    chip::Platform::MemoryFree(value.data());
    value = Span<Structs::DayStruct::Type>();
}

void CalendarProviderInstance::FreeMemoryCalendarPeriodStruct(Structs::CalendarPeriodStruct::Type & value)
{
    for (auto & item : value.days)
    {
        Structs::DayStruct::Type* day = (Structs::DayStruct::Type*)&item;
        FreeMemoryDayStruct(*day);
    }
    
    DataModel::List<const Structs::DayStruct::Type> tmp;
    std::swap(tmp, value.days);
    chip::Platform::MemoryFree((void*)tmp.data());
}

void CalendarProviderInstance::FreeMemoryCalendarPeriodStructList(DataModel::List<Structs::CalendarPeriodStruct::Type> & value)
{
    for (auto & item : value)
    {
        FreeMemoryCalendarPeriodStruct(item);
    }
    chip::Platform::MemoryFree(value.data());
    value = Span<Structs::CalendarPeriodStruct::Type>();
}


static std::unique_ptr<CalendarProviderInstance> gMIDelegate;
static std::unique_ptr<EnergyCalendarServer> gMIInstance;

void emberAfEnergyCalendarClusterInitCallback(chip::EndpointId endpointId)
{
    VerifyOrDie(endpointId == 1); // this cluster is only enabled for endpoint 1.
    VerifyOrDie(!gMIInstance);

    gMIDelegate = std::make_unique<CalendarProviderInstance>(endpointId);
    if (gMIDelegate)
    {
        gMIDelegate->Init();
        
        gMIInstance = std::make_unique<EnergyCalendarServer>(BitMask<Feature, uint32_t>(
            Feature::kPricingTier, Feature::kFriendlyCredit, Feature::kAuxiliaryLoad, Feature::kPeakPeriod));

        gMIInstance->AddCalendarProvider(&(*gMIDelegate));
    }
}

CalendarProviderInstance * chip::app::Clusters::EnergyCalendar::GetProvider()
{
    return &(*gMIDelegate);
}
