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

#include <device/types/room-air-conditioner/RoomAirConditioner.h>
#include <devices/Types.h>

namespace chip::app {

RoomAirConditioner::RoomAirConditioner(const Context & context) :
    DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRoomAirConditioner, 1)),
    mTimerDelegate(context.timerDelegate), mIdentifyDelegate(context.identifyDelegate), mOnOffDelegate(context.onOffDelegate),
    mThermostatDelegate(context.thermostatDelegate), mCoolingDelegate(context.coolingDelegate)
{}

CHIP_ERROR RoomAirConditioner::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                        EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    const EndpointId endpoint = allocator.Allocate();
    VerifyOrReturnError(endpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(
        endpoint, provider,
        EndpointComposition(composition.parentId, DataModel::EndpointCompositionPattern::kTree, composition.tagList)));
    mEndpointId = endpoint;

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mThermostatCluster.Create(endpoint, BitFlags<Clusters::Thermostat::Feature>(Clusters::Thermostat::Feature::kCooling),
                              CoolingThermostat::Config({}, mTimerDelegate), mThermostatDelegate, mCoolingDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mThermostatCluster.Registration()));

    Clusters::OnOffCluster::Context onOffContext{ mTimerDelegate };
    onOffContext.featureMap.Set(Clusters::OnOff::Feature::kDeadFrontBehavior);
    mOnOffCluster.Create(endpoint, onOffContext);
    mOnOffCluster.Cluster().AddDelegate(&mOnOffDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOnOffCluster.Registration()));

    ReturnErrorOnFailure(RegisterAdditionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    ReturnErrorOnFailure(RegisterAdditionalEndpoints(allocator, provider));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void RoomAirConditioner::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterAdditionalEndpoints(provider);
    UnregisterDescriptor(mEndpointId, provider);
    UnregisterAdditionalClusters(provider);
    if (mThermostatCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThermostatCluster.Cluster()));
        mThermostatCluster.Destroy();
    }
    if (mOnOffCluster.IsConstructed())
    {
        mOnOffCluster.Cluster().RemoveDelegate(&mOnOffDelegate);
        LogErrorOnFailure(provider.RemoveCluster(&mOnOffCluster.Cluster()));
        mOnOffCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    mEndpointId = kInvalidEndpointId;
}

} // namespace chip::app
