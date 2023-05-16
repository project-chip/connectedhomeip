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
 */

#include <app/util/af.h>

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/temperature-control-server/supported-temperature-levels-manager.h>
#include <app/util/error-mapping.h>
#include <app/util/config.h>
#include <app/util/error-mapping.h>
#include <app/util/odd-sized-integers.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <lib/core/CHIPEncoding.h>

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureControl;
using namespace chip::app::Clusters::TemperatureControl::Attributes;
using namespace chip::Protocols;
using chip::Protocols::InteractionModel::Status;

constexpr int16_t kDefaultMaxTemperature    = 3000; // 30C (86 F) is the default
constexpr int16_t kDefaultMinTemperature    = 1600; // 16C (61 F) is the default


#define FEATURE_MAP_TEMPERATURE_NUM   0x01
#define FEATURE_MAP_TEMPERATURE_LEVEL 0x02
#define FEATURE_MAP_DEFAULT FEATURE_MAP_TEMPERATURE_NUM

static bool isTemperatureSetpointWithinLimits(EndpointId endpoint, int16_t TemperatureSetpoint);
static bool isTemperatureLevelValueWithinLimits(const EndpointId endpointId, const uint8_t newTemperatureLevel);

namespace {

class TemperatureControlAttrAccess : public AttributeAccessInterface
{
public:
    TemperatureControlAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TemperatureControl::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

TemperatureControlAttrAccess gTemperatureControlAttrAccess;

CHIP_ERROR TemperatureControlAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == TemperatureControl::Id);

    const TemperatureControl::SupportedTemperatureLevelsManager * gSupportedTemperatureLevelManager = TemperatureControl::getSupportedTemperatureLevelsManager();

    if (TemperatureControl::Attributes::SupportedTemperatureLevels::Id == aPath.mAttributeId)
    {
        const TemperatureControl::SupportedTemperatureLevelsManager::TemperatureLevelOptionsProvider temperatureLevelOptionsProvider =
            gSupportedTemperatureLevelManager->getTemperatureLevelOptionsProvider(aPath.mEndpointId);
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


} // anonymous namespace

void emberAfTemperatureControlClusterServerInitCallback(chip::EndpointId endpoint)
{
    //TODO
}
bool emberAfTemperatureControlClusterSetTemperatureCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::SetTemperature::DecodableType & commandData)
{
    auto & targetTemperature      = commandData.targetTemperature;
    auto & targetTemperatureLevel = commandData.targetTemperatureLevel;

    EndpointId aEndpointId = commandPath.mEndpointId;

    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;

    uint32_t OurFeatureMap;
    bool TemperatureSupported         = false;
    bool TemperatureLevelSupported    = false;

    if (FeatureMap::Get(aEndpointId, &OurFeatureMap) != EMBER_ZCL_STATUS_SUCCESS)
        OurFeatureMap = FEATURE_MAP_DEFAULT;

    if (OurFeatureMap & 1 << 0)
        TemperatureSupported = true;

    if (OurFeatureMap & 1 << 1)
        TemperatureLevelSupported = true;

    if (TemperatureSupported && TemperatureLevelSupported)
    {
        ChipLogError(Zcl, "Error: both tempNum and tempLevel feature support");
        return false;
    }

    if (!TemperatureSupported && !TemperatureLevelSupported)
    {
        ChipLogError(Zcl, "Error: no tempNum or tempLevel feature support");
        return false;
    }

    if (TemperatureSupported)
    {
        if (isTemperatureSetpointWithinLimits(aEndpointId, targetTemperature))
            status = TemperatureSetpoint::Set(aEndpointId, targetTemperature);
        else
            status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }

    if (TemperatureLevelSupported)
    {
        if (isTemperatureLevelValueWithinLimits(aEndpointId, targetTemperatureLevel))
            status = CurrentTemperatureLevelIndex::Set(aEndpointId, targetTemperatureLevel);
        else
            status = EMBER_ZCL_STATUS_CONSTRAINT_ERROR;
    }

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Error: SetTemperature failed!");
    }

    commandObj->AddStatus(commandPath, app::ToInteractionModelStatus(status));
    return true;
}

void MatterTemperatureControlPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gTemperatureControlAttrAccess);
}

static bool isTemperatureSetpointWithinLimits(EndpointId endpoint, int16_t TemperatureSetpoint)
{
    int16_t MinTemperature = kDefaultMinTemperature;
    int16_t MaxTemperature = kDefaultMaxTemperature;

    EmberAfStatus status;

    status = MinTemperature::Get(endpoint, &MinTemperature);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: MinTemperature missing using default");
    }

    status = MaxTemperature::Get(endpoint, &MaxTemperature);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "Warning: MaxTemperature missing using default");
    }

    if (TemperatureSetpoint < MinTemperature)
        return false;

    if (TemperatureSetpoint > MaxTemperature)
        return false;

    return true;

}

/**
 * Checks the new temperature level against the endpoint's supported temperature levels.
 * @param endpointId    endpointId of the endpoint
 * @param newTemperatureLevel       value of the new temperature level
 * @return              Success status if the value is valid; InvalidValue otherwise.
 */
static bool isTemperatureLevelValueWithinLimits(const EndpointId endpointId, const uint8_t newTemperatureLevel)
{
    const TemperatureControl::Structs::TemperatureLevelStruct::Type * temperatureLevelOptionPtr;
    if (TemperatureControl::getSupportedTemperatureLevelsManager()->getTemperatureLevelOptionByTemperatureLevel(endpointId, newTemperatureLevel, &temperatureLevelOptionPtr) == Status::Success)
        return true;
    else
        return false;
}
