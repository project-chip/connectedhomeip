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
#pragma once

#include <app/clusters/thermostat-user-interface-configuration-server/ThermostatUserInterfaceConfigurationDelegate.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Attributes.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Enums.h>
#include <clusters/ThermostatUserInterfaceConfiguration/Metadata.h>

namespace chip::app::Clusters {

class ThermostatUserInterfaceConfigurationCluster : public DefaultServerCluster
{
public:
    using OptionalAttributeSet =
        app::OptionalAttributeSet<ThermostatUserInterfaceConfiguration::Attributes::ScheduleProgrammingVisibility::Id>;

    struct Config
    {
        Config() :
            temperatureDisplayMode(ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum::kCelsius),
            keypadLockout(ThermostatUserInterfaceConfiguration::KeypadLockoutEnum::kNoLockout),
            scheduleProgrammingVisibility(
                ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum::kScheduleProgrammingPermitted)
        {}

        ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum temperatureDisplayMode;
        ThermostatUserInterfaceConfiguration::KeypadLockoutEnum keypadLockout;
        ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum scheduleProgrammingVisibility;
        OptionalAttributeSet optionalAttributes{};
    };

    ThermostatUserInterfaceConfigurationCluster(EndpointId endpointId, const Config & config = {});

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    void SetDelegate(ThermostatUserInterfaceConfiguration::Delegate * delegate) { mDelegate = delegate; }

    Protocols::InteractionModel::Status
    SetTemperatureDisplayMode(ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum value);
    Protocols::InteractionModel::Status SetKeypadLockout(ThermostatUserInterfaceConfiguration::KeypadLockoutEnum value);
    Protocols::InteractionModel::Status
    SetScheduleProgrammingVisibility(ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum value);

    ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum GetTemperatureDisplayMode() const
    {
        return mTemperatureDisplayMode;
    }
    ThermostatUserInterfaceConfiguration::KeypadLockoutEnum GetKeypadLockout() const { return mKeypadLockout; }
    ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum GetScheduleProgrammingVisibility() const
    {
        return mScheduleProgrammingVisibility;
    }

protected:
    ThermostatUserInterfaceConfiguration::Delegate * mDelegate = nullptr;
    const OptionalAttributeSet mOptionalAttributes;
    ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum mTemperatureDisplayMode;
    ThermostatUserInterfaceConfiguration::KeypadLockoutEnum mKeypadLockout;
    ThermostatUserInterfaceConfiguration::ScheduleProgrammingVisibilityEnum mScheduleProgrammingVisibility;
};

} // namespace chip::app::Clusters
