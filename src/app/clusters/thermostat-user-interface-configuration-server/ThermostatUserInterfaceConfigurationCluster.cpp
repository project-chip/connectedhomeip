/*
 *
 *    Copyright (c) 2021-2026 Project CHIP Authors
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

#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ThermostatUserInterfaceConfiguration/EnumsCheck.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip::app::Clusters {

using namespace ThermostatUserInterfaceConfiguration;
using namespace ThermostatUserInterfaceConfiguration::Attributes;
using chip::Protocols::InteractionModel::Status;

ThermostatUserInterfaceConfigurationCluster::ThermostatUserInterfaceConfigurationCluster(EndpointId endpointId,
                                                                                         const Config & config) :
    DefaultServerCluster({ endpointId, ThermostatUserInterfaceConfiguration::Id }), mOptionalAttributes(config.optionalAttributes),
    mTemperatureDisplayMode(config.temperatureDisplayMode), mKeypadLockout(config.keypadLockout),
    mScheduleProgrammingVisibility(config.scheduleProgrammingVisibility)
{}

CHIP_ERROR ThermostatUserInterfaceConfigurationCluster::Attributes(const ConcreteClusterPath & path,
                                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const DataModel::AttributeEntry optionalAttributes[] = {
        ScheduleProgrammingVisibility::kMetadataEntry,
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), mOptionalAttributes);
}

DataModel::ActionReturnStatus
ThermostatUserInterfaceConfigurationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case TemperatureDisplayMode::Id:
        return encoder.Encode(mTemperatureDisplayMode);
    case KeypadLockout::Id:
        return encoder.Encode(mKeypadLockout);
    case ScheduleProgrammingVisibility::Id:
        return encoder.Encode(mScheduleProgrammingVisibility);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus
ThermostatUserInterfaceConfigurationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                            AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case TemperatureDisplayMode::Id: {
        TemperatureDisplayModeEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetTemperatureDisplayMode(value);
    }
    case KeypadLockout::Id: {
        KeypadLockoutEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetKeypadLockout(value);
    }
    case ScheduleProgrammingVisibility::Id: {
        ScheduleProgrammingVisibilityEnum value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetScheduleProgrammingVisibility(value);
    }
    default:
        return Status::UnsupportedAttribute;
    }
}

Status ThermostatUserInterfaceConfigurationCluster::SetTemperatureDisplayMode(TemperatureDisplayModeEnum value)
{
    if (EnsureKnownEnumValue(value) == TemperatureDisplayModeEnum::kUnknownEnumValue)
    {
        return Status::ConstraintError;
    }

    if (SetAttributeValue(mTemperatureDisplayMode, value, TemperatureDisplayMode::Id) && mDelegate != nullptr)
    {
        mDelegate->OnTemperatureDisplayModeChanged(value);
    }
    return Status::Success;
}

Status ThermostatUserInterfaceConfigurationCluster::SetKeypadLockout(KeypadLockoutEnum value)
{
    if (EnsureKnownEnumValue(value) == KeypadLockoutEnum::kUnknownEnumValue)
    {
        return Status::ConstraintError;
    }

    if (SetAttributeValue(mKeypadLockout, value, KeypadLockout::Id) && mDelegate != nullptr)
    {
        mDelegate->OnKeypadLockoutChanged(value);
    }
    return Status::Success;
}

Status ThermostatUserInterfaceConfigurationCluster::SetScheduleProgrammingVisibility(ScheduleProgrammingVisibilityEnum value)
{
    if (EnsureKnownEnumValue(value) == ScheduleProgrammingVisibilityEnum::kUnknownEnumValue)
    {
        return Status::ConstraintError;
    }

    if (SetAttributeValue(mScheduleProgrammingVisibility, value, ScheduleProgrammingVisibility::Id) && mDelegate != nullptr)
    {
        mDelegate->OnScheduleProgrammingVisibilityChanged(value);
    }
    return Status::Success;
}

} // namespace chip::app::Clusters
