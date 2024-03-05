/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "DeviceEnergyManagementDelegateImpl.h"
#include "utils.h"
#include <string>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;

using namespace chip::app::DataModel;
using chip::Protocols::InteractionModel::Status;
using CostsList = DataModel::List<const Structs::CostStruct::Type>;


#define DELEGATE_TEST_DATA        // TODO: comment out if not needed

#ifdef  DELEGATE_TEST_DATA
static const Structs::SlotStruct::Type gSlot {
    static_cast<uint32_t>(120),   // minDuration
    static_cast<uint32_t>(230),   // maxDuration
    static_cast<uint32_t>(340),   // defaultDuration
    static_cast<uint32_t>(450),   // elapsedSlotTime
    static_cast<uint32_t>(560),   // remainingSlotTime
    Optional<bool>(true),         // slotIsPauseable
    Optional<uint32_t>(static_cast<uint32_t>(670)),   // minPauseDuration
    Optional<uint32_t>(static_cast<uint32_t>(780)),   // maxPauseDuration
    Optional<uint16_t>{0},         // manufacturerESAState;
    Optional<int64_t>{1},          // nominalPower;
    Optional<int64_t>{2},          // minPower;
    Optional<int64_t>{3},          // maxPower;
    Optional<int64_t>{4},          // nominalEnergy;

    Optional<DataModel::List<const Structs::CostStruct::Type>>{
        DataModel::List<const Structs::CostStruct::Type>(),
    },

    Optional<int64_t> {5},         // minPowerAdjustment;
    Optional<int64_t> {6},         // maxPowerAdjustment;
    Optional<uint32_t>{7},         // minDurationAdjustment;
    Optional<uint32_t>{8}          // maxDurationAdjustment;
    };

// Just for testing. TODO: To be removed ?
static void FillCostStruct(Structs::CostStruct::Type & cost)
{
    static uint8_t inc = 0;
    static CostTypeEnum t = CostTypeEnum::kComfort;

    cost.costType      = t;
    cost.value         = static_cast<int32_t>(222 + inc);
    cost.decimalPoints = static_cast<uint8_t>(111 + inc);
    cost.currency      = Optional<uint16_t> { static_cast<uint16_t>(333 + inc)};

    if (t == CostTypeEnum::kComfort)
    {
        t = CostTypeEnum::kTemperature;
    }
    else
    {
        t = CostTypeEnum::kComfort;
    }
    inc++;
}

// Just for testing. TODO: To be removed ?
static void FillForecast(DataModel::Nullable<Structs::ForecastStruct::Type> & nullableForecast)
{
    nullableForecast.SetNull();
    if (!nullableForecast.IsNull())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Null but nullableForecast.HasValue",  __FUNCTION__);
    }
    else
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Null & nullableForecast.NullValue",  __FUNCTION__);
    }

    Structs::ForecastStruct::Type forecast;

    forecast.forecastId        = static_cast<uint16_t>(1234);

    forecast.activeSlotNumber.SetNonNull<uint16_t>(2345);
    forecast.startTime         = static_cast<uint32_t>(34567);
    forecast.endTime           = static_cast<uint32_t>(45678);
    forecast.earliestStartTime = Optional<DataModel::Nullable<uint32_t>> {DataModel::Nullable<uint32_t>{56789}};     // Optional<DataModel::Nullable<uint32_t>>
    forecast.latestEndTime     = Optional<uint32_t> (static_cast<uint32_t>(67890));
    forecast.isPauseable       = true;

    uint32_t slotCount = 2;
    Structs::SlotStruct::Type  * slots = new Structs::SlotStruct::Type[slotCount];
    slots[0] = gSlot;

    slots[1].minDuration       = gSlot.minDuration + 1;
    slots[1].maxDuration       = gSlot.maxDuration + 1;
    slots[1].defaultDuration   = gSlot.defaultDuration + 1;
    slots[1].elapsedSlotTime   = gSlot.elapsedSlotTime + 1;
    slots[1].remainingSlotTime = gSlot.remainingSlotTime + 1;
    slots[1].slotIsPauseable   = Optional<bool>( !gSlot.slotIsPauseable.ValueOr(false));
    slots[1].minPauseDuration  = Optional<uint32_t>(gSlot.minPauseDuration.ValueOr(0) + 1);
    slots[1].maxPauseDuration  = Optional<uint32_t>(gSlot.maxPauseDuration.ValueOr(0) + 1);
    slots[1].manufacturerESAState = Optional<uint16_t>{90};
    slots[1].nominalPower       = Optional<int64_t>{91};
    slots[1].minPower           = Optional<int64_t>{92};
    slots[1].maxPower           = Optional<int64_t>{93};
    slots[1].nominalEnergy      = Optional<int64_t>{94};

    // Optional<DataModel::List<const Structs::CostStruct::Type>> costs;
    slots[1].costs = Optional<DataModel::List<const Structs::CostStruct::Type>>{
                     DataModel::List<const Structs::CostStruct::Type>()};

    slots[1].minPowerAdjustment = Optional<int64_t>{95};
    slots[1].maxPowerAdjustment = Optional<int64_t>{96};
    slots[1].minDurationAdjustment = Optional<uint32_t>{97};
    slots[1].maxDurationAdjustment = Optional<uint32_t>{98};

    forecast.slots = DataModel::List<const Structs::SlotStruct::Type>(slots, slotCount);

    uint32_t costsCount = 2;
    Structs::CostStruct::Type  * costs1 = new Structs::CostStruct::Type[costsCount];
    Structs::CostStruct::Type  * costs2 = new Structs::CostStruct::Type[costsCount];

    FillCostStruct(costs1[0]);
    FillCostStruct(costs1[1]);
    FillCostStruct(costs2[0]);
    FillCostStruct(costs2[1]);

    slots[0].costs = Optional<CostsList> {CostsList(costs1, costsCount)};
    slots[1].costs = Optional<CostsList> {CostsList(costs2, costsCount)};

    nullableForecast = MakeNullable(forecast);
}

