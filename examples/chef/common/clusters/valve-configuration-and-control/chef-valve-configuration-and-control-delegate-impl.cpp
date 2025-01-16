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

#include "chef-valve-configuration-and-control-delegate-impl.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-server.h>
#include <app/util/config.h>

// #ifdef MATTER_DM_PLUGIN_VALVE_CONFIGURATION_AND_CONTROL_SERVER

using namespace chip::app::Clusters::ValveConfigurationAndControl;
using namespace chip::app::Clusters;
using namespace chip::app;

static DelegateImpl * gValveConfigurationAndControlDelegate = nullptr;

DataModel::Nullable<chip::Percent> DelegateImpl::HandleOpenValve(DataModel::Nullable<chip::Percent> level)
{
    if (!level.IsNull())
    {
        ChipLogProgress(DeviceLayer, "HandleOpenValve with level = %d", level.Value());
    }
    else
    {
        ChipLogProgress(DeviceLayer, "HandleOpenValve with level = NULL");
    }
    return level;
}

CHIP_ERROR DelegateImpl::HandleCloseValve()
{
    ChipLogProgress(DeviceLayer, "HandleCloseValve");
    return CHIP_NO_ERROR;
}

void DelegateImpl::HandleRemainingDurationTick(uint32_t duration)
{
    ChipLogProgress(DeviceLayer, "HandleRemainingDurationTick with duration = %d", duration);
}

void ValveConfigurationAndControl::Shutdown()
{
    if (gValveConfigurationAndControlDelegate != nullptr)
    {
        delete gValveConfigurationAndControlDelegate;
        gValveConfigurationAndControlDelegate = nullptr;
    }
}

chip::Protocols::InteractionModel::Status
chefValveConfigurationAndControlWriteCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                              const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;

    if (endpointId != 1)
    {
        ChipLogError(DeviceLayer, "Endpoint %d not supported for ValveConfigurationAndControl cluster.", endpointId);
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::ValveConfigurationAndControl::Attributes::RemainingDuration::Id: {
        uint32_t newVal = 0;
        std::memcpy(&newVal, buffer, sizeof(uint32_t));
        ChipLogProgress(DeviceLayer, "Setting RemainingDuration to %d", newVal);
        DataModel::Nullable<uint32_t> aRemainingDuration(newVal);
        CHIP_ERROR err = SetRemainingDurationExt(endpointId, aRemainingDuration);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Unable to write RemainingDuration");
            ret = chip::Protocols::InteractionModel::Status::Failure;
        }
        break;
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

chip::Protocols::InteractionModel::Status
chefValveConfigurationAndControlReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                             const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                             uint16_t maxReadLength)
{
    chip::Protocols::InteractionModel::Status ret = chip::Protocols::InteractionModel::Status::Success;

    if (endpointId != 1)
    {
        ChipLogError(DeviceLayer, "Endpoint %d not supported for ValveConfigurationAndControl cluster.", endpointId);
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case chip::app::Clusters::ValveConfigurationAndControl::Attributes::RemainingDuration::Id: {

        DataModel::Nullable<uint32_t> duration;
        CHIP_ERROR err = GetRemainingDurationExt(endpointId, duration);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Unable to read RemainingDuration");
            ret = chip::Protocols::InteractionModel::Status::Failure;
            break;
        }
        if (duration.IsNull())
        {
            std::memset(buffer, '\0', maxReadLength);
        }
        else
        {
            uint32_t val = duration.Value();
            std::memcpy(buffer, &val, std::min(maxReadLength, static_cast<uint16_t>(sizeof(uint32_t))));
        }
    }
    break;
    default:
        ret = chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        break;
    }

    return ret;
}

void emberAfValveConfigurationAndControlClusterInitCallback(chip::EndpointId endpointId)
{
    // Currently this cluster delegate handles only one endpoint.
    VerifyOrDieWithMsg(endpointId == 1, DeviceLayer, "ValveConfigurationAndControl cluster is only eabled for endpoint 1.");

    // Ensures this is called only once
    VerifyOrDieWithMsg(gValveConfigurationAndControlDelegate == nullptr, DeviceLayer,
                       "Attempted to call emberAfValveConfigurationAndControlClusterInitCallback more than once.");

    gValveConfigurationAndControlDelegate = new DelegateImpl;

    ValveConfigurationAndControl::SetDefaultDelegate(endpointId, gValveConfigurationAndControlDelegate);
}

// #endif // #ifdef MATTER_DM_PLUGIN_VALVE_CONFIGURATION_AND_CONTROL_SERVER

