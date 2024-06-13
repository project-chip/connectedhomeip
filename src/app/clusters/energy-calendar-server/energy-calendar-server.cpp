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

static TransitionDayOfWeekBitmap GetWeekDate(uint32_t date)
{
    tm calendarTime{};
    localtime_r(&date, &calendarTime);
    return (TransitionDayOfWeekBitmap)(calendarTime.tm_wday);
}

EnergyCalendarContent::EnergyCalendarContent()
{
    endpoint = 0;
}

EnergyCalendarContent::EnergyCalendarContent(EndpointId endpoint)
{
    this->endpoint = endpoint;
}

bool EnergyCalendarServer::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

DataModel::Nullable<Structs::TransitionStruct::Type> EnergyCalendarContent::GetTransition()
{
    if (content[0].CurrentDay.IsNull())
    {
        return DataModel::Nullable<Structs::TransitionStruct::Type>();
    }

    uint32_t date = get_current_utc();
    uint32_t time = date % kOneDay;

    auto transition_iterator = content[0].CurrentDay.Value.Transitions.begin();
    uint32_t next_tr_time = kOneDay;

    Structs::TransitionStruct::Type current;// = content[0].CurrentDay.Value.Transitions[0];

    while (transition_iterator.Next())
    {
        auto & transition = transition_iterator.GetValue();
        auto tr_time = transition.TransitionTime;
        if (tr_time <= time && (CurrentTransition.IsNull() || CurrentTransition.Value().TransitionTime < tr_time))
        {
            CurrentTransition = transition;
        }
        if ((time > tr_time) && (time < next_tr_time))
        {
            next_tr_time = time;
        }
    }
    content[0].CurrentTransition = current;
 //CurrentPeakPeriod.Value() = DayToPeak(day);
    }
}

EnergyCalendarServer::EnergyCalendarServer(EndpointId aEndpointId, Feature aFeature, CalendarProvider *provider) :
        mFeature(aFeature), mProvider(provider)
{
    content[0].endpoint = aEndpointId;
        
    auto CalendarChangingHandler_cb = [this](){ this->CalendarChangingHandler(); };
    auto PeakPeriodsChangingHandler_cb = [this](){ this->PeakPeriodsChangingHandler(); };
    mProvider->SignalsHandlerSet(CalendarChangingHandler_cb, PeakPeriodsChangingHandler_cb);

    uint32_t date = get_current_utc();
    uint32_t time = date % kOneDay;

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(kOneDay - time), MidnightTimerCallback, nullptr);
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
        return aEncoder.Encode(content[endpointIndex].CurrentDay);
    case NextDay::Id:
        return aEncoder.Encode(content[endpointIndex].NextDay);
    case CurrentTransition::Id:
        return aEncoder.Encode(content[endpointIndex].CurrentTransition);
    case CurrentPeakPeriod::Id:
        return aEncoder.Encode(content[endpointIndex].CurrentPeakPeriod);
    case NextPeakPeriod::Id:
        return aEncoder.Encode(content[endpointIndex].NextPeakPeriod);
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

void LockThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

void EnergyCalendarServer::UpdateCurrentAttrs(void)
{
    if (mProvider == nullptr)
        return;

    DataModel::Nullable<Structs::DayStruct::Type> currentDay;    
    DataModel::Nullable<Structs::DayStruct::Type> nextDay;
    mProvider->GetCurrentAndNextDays(content[0].endpoint, currentDay, nextDay);

    // todo stop matter
    LockThreadTask();

    content[0].CurrentDay = currentDay;
    content[0].NextDay = nextDay;

    // todo start matter
    UnlockThreadTask();

    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, CurrentDay::Id);
    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, NextDay::Id);
}

void EnergyCalendarServer::CalendarChangingHandler(void)
{
    if (mProvider == nullptr)
        return;

    if (content[endpointIndex].CalendarID.IsNull()) 
    {
        mProvider->GetCommonAttributes(content[0].endpoint,
            content[0].CalendarID, content[0].Name, content[0].ProviderID, content[0].EventID);
    
        MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, CalendarID::Id);
        MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, Name::Id);
        MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, ProviderID::Id);
        MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, EventID::Id);
    }

    DataModel::Nullable<uint32_t> startDate;
    DataModel::DecodableList<Structs::CalendarPeriod::Type> calendarPeriods;
    DataModel::DecodableList<Structs::DayStruct::Type> specialDays;

    mProvider->GetCalendarPeriod(content[0].endpoint, startDate, calendarPeriods);
    mProvider->GetSpecialDays(content[0].endpoint, specialDays);

    // todo stop matter
    LockThreadTask();

    content[0].StartDate = startDate;
    content[0].CalendarPeriods = calendarPeriods;
    content[0].SpecialDays = specialDays;

    // todo start matter
    UnlockThreadTask();

    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, StartDate::Id);
    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, CalendarPeriods::Id);
    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, SpecialDays::Id);

    UpdateCurrentAttrs();
}

void EnergyCalendarServer::PeakPeriodsChangingHandler(void)
{
    if (mProvider == nullptr)
        return;

    DataModel::Nullable<Structs::PeakPeriodStruct::Type> current;
    DataModel::Nullable<Structs::PeakPeriodStruct::Type> next;

    mProvider->GetPeakPeriods(content[0].endpoint, current, next);

    // todo stop matter
    LockThreadTask();

    content[0].CurrentPeakPeriod = current;
    content[0].NextPeakPeriod = next;
    
    // todo start matter
    UnlockThreadTask();

    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, CurrentPeakPeriod::Id);
    MatterReportingAttributeChangeCallback(content[0].endpoint, EnergyCalendar::Id, NextPeakPeriod::Id);
}

void EnergyCalendarServer::MidnightTimerCallback(chip::System::Layer *, void * callbackContext)
{
    UpdateCurrentAttrs();

    uint32_t date = get_current_utc();
    uint32_t time = date % kOneDay;

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(kOneDay - time), MidnightTimerCallback, nullptr);
}

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterEnergyCalendarPluginServerInitCallback() {}
