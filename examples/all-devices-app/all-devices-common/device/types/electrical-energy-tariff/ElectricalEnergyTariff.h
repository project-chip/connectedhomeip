/*
 *
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

#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>

namespace chip::app {

/**
 * Electrical Energy Tariff device type (0x0513).
 *
 * The device type has no mandatory server clusters. Commodity Price, Electrical Grid Conditions and
 * Commodity Tariff are all optional, so subclasses add the ones they support through
 * RegisterOptionalClusters() / UnregisterOptionalClusters().
 */
class ElectricalEnergyTariff : public SingleEndpoint
{
public:
    ElectricalEnergyTariff();
    ~ElectricalEnergyTariff() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;

    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    /// Called by Register() after the Descriptor is registered and before the endpoint is added.
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    /// Called by Unregister(), also when Register() fails partway. Implementations must only remove
    /// clusters that are constructed, and must not rely on GetEndpointId() since the endpoint ID is
    /// already reset at this point.
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}
};

} // namespace chip::app
