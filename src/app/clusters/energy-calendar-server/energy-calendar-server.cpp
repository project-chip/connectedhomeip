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
#include <ThreadStackManager.h>

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

constexpr uint32_t kSecInOneDay = 60*60*24;

static TransitionDayOfWeekBitmap GetWeekDate(uint32_t date)
{
    tm calendarTime{};
    time_t tm = date;
    localtime_r(&tm, &calendarTime);
    return (TransitionDayOfWeekBitmap)(calendarTime.tm_wday);
}

static uint32_t GetCurrentTime(void)
{
    System::Clock::Timestamp time = System::SystemClock().GetMonotonicTimestamp();
    using cast = std::chrono::duration<std::uint64_t>;
    uint64_t msec = std::chrono::duration_cast< cast >(time).count();

    uint32_t sec = (msec / 1000) % kSecInOneDay;
    return sec;
}

void LockThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

Status CalendarProvider::SetCommonAttributes(DataModel::Nullable<uint32_t> CalendarID,
        DataModel::Nullable<std::string> Name,
        DataModel::Nullable<uint32_t> ProviderID,
        DataModel::Nullable<uint32_t> EventID)
{
    bool change;

    change = calendarID != CalendarID;
    if (change)
    {
        calendarID.SetNonNull(*CalendarID);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, CalendarID::Id);
    }

    change = Name != name;
    if (change)
    {
        name.SetNonNull(*Name);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, Name::Id);
    }

    change = ProviderID != providerID;
    if (change)
    {
        providerID.SetNonNull(*ProviderID);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, ProviderID::Id);
    }

    change = EventID != eventID;
    if (change)
    {
        eventID.SetNonNull(*EventID);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, EventID::Id);
    }

    return Status::Success;
}

Status CalendarProvider::SetCalendarPeriod(DataModel::Nullable<uint32_t> StartDate,
        DataModel::DecodableList<Structs::CalendarPeriodStruct::Type> CalendarPeriods)
{
    bool change;

    LockThreadTask();

    change = StartDate != startDate;
    if (change)
    {
        startDate.SetNonNull(*StartDate);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, StartDate::Id);
    }

    calendarPeriods = CalendarPeriods;
    //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, CalendarPeriods::Id);

    UnlockThreadTask();
    return Status::Success;
}

Status CalendarProvider::SetSpecialDays(DataModel::DecodableList<Structs::DayStruct::Type> SpecialDays)
{
    LockThreadTask();

    specialDays = SpecialDays;
    //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, SpecialDays::Id);

    UnlockThreadTask();
    return Status::Success;
}

Status CalendarProvider::SetCurrentAndNextDays(
        DataModel::Nullable<Structs::DayStruct::Type> &CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> &NextDay)
{
    bool change;

    LockThreadTask();

    change = CurrentDay != currentDay;
    if (change)
    {
        currentDay.SetNonNull(*CurrentDay);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, CurrentDay::Id);
    }

    change = NextDay != nextDay;
    if (change)
    {
        nextDay.SetNonNull(*NextDay);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, NextDay::Id);
    }

    UnlockThreadTask();
    return Status::Success;
}
    
Status CalendarProvider::SetPeakPeriods(
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> &CurrentPeakPeriod,
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> &NextPeakPeriod)
{
    bool change;
    LockThreadTask();

    change = CurrentPeakPeriod != currentPeakPeriod;
    if (change)
    {
        currentPeakPeriod.SetNonNull(*CurrentPeakPeriod);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, CurrentPeakPeriod::Id);
    }

    change = NextPeakPeriod != nextPeakPeriod;
    if (change)
    {
        nextPeakPeriod.SetNonNull(*NextPeakPeriod);
        //MatterReportingAttributeChangeCallback(_endpoint, EnergyCalendar::Id, NextPeakPeriod::Id);
    }

    UnlockThreadTask();
    return Status::Success;
}

Status CalendarProvider::UpdateDays(void)
{
    Status status;
    DataModel::Nullable<Structs::DayStruct::Type> currentDay;
    DataModel::Nullable<Structs::DayStruct::Type> nextDay;

    status = GetDays(_endpoint, currentDay, nextDay);
    if (status == Status::Success)
    {
        status = SetCurrentAndNextDays(currentDay, nextDay);
    }

    return status;
}