// Prepare for Test Procedure
void ForecastTestSetup_TP3b(DataModel::Nullable<Structs::ForecastStruct::Type> & nullableForecast)
{
    if (nullableForecast.IsNull())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Null but nullableForecast.HasValue",  __FUNCTION__);
        return;
    }

    Structs::ForecastStruct::Type & forecast = nullableForecast.Value();
    ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d xxForecast.startTime = %d", __LINE__ , forecast.startTime);
    ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d xxForecast.endTime = %d", __LINE__, forecast.endTime );
    ChipLogProgress(Support, "[StartTimeAdjustment-handle] L-%d xxForecast.isPauseable = %s", __LINE__, forecast.isPauseable? "T":"F" );

    uint32_t chipEpoch = 0;

    CHIP_ERROR ce = UtilsGetEpochTS(chipEpoch);
    ChipLogProgress(Support, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ ce=%s  epoch = %d",
                         (ce != CHIP_NO_ERROR)? "Err":"Good", chipEpoch);

#if 1
    forecast.startTime         = static_cast<uint32_t>(chipEpoch);       // planned start time, in UTC, for the entire Forecast.
    // earliest start time, in UTC, that the entire Forecast can be shifted to. null value indicates that it can be started immediately.
    forecast.earliestStartTime = Optional<DataModel::Nullable<uint32_t>> {DataModel::Nullable<uint32_t>{chipEpoch}}; 

    forecast.endTime           = static_cast<uint32_t>(chipEpoch * 3);   // planned end time, in UTC, for the entire Forecast.
    forecast.latestEndTime     = Optional<uint32_t> (static_cast<uint32_t>(chipEpoch * 3));  // latest end time, in UTC, for the entire Forecast
#endif
}

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

//DeviceEnergyManagementDelegateImpl::DeviceEnergyManagementDelegateImpl() 
DeviceEnergyManagementDelegate::DeviceEnergyManagementDelegate() 
{
    BitMask<DeviceEnergyManagement::Feature> FeatureMap;
    FeatureMap.Set(DeviceEnergyManagement::Feature::kForecastAdjustment);
    //Status status = DeviceEnergyManagement::Attributes::FeatureMap::Set(mEndpointId, FeatureMap.Raw());

#if 0       // Set  UserOptOut in  ESAState for testing ?
    {
        DeviceEnergyManagement::ESAStateEnum esaStatus = DeviceEnergyManagement::ESAStateEnum::kUserOptOut;
        s = DeviceEnergyManagement::Attributes::ESAState::Set(endpoint, esaStatus);
        if (EMBER_ZCL_STATUS_SUCCESS != s)
        {
            ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Failed to set() ESAStateEnum::kUserOptOut",  __FUNCTION__);
            goto DEPARTURES;
        }
    }
#endif

    FillForecast(mForecast);
    ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Enabled Feature ForecastAdjustment",  __FUNCTION__);

    return;
}
#endif
}
}
}
}
