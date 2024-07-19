/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#include "occupancy-sensor-server.h"
#include "occupancy-hal.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/data-model/Encode.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {

namespace {
Structs::HoldTimeLimitsStruct::Type
    sHoldTimeLimitsStructs[MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];

uint16_t sHoldTime[MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
} // namespace

CHIP_ERROR OccupancySensingAttrAccess::Init()
{
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void OccupancySensingAttrAccess::Shutdown()
{
    unregisterAttributeAccessOverride(this);
}

CHIP_ERROR OccupancySensingAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == app::Clusters::OccupancySensing::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case Attributes::HoldTime::Id: {

        uint16_t * holdTime = GetHoldTimeForEndpoint(aPath.mEndpointId);

        if (holdTime == nullptr)
        {
            return CHIP_ERROR_NOT_FOUND;
        }

        return aEncoder.Encode(*holdTime);
    }
    case Attributes::HoldTimeLimits::Id: {

        Structs::HoldTimeLimitsStruct::Type * holdTimeLimitsStruct = GetHoldTimeLimitsForEndpoint(aPath.mEndpointId);

        if (holdTimeLimitsStruct == nullptr)
        {
            return CHIP_ERROR_NOT_FOUND;
        }

        return aEncoder.Encode(*holdTimeLimitsStruct);
    }
    default:
        return CHIP_NO_ERROR;
    }

    return CHIP_NO_ERROR;
}

bool OccupancySensingAttrAccess::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

Structs::HoldTimeLimitsStruct::Type * GetHoldTimeLimitsForEndpoint(EndpointId endpoint)
{
    auto index = emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::OccupancySensing::Id,
                                                      MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    if (index >= ArraySize(sHoldTimeLimitsStructs))
    {
        ChipLogError(NotSpecified, "Internal error: invalid/unexpected hold time limits index.");
        return nullptr;
    }
    return &sHoldTimeLimitsStructs[index];
}

CHIP_ERROR SetHoldTimeLimits(EndpointId endpointId, const Structs::HoldTimeLimitsStruct::Type & holdTimeLimits)
{

    VerifyOrReturnError(kInvalidEndpointId != endpointId, CHIP_ERROR_INVALID_ARGUMENT);

    Structs::HoldTimeLimitsStruct::Type * holdTimeLimitsForEndpoint = GetHoldTimeLimitsForEndpoint(endpointId);
    VerifyOrReturnError(holdTimeLimitsForEndpoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    holdTimeLimitsForEndpoint->holdTimeMin     = holdTimeLimits.holdTimeMin;
    holdTimeLimitsForEndpoint->holdTimeMax     = holdTimeLimits.holdTimeMax;
    holdTimeLimitsForEndpoint->holdTimeDefault = holdTimeLimits.holdTimeDefault;

    MatterReportingAttributeChangeCallback(endpointId, OccupancySensing::Id, Attributes::HoldTimeLimits::Id);

    return CHIP_NO_ERROR;
}

uint16_t * GetHoldTimeForEndpoint(EndpointId endpoint)
{
    auto index = emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::OccupancySensing::Id,
                                                      MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index == kEmberInvalidEndpointIndex)
    {
        return nullptr;
    }

    if (index >= ArraySize(sHoldTimeLimitsStructs))
    {
        ChipLogError(NotSpecified, "Internal error: invalid/unexpected hold time index.");
        return nullptr;
    }
    return &sHoldTime[index];
}

CHIP_ERROR SetHoldTime(EndpointId endpointId, const uint16_t & holdTime)
{
    VerifyOrReturnError(kInvalidEndpointId != endpointId, CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t * holdTimeForEndpoint = GetHoldTimeForEndpoint(endpointId);
    VerifyOrReturnError(holdTimeForEndpoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    *holdTimeForEndpoint = holdTime;

    MatterReportingAttributeChangeCallback(endpointId, OccupancySensing::Id, Attributes::HoldTime::Id);

    return CHIP_NO_ERROR;
}

} // namespace OccupancySensing
} // namespace Clusters
} // namespace app
} // namespace chip

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OccupancySensing;

//******************************************************************************
// Plugin init function
//******************************************************************************
void emberAfOccupancySensingClusterServerInitCallback(EndpointId endpoint)
{
    auto deviceType = halOccupancyGetSensorType(endpoint);

    chip::BitMask<OccupancySensorTypeBitmap> deviceTypeBitmap = 0;
    switch (deviceType)
    {
    case HAL_OCCUPANCY_SENSOR_TYPE_PIR:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kPir);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kPir);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_ULTRASONIC:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kUltrasonic);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kUltrasonic);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PIR_AND_ULTRASONIC:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kPir);
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kUltrasonic);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kPIRAndUltrasonic);
        break;

    case HAL_OCCUPANCY_SENSOR_TYPE_PHYSICAL:
        deviceTypeBitmap.Set(OccupancySensorTypeBitmap::kPhysicalContact);
        Attributes::OccupancySensorType::Set(endpoint, OccupancySensorTypeEnum::kPhysicalContact);
        break;

    default:
        break;
    }
    Attributes::OccupancySensorTypeBitmap::Set(endpoint, deviceTypeBitmap);
}

//******************************************************************************
// Notification callback from the HAL plugin
//******************************************************************************
void halOccupancyStateChangedCallback(EndpointId endpoint, HalOccupancyState occupancyState)
{
    chip::BitMask<OccupancyBitmap> mappedOccupancyState;
    if (occupancyState & HAL_OCCUPANCY_STATE_OCCUPIED)
    {
        mappedOccupancyState.Set(OccupancyBitmap::kOccupied);
        ChipLogProgress(Zcl, "Occupancy detected");
    }
    else
    {
        ChipLogProgress(Zcl, "Occupancy no longer detected");
    }

    Attributes::Occupancy::Set(endpoint, occupancyState);
}

HalOccupancySensorType __attribute__((weak)) halOccupancyGetSensorType(EndpointId endpoint)
{
    return HAL_OCCUPANCY_SENSOR_TYPE_PIR;
}

void MatterOccupancySensingPluginServerInitCallback() {}