EnergyCalendarServer::EnergyCalendarServer() :
    AttributeAccessInterface(NullOptional, EnergyCalendar::Id), feature(0), calendars({nullptr})
{
   uint32_t time = GetCurrentTime();

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(kSecInOneDay - time), MidnightTimerCallback, this);
}

EnergyCalendarServer::EnergyCalendarServer(Feature aFeature) :
    AttributeAccessInterface(NullOptional, EnergyCalendar::Id), feature(aFeature), calendars({nullptr})
{
   uint32_t time = GetCurrentTime();

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(kSecInOneDay - time), MidnightTimerCallback, this);
}

bool EnergyCalendarServer::HasFeature(Feature aFeature) const
{
    return feature.Has(aFeature);
}

Status EnergyCalendarServer::AddCalendarProvider(CalendarProvider *provider)
{
    for (int i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (calendars[i] == nullptr)
        {
            calendars[i] = provider;
            return Status::Success;
        }
    }
    return Status::ResourceExhausted;
}

CalendarProvider *EnergyCalendarServer::GetProvider(EndpointId ep)
{
    for (int i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (calendars[i] != nullptr && calendars[i]->endpoint() == ep)
        {
            return calendars[i];
        }
    }
    return nullptr;
}

DataModel::Nullable<Structs::TransitionStruct::Type> EnergyCalendarServer::GetTransition(EndpointId ep)
{
    CalendarProvider *provider = GetProvider(ep);
    if (provider == nullptr || provider->GetCurrentDay().IsNull())
    {
        return DataModel::Nullable<Structs::TransitionStruct::Type>();
    }

    Structs::DayStruct::Type & currentDay = provider->GetCurrentDay().Value();

    uint32_t time = GetCurrentTime();

    auto transition = currentDay.transitions.begin();
    uint32_t next_tr_time = kSecInOneDay;

    const Structs::TransitionStruct::Type *current = nullptr;
    
    while(transition != currentDay.transitions.end())
    {
        auto tr_time = transition->transitionTime;
        if (tr_time <= time && (current == nullptr || current->transitionTime < tr_time))
        {
            current = transition;
        }
        if ((time > tr_time) && (time < next_tr_time))
        {
            next_tr_time = time;
        }
    }
    return DataModel::Nullable<Structs::TransitionStruct::Type>(*current);
}

// AttributeAccessInterface
CHIP_ERROR EnergyCalendarServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CalendarProvider *provider = GetProvider(aPath.mEndpointId);
    switch (aPath.mAttributeId)
    {
    case CalendarID::Id:
        return aEncoder.Encode(provider->GetCalendarID());
    case Name::Id:
        return aEncoder.Encode(provider->GetName());
    case ProviderID::Id:
        return aEncoder.Encode(provider->GetProviderID());
    case EventID::Id:
        return aEncoder.Encode(provider->GetEventID());
    case StartDate::Id:
        return aEncoder.Encode(provider->GetStartDate());
    case CalendarPeriods::Id:
        return aEncoder.Encode(provider->GetCalendarPeriods());
    case SpecialDays::Id:
        return aEncoder.Encode(provider->GetSpecialDays());
    /* Date relative attributes */
    case CurrentDay::Id:
        return aEncoder.Encode(provider->GetCurrentDay());
    case NextDay::Id:
        return aEncoder.Encode(provider->GetNextDay());
    case CurrentTransition::Id:
        return aEncoder.Encode(GetTransition(aPath.mEndpointId));
    case CurrentPeakPeriod::Id:
        return aEncoder.Encode(provider->GetCurrentPeakPeriod());
    case NextPeakPeriod::Id:
        return aEncoder.Encode(provider->GetNextPeakPeriod());
    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(feature);
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

//void EnergyCalendarServer::InvokeCommand(HandlerContext & handlerContext)
//{
//    //using namespace Commands;
//
//    //switch (handlerContext.mRequestPath.mCommandId)
//    //{
//    //}
//    return;
//}

void EnergyCalendarServer::MidnightTimerCallback(chip::System::Layer *, void * callbackContext)
{
    EnergyCalendarServer *instance = (EnergyCalendarServer*)callbackContext;

    for (int i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (instance->calendars[i] != nullptr)
        {
            instance->calendars[i]->UpdateDays();
        }
    }

    uint32_t time = GetCurrentTime();
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(kSecInOneDay - time), MidnightTimerCallback, callbackContext);
}

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterEnergyCalendarPluginServerInitCallback() {}
