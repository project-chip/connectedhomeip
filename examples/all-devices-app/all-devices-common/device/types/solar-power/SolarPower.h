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
#include <device/types/power-source/WiredPowerSource.h>

namespace chip::app {

/// Solar Power device type (0x0017) rev 2 parent with mandatory grid Power Source;
/// subclasses must register the mandatory Electrical Sensor (0x0510) endpoint.
class SolarPower : public DeviceInterface
{
public:
    ~SolarPower() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    EndpointId GetEndpointId() const { return mEndpointId; }

    WiredPowerSource & GetGridPowerSource() { return mGridPowerSource; }

protected:
    SolarPower();

    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    virtual CHIP_ERROR RegisterAdditionalComposedEndpoints(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                                           EndpointId parent)
    {
        return CHIP_NO_ERROR;
    }

    virtual void UnregisterAdditionalComposedEndpoints(CodeDrivenDataModelProvider & provider) {}

private:
    void UnregisterMandatoryComposedEndpoints(CodeDrivenDataModelProvider & provider);

    EndpointId mEndpointId = kInvalidEndpointId;

    WiredPowerSource mGridPowerSource;
};

} // namespace chip::app
