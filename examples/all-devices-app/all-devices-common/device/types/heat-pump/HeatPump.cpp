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

#include "HeatPump.h"

#include <clusters/Thermostat/ClusterId.h>
#include <devices/Types.h>

namespace chip {
namespace app {

HeatPump::HeatPump(const Config & config, Parts parts) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kHeatPump, 1)), mConfig(config),
    mPowerSource(std::move(parts.powerSource)), mEnergyManagement(std::move(parts.energyManagement)),
    mElectricalSensor(std::move(parts.electricalSensor)), mThermostat(std::move(parts.thermostat))
{
    VerifyOrDie(mPowerSource != nullptr && mEnergyManagement != nullptr && mElectricalSensor != nullptr);
}

CHIP_ERROR HeatPump::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                              EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(HasThermostatControl(), CHIP_ERROR_INCORRECT_STATE);
    const EndpointId endpoint = allocator.Allocate();
    VerifyOrReturnError(endpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    DeviceRegistrationTransaction transaction(*this, provider);

    Span<const EndpointComposition::SemanticTag> tags = composition.tagList.empty() ? mConfig.tags : composition.tagList;
    ReturnErrorOnFailure(RegisterDescriptor(
        endpoint, provider, EndpointComposition(composition.parentId, DataModel::EndpointCompositionPattern::kTree, tags)));
    mEndpointId = endpoint;

    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(RegisterParts(allocator, provider));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void HeatPump::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterParts(provider);
    if (mEndpointId != kInvalidEndpointId)
    {
        UnregisterDescriptor(mEndpointId, provider);
        mEndpointId = kInvalidEndpointId;
    }
    UnregisterOptionalClusters(provider);
}

bool HeatPump::HasThermostatControl() const
{
    VerifyOrReturnValue(mThermostat == nullptr, true);

    ReadOnlyBufferBuilder<ClusterId> clientClusters;
    VerifyOrReturnValue(ClientClusters(clientClusters) == CHIP_NO_ERROR, false);
    for (ClusterId clusterId : clientClusters.TakeBuffer())
    {
        VerifyOrReturnValue(clusterId != Clusters::Thermostat::Id, true);
    }
    return false;
}

CHIP_ERROR HeatPump::RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider)
{
    const EndpointComposition child = EndpointComposition::WithParent(mEndpointId);

    ReturnErrorOnFailure(mPowerSource->Register(allocator, provider, child));
    ReturnErrorOnFailure(mEnergyManagement->Register(allocator, provider, child));
    ReturnErrorOnFailure(mElectricalSensor->Register(allocator, provider, child));
    if (mThermostat != nullptr)
    {
        ReturnErrorOnFailure(mThermostat->Register(allocator, provider, child));
    }
    return CHIP_NO_ERROR;
}

void HeatPump::UnregisterParts(CodeDrivenDataModelProvider & provider)
{
    if (mThermostat != nullptr && mThermostat->GetEndpointId() != kInvalidEndpointId)
    {
        mThermostat->Unregister(provider);
    }
    if (mElectricalSensor->GetEndpointId() != kInvalidEndpointId)
    {
        mElectricalSensor->Unregister(provider);
    }
    if (mEnergyManagement->GetEndpointId() != kInvalidEndpointId)
    {
        mEnergyManagement->Unregister(provider);
    }
    if (mPowerSource->GetEndpointId() != kInvalidEndpointId)
    {
        mPowerSource->Unregister(provider);
    }
}

} // namespace app
} // namespace chip
