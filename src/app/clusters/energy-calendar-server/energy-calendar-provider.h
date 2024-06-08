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
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyCalendar {

/** @brief
 * CalendarProvider is interface of the Calendar Provider
 */
class CalendarProvider
{
public:
   CalendarProvider() : CalendarChanging_cb(null), PeakPeriodsChanging_cb(null) {}

    /* Get methods */

    virtual CHIP_ERROR GetCommonAttributes(EndpointId endpoint, 
        DataModel::Nullable<uint32_t> &CalendarID,
        DataModel::Nullable<std::string> &Name,
        DataModel::Nullable<uint32_t> ProviderID,
        DataModel::Nullable<uint32_t> EventID) = 0;

    virtual CHIP_ERROR GetCalendarPeriod(EndpointId endpoint, 
        DataModel::Nullable<uint32_t> &StartDate,
        DataModel::DecodableList<Structs::CalendarPeriod::Type> &CalendarPeriods) = 0;

    virtual CHIP_ERROR GetSpecialDays(EndpointId endpoint, 
        DataModel::DecodableList<Structs::DayStruct::Type> &SpecialDays) = 0;

    virtual CHIP_ERROR GetCurrentAndNextDays(EndpointId endpoint, 
        DataModel::Nullable<Structs::DayStruct::Type> &CurrentDay,
        DataModel::Nullable<Structs::DayStruct::Type> &NextDay) = 0;
    
    virtual CHIP_ERROR GetPeakPeriods(EndpointId endpoint, 
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> &CurrentPeakPeriod,
        DataModel::Nullable<Structs::PeakPeriodStruct::Type> &NextPeakPeriod) = 0;

    /* Signals */
    void CalendarChanging(void)
    {
        if (CalendarChanging_cb)
            (*CalendarChanging_cb)();
    }

    void PeakPeriodsChanging(void)
    {
        if (PeakPeriodsChanging_cb)
            (*PeakPeriodsChanging_cb)();
    }

    virtual void SignalsHandlerSet(void (*cal_cb)(void), void (*pp_cb)(void))
    {
        CalendarChanging_cb = cal_cb;
        PeakPeriodsChanging_cb = pp_cb;
    };

private:
    void (*CalendarChanging_cb)(void);
    void (*PeakPeriodsChanging_cb)(void);
};

} // namespace EnergyCalendar
} // namespace Clusters
} // namespace app
} // namespace chip
