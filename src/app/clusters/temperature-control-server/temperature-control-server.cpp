/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureControl::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

static Status setTemperatureHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                    chip::Optional<int16_t> targetTemperature, chip::Optional<uint8_t> targetTemperatureLevel);

namespace {

class TemperatureControlAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the TemperatureControl cluster on all endpoints.
    TemperatureControlAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TemperatureControl::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace
TemperatureControlAttrAccess gAttrAccess;

CHIP_ERROR TemperatureControlAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;

    if (aPath.mClusterId != TemperatureControl::Id)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR error = CHIP_NO_ERROR;
    EmberAfStatus status;

    switch (aPath.mAttributeId)
    {
    case TemperatureSetpoint::Id: {
        int16_t tempSetpoint = 0;
        status = TemperatureSetpoint::Get(endpoint, &tempSetpoint);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            error = aEncoder.Encode(tempSetpoint);
        }
        break;
    }

    case MinTemperature::Id: {
        int16_t minTemperature = 0;
        status = MinTemperature::Get(endpoint, &minTemperature);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            error = aEncoder.Encode(minTemperature);
        }
        break;
    }

    case MaxTemperature::Id: {
        int16_t maxTemperature = 0;
        status = MaxTemperature::Get(endpoint, &maxTemperature);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            error = aEncoder.Encode(maxTemperature);
        }
        break;
    }

    case Step::Id: {
        int16_t step = 0;
        status = Step::Get(endpoint, &step);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            error = aEncoder.Encode(step);
        }
        break;
    }

    case CurrentTemperatureLevelIndex::Id: {
        uint8_t currentTemperatureLevelIndex = 0;
        status = CurrentTemperatureLevelIndex::Get(endpoint, &currentTemperatureLevelIndex);
        if (status == EMBER_ZCL_STATUS_SUCCESS)
        {
            error = aEncoder.Encode(currentTemperatureLevelIndex);
        }
        break;
    }

    default:
        // We did not find a processing path, the caller will delegate elsewhere.
        break;
    }

    return error;
}

bool TemperatureControlHasFeature(EndpointId endpoint, TemperatureControlFeature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
}

static Status setTemperatureHandler(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                    chip::Optional<int16_t> targetTemperature, chip::Optional<uint8_t> targetTemperatureLevel)
{
    EndpointId endpoint = commandPath.mEndpointId;
    EmberAfStatus status;

    if (TemperatureControlHasFeature(endpoint, TemperatureControlFeature::kTemperatureNumber))
    {
        if (!targetTemperature.HasValue())
        {
            return Status::Failure;
        }

        int16_t minTemperature = 0;
        int16_t maxTemperature = 0;
        status = MinTemperature::Get(endpoint, &minTemperature);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return Status::Failure;
        }
        status = MaxTemperature::Get(endpoint, &maxTemperature);
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return Status::Failure;
        }

        if (targetTemperature.Value() < minTemperature || targetTemperature.Value() > maxTemperature)
        {
            return Status::ConstraintError;
        }
        status = TemperatureSetpoint::Set(endpoint, targetTemperature.Value());
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return Status::Failure;
        }
    }
    if (TemperatureControlHasFeature(endpoint, TemperatureControlFeature::kTemperatureLevel))
    {
        if (!targetTemperatureLevel.HasValue())
        {
            return Status::Failure;
        }
        // TODO: Check is targetTemperatureLevel is one of the SupportedTemperatureLevels
        // And ser CurrentTemperatureLevelIndex to Index from SupportedTemperatureLevels whose level
        // is mattching to targetTemperatureLevel

        //status = CurrentTemperatureLevelIndex::Set(endpoint, targetTemperatureLevel.Value());
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return Status::Failure;
        }
    }
    return Status::Success;
}
/**********************************************************
 * Callbacks Implementation
 *********************************************************/

bool emberAfTemperatureControlClusterSetTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::SetTemperature::DecodableType & commandData)
{
    auto & targetTemperature      = commandData.targetTemperature;
    auto & targetTemperatureLevel = commandData.targetTemperatureLevel;

    Status status = setTemperatureHandler(commandObj, commandPath, targetTemperature, targetTemperatureLevel);
    commandObj->AddStatus(commandPath, status);

    return true;
}

void emberAfTemperatureControlClusterServerInitCallback(chip::EndpointId endpoint) {}

void MatterTemperatureControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
