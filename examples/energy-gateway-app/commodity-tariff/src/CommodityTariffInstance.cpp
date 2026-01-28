/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <CommodityTariffInstance.h>
#include <cinttypes>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::Clusters::Globals::Structs;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::CommodityTariff::Attributes;
using namespace chip::app::Clusters::CommodityTariff::Structs;
using namespace chip::app::CommodityTariffAttrsDataMgmt;
using namespace CommodityTariffConsts;
using namespace chip::System;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

using chip::Protocols::InteractionModel::Status;

using TariffInformationStructType = DataModel::Nullable<TariffInformationStruct::Type>;
using TariffPeriodStructType      = TariffPeriodStruct::Type;
using DayEntryStructType          = DayEntryStruct::Type;
using TariffComponentStructType   = TariffComponentStruct::Type;
using DayStructType               = DayStruct::Type;
using DayPatternStructType        = DayPatternStruct::Type;
using CalendarPeriodStructType    = CalendarPeriodStruct::Type;

// ============================================================================
// Existing Tariff Implementation
// ============================================================================

void CommodityTariffInstance::ScheduleTariffTimeUpdate()
{
    constexpr auto pollInterval = kTimerPollIntervalInSec * 1_s;

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(
        pollInterval, [](System::Layer *, void * context) { static_cast<CommodityTariffInstance *>(context)->TariffTimeUpdCb(); },
        this);
}

void CommodityTariffInstance::CancelTariffTimeUpdate()
{
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().CancelTimer(
        [](System::Layer *, void * context) { static_cast<CommodityTariffInstance *>(context)->TariffTimeUpdCb(); }, this);
}

void CommodityTariffInstance::TariffTimeUpdCb()
{
    uint32_t currentTimestamp = 0;

    if (CHIP_NO_ERROR == GetClock_MatterEpochS(currentTimestamp))
    {
        GetDelegate()->TryToactivateDelayedTariff(currentTimestamp);
    }

    TariffTimeAttrsSync();
    ScheduleTariffTimeUpdate();
}

CHIP_ERROR CommodityTariffInstance::Init()
{
    ScheduleTariffTimeUpdate();
    return Instance::Init();
}

void CommodityTariffInstance::Shutdown()
{
    CancelTariffTimeUpdate();
    Instance::Shutdown();
}

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

CommodityTariffDelegate::CommodityTariffDelegate() {}
