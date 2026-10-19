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

#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <clusters/shared/Enums.h>
#include <device/api/Interface.h>
#include <devices/Types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class IrrigationSystem : public DeviceInterface
{
public:
    IrrigationSystem(TimerDelegate & timerDelegate) :
        DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kIrrigationSystem, 1)), mTimerDelegate(timerDelegate)
    {}
    ~IrrigationSystem() override = default;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    EndpointId GetEndpointId() const { return mEndpointId; }

protected:
    TimerDelegate & mTimerDelegate;

    // Hook for subclasses to add optional clusters (Identify, Operational State, Flow Measurement, ...)
    // to the irrigation system endpoint. Called before the endpoint is added. Default: none added.
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    // Counterpart to RegisterOptionalClusters(). Also called when Register() fails partway, so
    // implementations must only remove clusters that were constructed.
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

private:
    virtual CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                     EndpointComposition composition)    = 0;
    virtual void UnregisterParts(CodeDrivenDataModelProvider & provider) = 0;
    EndpointId mEndpointId                                               = kInvalidEndpointId;
};

} // namespace app
} // namespace chip
