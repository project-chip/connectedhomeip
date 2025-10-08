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
#include <cstdint>
#include <unordered_set>

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

using chip::Protocols::InteractionModel::Status;

using TariffInformationStructType = DataModel::Nullable<TariffInformationStruct::Type>;
using TariffPeriodStructType      = TariffPeriodStruct::Type;
using DayEntryStructType          = DayEntryStruct::Type;
using TariffComponentStructType   = TariffComponentStruct::Type;
using DayStructType               = DayStruct::Type;
using DayPatternStructType        = DayPatternStruct::Type;
using CalendarPeriodStructType    = CalendarPeriodStruct::Type;

CHIP_ERROR CommodityTariffInstance::Init()
{
    return Instance::Init();
}

uint32_t CommodityTariffInstance::GetCurrentTimestamp()
{
    return TimestampNow + TestTimeOverlay;
}

void CommodityTariffInstance::ScheduleTariffTimeUpdate()
{
    DeviceLayer::SystemLayer().StartTimer(
        System::Clock::Milliseconds32(kTimerPollIntervalInSec * 1000),
        [](System::Layer *, void * context) { static_cast<CommodityTariffInstance *>(context)->TariffTimeUpdCb(); }, this);
}

void CommodityTariffInstance::TariffTimeUpdCb()
{
    GetDelegate()->TryToactivateDelayedTariff(TimestampNow);
    TimestampNow += kTimerPollIntervalInSec;
    TariffTimeAttrsSync();
    ScheduleTariffTimeUpdate();
}

void CommodityTariffInstance::ActivateTariffTimeTracking(uint32_t timestamp)
{
    TimestampNow = timestamp;

    ScheduleTariffTimeUpdate();
}

void CommodityTariffInstance::TariffTimeTrackingSetOffset(uint32_t offset)
{
    if (offset)
    {
        TestTimeOverlay += offset;
        return;
    }

    TestTimeOverlay = 0;
}

void CommodityTariffInstance::Shutdown()
{
    Instance::Shutdown();
}

CHIP_ERROR CommodityTariffInstance::AppInit()
{
    return CHIP_NO_ERROR;
}

CommodityTariffDelegate::CommodityTariffDelegate() {}
