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
#include <app/reporting/reporting.h>

#ifdef MATTER_DM_PLUGIN_VALVE_CONFIGURATION_AND_CONTROL_SERVER

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

void DelegateImpl::HandleRemainingDurationTick(uint32_t duration_sec)
{
    ChipLogProgress(DeviceLayer, "HandleRemainingDurationTick with duration_sec = %u", duration_sec);
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
    if (endpointId != 1)
    {
        ChipLogError(DeviceLayer, "Endpoint %d not supported for ValveConfigurationAndControl cluster.", endpointId);
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case Attributes::RemainingDuration::Id: {
        CHIP_ERROR err;
        uint32_t * bufUint32 = reinterpret_cast<uint32_t *>(buffer);
        if (NumericAttributeTraits<uint32_t>::IsNullValue(*bufUint32)) // Max value is interpreted as NULL
        {
            ChipLogProgress(DeviceLayer, "Setting RemainingDuration to NULL.");
            err = SetRemainingDurationNull(endpointId);
        }
        else
        {
            DataModel::Nullable<uint32_t> aRemainingDuration(NumericAttributeTraits<uint32_t>::StorageToWorking(*bufUint32));
            ChipLogProgress(DeviceLayer, "Setting RemainingDuration to %u.", aRemainingDuration.Value());
            err = SetRemainingDuration(endpointId, aRemainingDuration);
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Unable to write RemainingDuration: %" CHIP_ERROR_FORMAT, err.Format());
            return chip::Protocols::InteractionModel::Status::Failure;
        }
        MatterReportingAttributeChangeCallback(endpointId, ValveConfigurationAndControl::Id, Attributes::RemainingDuration::Id);
        break;
    }
    default:
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        return chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status
chefValveConfigurationAndControlReadCallback(chip::EndpointId endpointId, chip::ClusterId clusterId,
                                             const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                             uint16_t maxReadLength)
{
    if (endpointId != 1)
    {
        ChipLogError(DeviceLayer, "Endpoint %d not supported for ValveConfigurationAndControl cluster.", endpointId);
        return chip::Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    chip::AttributeId attributeId = attributeMetadata->attributeId;

    switch (attributeId)
    {
    case Attributes::RemainingDuration::Id: {

        DataModel::Nullable<uint32_t> duration;
        CHIP_ERROR err = GetRemainingDuration(endpointId, duration);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Unable to read RemainingDuration: %" CHIP_ERROR_FORMAT, err.Format());
            return chip::Protocols::InteractionModel::Status::Failure;
        }
        // Max value is interpreted as NULL
        uint32_t val = duration.ValueOr(std::numeric_limits<uint32_t>::max());
        std::memcpy(buffer, &val, std::min<size_t>(maxReadLength, sizeof(uint32_t)));
        break;
    }
    default:
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
        return chip::Protocols::InteractionModel::Status::UnsupportedAttribute;
    }

    return chip::Protocols::InteractionModel::Status::Success;
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

#endif // MATTER_DM_PLUGIN_VALVE_CONFIGURATION_AND_CONTROL_SERVER
