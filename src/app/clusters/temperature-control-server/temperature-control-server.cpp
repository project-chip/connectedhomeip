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
#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureControl::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

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
    VerifyOrDie(aPath.mClusterId == TemperatureControl::Id);

    const TemperatureControl::SupportedTemperatureLevelsManager * gSupportedTemperatureLevelManager =
        TemperatureControl::GetSupportedTemperatureLevelsManager();

    if (TemperatureControl::Attributes::SupportedTemperatureLevels::Id == aPath.mAttributeId)
    {
        const TemperatureControl::SupportedTemperatureLevelsManager::TemperatureLevelOptionsProvider
            temperatureLevelOptionsProvider =
                gSupportedTemperatureLevelManager->GetTemperatureLevelOptionsProvider(aPath.mEndpointId);
        if (temperatureLevelOptionsProvider.begin() == nullptr)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }
        CHIP_ERROR err;
        err = aEncoder.EncodeList([temperatureLevelOptionsProvider](const auto & encoder) -> CHIP_ERROR {
            const auto * end = temperatureLevelOptionsProvider.end();
            for (auto * it = temperatureLevelOptionsProvider.begin(); it != end; ++it)
            {
                auto & temperatureLevelOption = *it;
                ReturnErrorOnFailure(encoder.Encode(temperatureLevelOption));
            }
            return CHIP_NO_ERROR;
        });
        ReturnErrorOnFailure(err);
    }
    return CHIP_NO_ERROR;
}

bool TemperatureControlHasFeature(EndpointId endpoint, TemperatureControl::Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
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
    EndpointId endpoint           = commandPath.mEndpointId;
    Status status                 = Status::Success;

    if (TemperatureControlHasFeature(endpoint, TemperatureControl::Feature::kTemperatureNumber))
    {
        if (!targetTemperature.HasValue())
        {
            status = Status::Failure;
        }

        int16_t minTemperature = 0;
        int16_t maxTemperature = 0;
        if (MinTemperature::Get(endpoint, &minTemperature) != EMBER_ZCL_STATUS_SUCCESS)
        {
            status = Status::Failure;
        }
        if (MaxTemperature::Get(endpoint, &maxTemperature) != EMBER_ZCL_STATUS_SUCCESS)
        {
            status = Status::Failure;
        }

        if (targetTemperature.Value() < minTemperature || targetTemperature.Value() > maxTemperature)
        {
            status = Status::ConstraintError;
        }

        if (TemperatureSetpoint::Set(endpoint, targetTemperature.Value()) != EMBER_ZCL_STATUS_SUCCESS)
        {
            status = Status::Failure;
        }
    }
    if (TemperatureControlHasFeature(endpoint, TemperatureControl::Feature::kTemperatureLevel))
    {
        if (!targetTemperatureLevel.HasValue())
        {
            status = Status::Failure;
        }
        const Structs::TemperatureLevelStruct::Type * dataPtr;
        // TODO: Update implemetation when https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/7005 fixed.
        const TemperatureControl::SupportedTemperatureLevelsManager * gSupportedTemperatureLevelManager =
            TemperatureControl::GetSupportedTemperatureLevelsManager();
        status = gSupportedTemperatureLevelManager->GetTemperatureLevelOptionByTemperatureLevel(
            endpoint, targetTemperatureLevel.Value(), &dataPtr);

        if (status == Status::Success)
        {
            if (CurrentTemperatureLevelIndex::Set(endpoint, targetTemperatureLevel.Value()) != EMBER_ZCL_STATUS_SUCCESS)
            {
                status = Status::Failure;
            }
        }
    }

    commandObj->AddStatus(commandPath, status);

    return true;
}

void emberAfTemperatureControlClusterServerInitCallback(chip::EndpointId endpoint) {}

void MatterTemperatureControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
