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
#include <app/AttributeAccessInterfaceRegistry.h>
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

const uint8_t kMaxTemperatureLevelStringSize = 32;

static SupportedTemperatureLevelsIteratorDelegate * sInstance = nullptr;

class TemperatureControlAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the TemperatureControl cluster on all endpoints.
    TemperatureControlAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), TemperatureControl::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
};

} // namespace
TemperatureControlAttrAccess gAttrAccess;

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

SupportedTemperatureLevelsIteratorDelegate * GetInstance()
{
    return sInstance;
}

void SetInstance(SupportedTemperatureLevelsIteratorDelegate * instance)
{
    sInstance = instance;
}

} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip

CHIP_ERROR TemperatureControlAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrDie(aPath.mClusterId == TemperatureControl::Id);
    if (TemperatureControl::Attributes::SupportedTemperatureLevels::Id == aPath.mAttributeId)
    {
        TemperatureControl::SupportedTemperatureLevelsIteratorDelegate * instance = TemperatureControl::GetInstance();
        if (instance == nullptr)
        {
            aEncoder.EncodeEmptyList();
            return CHIP_NO_ERROR;
        }
        instance->Reset(aPath.mEndpointId);
        err = aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
            char buffer[kMaxTemperatureLevelStringSize];
            MutableCharSpan item(buffer);
            while (instance->Next(item) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encoder.Encode(item));
                item = MutableCharSpan(buffer);
            }
            return CHIP_NO_ERROR;
        });
    }
    return err;
}

bool TemperatureControlHasFeature(EndpointId endpoint, TemperatureControl::Feature feature)
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(endpoint, &featureMap) == Status::Success);

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

    if (TemperatureControlHasFeature(endpoint, Feature::kTemperatureNumber) &&
        TemperatureControlHasFeature(endpoint, Feature::kTemperatureLevel))
    {
        commandObj->AddStatus(commandPath, Status::Failure);
        return false;
    }
    if (TemperatureControlHasFeature(endpoint, TemperatureControl::Feature::kTemperatureNumber))
    {
        if (targetTemperature.HasValue())
        {
            int16_t minTemperature = 0;
            int16_t maxTemperature = 0;
            status                 = MinTemperature::Get(endpoint, &minTemperature);
            if (status != Status::Success)
            {
                goto exit;
            }

            status = MaxTemperature::Get(endpoint, &maxTemperature);
            if (status != Status::Success)
            {
                goto exit;
            }

            if (targetTemperature.Value() < minTemperature || targetTemperature.Value() > maxTemperature)
            {
                status = Status::ConstraintError;
                goto exit;
            }
            if (TemperatureControlHasFeature(endpoint, TemperatureControl::Feature::kTemperatureStep))
            {
                int16_t step = 0;
                status       = Step::Get(endpoint, &step);
                if (status != Status::Success)
                {
                    goto exit;
                }

                if ((targetTemperature.Value() - minTemperature) % step != 0)
                {
                    status = Status::ConstraintError;
                    goto exit;
                }
            }
            status = TemperatureSetpoint::Set(endpoint, targetTemperature.Value());
            if (status != Status::Success)
            {
                /**
                 * If the server is unable to execute the command at the time the command is received
                 * by the server (e.g. due to the design of a device it cannot accept a change in its
                 * temperature setting after it has begun operation), then the server SHALL respond
                 * with a status code of INVALID_IN_STATE, and discard the command.
                 **/
                status = Status::InvalidInState;
            }
        }
        else
        {
            status = Status::InvalidCommand;
        }
    }
    if (TemperatureControlHasFeature(endpoint, TemperatureControl::Feature::kTemperatureLevel))
    {
        if (targetTemperatureLevel.HasValue())
        {
            TemperatureControl::SupportedTemperatureLevelsIteratorDelegate * instance = TemperatureControl::GetInstance();
            if (instance == nullptr)
            {
                status = Status::NotFound;
                goto exit;
            }

            instance->Reset(endpoint);

            uint8_t size = instance->Size();

            if (targetTemperatureLevel.Value() < size)
            {
                status = SelectedTemperatureLevel::Set(endpoint, targetTemperatureLevel.Value());
                if (status != Status::Success)
                {
                    /**
                     * If the server is unable to execute the command at the time the command is received
                     * by the server (e.g. due to the design of a device it cannot accept a change in its
                     * temperature setting after it has begun operation), then the server SHALL respond
                     * with a status code of INVALID_IN_STATE, and discard the command.
                     **/
                    status = Status::InvalidInState;
                }
            }
            else
            {
                status = Status::ConstraintError;
            }
        }
        else
        {
            status = Status::InvalidCommand;
        }
    }
exit:
    commandObj->AddStatus(commandPath, status);

    return true;
}

void emberAfTemperatureControlClusterServerInitCallback(EndpointId endpoint) {}

void MatterTemperatureControlPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
