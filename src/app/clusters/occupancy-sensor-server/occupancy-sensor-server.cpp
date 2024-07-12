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
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace OccupancySensing {

Structs::HoldTimeLimitsStruct::Type * HoldTimeLimitsManager::HoldTimeLimits::GetHoldTimeLimitsStruct(EndpointId endpoint)
{
    size_t endpointIndex                                       = 0;
    Structs::HoldTimeLimitsStruct::Type * holdTimeLimitsStruct = nullptr;
    CHIP_ERROR status                                          = FindHoldTimeLimitsIndex(endpoint, endpointIndex);
    if (CHIP_NO_ERROR == status)
    {
        holdTimeLimitsStruct = &mHoldTimeLimitsStructs[endpointIndex];
    }
    return holdTimeLimitsStruct;
}

CHIP_ERROR
HoldTimeLimitsManager::HoldTimeLimits::SetHoldTimeLimitsStruct(EndpointId endpoint,
                                                               Structs::HoldTimeLimitsStruct::Type & holdTimeLimitsStruct)
{
    VerifyOrReturnError(kInvalidEndpointId != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

    size_t endpointIndex = 0;
    ReturnErrorOnFailure(FindHoldTimeLimitsIndex(endpoint, endpointIndex));

    mHoldTimeLimitsStructs[endpointIndex] = holdTimeLimitsStruct;

    return CHIP_NO_ERROR;
}

/// @brief Returns the index of the HoldTimeLimits associated to an endpoint
/// @param[in] endpoint target endpoint
/// @param[out] endpointIndex index of the corresponding HoldTimeLimits for an endpoint
/// @return CHIP_NO_ERROR or CHIP_ERROR_NOT_FOUND, CHIP_ERROR_INVALID_ARGUMENT if invalid endpoint
CHIP_ERROR HoldTimeLimitsManager::HoldTimeLimits::FindHoldTimeLimitsIndex(EndpointId endpoint, size_t & endpointIndex)
{
    VerifyOrReturnError(kInvalidEndpointId != endpoint, CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t index = emberAfGetClusterServerEndpointIndex(endpoint, OccupancySensing::Id,
                                                          MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT);

    if (index < ArraySize(mHoldTimeLimitsStructs))
    {
        endpointIndex = index;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

HoldTimeLimitsManager HoldTimeLimitsManager::mInstance;

HoldTimeLimitsManager & HoldTimeLimitsManager::Instance()
{
    return mInstance;
}

CHIP_ERROR HoldTimeLimitsManager::Init()
{
    // Prevents re-initializing
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    for (size_t i = 0; i <= kOccupancySensingServerMaxEndpointCount; i++)
    {
        if (emberAfContainsServer(EndpointId(i), OccupancySensing::Id))
        {
            Structs::HoldTimeLimitsStruct::Type holdTimeLimitsInit;

            // Set up some sane initial values for hold time limits structures
            holdTimeLimitsInit.holdTimeMin     = 1;
            holdTimeLimitsInit.holdTimeMax     = 300;
            holdTimeLimitsInit.holdTimeDefault = 10;

            HoldTimeLimitsManager::Instance().SetHoldTimeLimitsStruct(EndpointId(i), holdTimeLimitsInit);
        }
    }

    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

// AttributeAccessInterface
CHIP_ERROR HoldTimeLimitsManager::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::HoldTimeLimits::Id: {

        Structs::HoldTimeLimitsStruct::Type * holdTimeLimitsStruct =
            HoldTimeLimitsManager::Instance().mHoldTimeLimits.GetHoldTimeLimitsStruct(aPath.mEndpointId);
        Structs::HoldTimeLimitsStruct::Type res;
        res.holdTimeMin     = holdTimeLimitsStruct->holdTimeMin;
        res.holdTimeMax     = holdTimeLimitsStruct->holdTimeMax;
        res.holdTimeDefault = holdTimeLimitsStruct->holdTimeDefault;
        return aEncoder.Encode(res);
    }
    default:
        return CHIP_NO_ERROR;
    }
}

Structs::HoldTimeLimitsStruct::Type * HoldTimeLimitsManager::GetHoldTimeLimitsStruct(EndpointId endpoint)
{
    Structs::HoldTimeLimitsStruct::Type * holdTimeLimitsStruct = mHoldTimeLimits.GetHoldTimeLimitsStruct(endpoint);
    return holdTimeLimitsStruct;
}

CHIP_ERROR HoldTimeLimitsManager::SetHoldTimeLimitsStruct(EndpointId endpoint,
                                                          Structs::HoldTimeLimitsStruct::Type & holdTimeLimitsStruct)
{
    ReturnErrorOnFailure(mHoldTimeLimits.SetHoldTimeLimitsStruct(endpoint, holdTimeLimitsStruct));
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

void MatterOccupancySensingPluginServerInitCallback()
{
    CHIP_ERROR err = HoldTimeLimitsManager::Instance().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HoldTimeLimitsManager::Instance().Init() error: %" CHIP_ERROR_FORMAT, err.Format());
    }
}
