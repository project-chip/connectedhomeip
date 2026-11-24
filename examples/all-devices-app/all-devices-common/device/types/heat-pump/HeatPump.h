/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/api/Interface.h>
#include <device/api/SingleEndpoint.h>

#include <memory>

namespace chip {
namespace app {

/// Heat Pump device type (0x0309) with its mandatory composed Power Source, Device Energy
/// Management and Electrical Sensor endpoints.
///
/// Optional child endpoints (Thermostat, Temperature Sensor, Water Heater) are added by subclasses
/// through RegisterOptionalEndpoints(). The spec requires one or more Thermostat endpoints or a
/// Thermostat client, so Register() fails with CHIP_ERROR_INCORRECT_STATE when
/// HasThermostatControl() is false after all endpoints are registered.
class HeatPump : public DeviceInterface
{
public:
    struct Config
    {
        Span<const EndpointComposition::SemanticTag> tags = {};
    };

    /// Mandatory endpoints composed under the heat pump endpoint. None may be null.
    struct Parts
    {
        std::unique_ptr<SingleEndpoint> powerSource;
        std::unique_ptr<SingleEndpoint> energyManagement;
        std::unique_ptr<SingleEndpoint> electricalSensor;
    };

    HeatPump(const Config & config, Parts parts);
    ~HeatPump() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    EndpointId GetEndpointId() const { return mEndpointId; }

    /// True when ClientClusters() reports the Thermostat cluster, or when `provider` has an endpoint
    /// with the Thermostat device type whose parent is this heat pump endpoint.
    bool HasThermostatControl(CodeDrivenDataModelProvider & provider) const;

protected:
    /// Called before the heat pump endpoint is added, within the registration transaction.
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called after the heat pump endpoint is removed, including on partial registration failure.
    /// Overrides must tolerate clusters that were not constructed or registered.
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    /// Called after the mandatory endpoints are registered, to add optional child endpoints under `parent`.
    virtual CHIP_ERROR RegisterOptionalEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                 EndpointId parent)
    {
        return CHIP_NO_ERROR;
    }

    /// Called before the mandatory endpoints are removed, including on partial registration failure.
    /// Overrides must tolerate endpoints that were not registered.
    virtual void UnregisterOptionalEndpoints(CodeDrivenDataModelProvider & provider) {}

private:
    CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider);
    void UnregisterParts(CodeDrivenDataModelProvider & provider);

    EndpointId mEndpointId = kInvalidEndpointId;
    const Config mConfig;
    std::unique_ptr<SingleEndpoint> mPowerSource;
    std::unique_ptr<SingleEndpoint> mEnergyManagement;
    std::unique_ptr<SingleEndpoint> mElectricalSensor;
};

} // namespace app
} // namespace chip
