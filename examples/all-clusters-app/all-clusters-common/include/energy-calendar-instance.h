/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/energy-calendar-server/energy-calendar-server.h>
#include <json/value.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyCalendar {

/** @brief
 * CalendarProvider is interface of the Calendar Provider
 */
class CalendarProviderInstance : public CalendarProvider
{
public:
    CalendarProviderInstance() : CalendarProvider(1) {}
    CalendarProviderInstance(EndpointId ep) : CalendarProvider(ep) {}
    ~CalendarProviderInstance();

    void Init(void);

    void SetDefault(void);

    CHIP_ERROR LoadJson(Json::Value & root);

    /* owerride */
    CHIP_ERROR GetDays(EndpointId ep, DataModel::Nullable<Structs::DayStruct::Type> & CurrentDay,
                       DataModel::Nullable<Structs::DayStruct::Type> & NextDay) override;

private:
    uint32_t mDate;
    uint32_t mTime;

    // Attributes contaners allocated memory
    DataModel::Nullable<CharSpan> mName;
    DataModel::List<Structs::CalendarPeriodStruct::Type> mCalendarPeriods;
    DataModel::List<Structs::DayStruct::Type> mSpecialDays;
    Structs::DayStruct::Type mCurrentDay;
    Structs::DayStruct::Type mNextDay;

    DataModel::Nullable<Structs::DayStruct::Type> GetDay(uint32_t date);
    bool CheckSpecialDays();
    bool CheckDay(const Structs::DayStruct::Type & day);
    bool CheckPeriods();

    void JsonToCalendarPeriodStruct(Json::Value & root, Structs::CalendarPeriodStruct::Type & value);
    void JsonToDayStruct(Json::Value & root, Structs::DayStruct::Type & value);
    void JsonToPeakPeriodStruct(Json::Value & root, Structs::PeakPeriodStruct::Type & value);

    void JsonToCalendarPeriodStructList(Json::Value & root, DataModel::List<Structs::CalendarPeriodStruct::Type> & value);
    void JsonToDayStructList(Json::Value & root, DataModel::List<Structs::DayStruct::Type> & value);
    void JsonToTransitionStructList(Json::Value & root, DataModel::List<Structs::TransitionStruct::Type> & value);

    void FreeMemoryDayStruct(Structs::DayStruct::Type & value);
    void FreeMemoryDayStructList(DataModel::List<Structs::DayStruct::Type> & value);
    void FreeMemoryCalendarPeriodStruct(Structs::CalendarPeriodStruct::Type & value);
    void FreeMemoryCalendarPeriodStructList(DataModel::List<Structs::CalendarPeriodStruct::Type> & value);
};

CalendarProviderInstance * GetProvider();

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip
