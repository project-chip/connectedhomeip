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

#include <devices/Types.h>
#include <devices/device-energy-management/DeviceEnergyManagementDevice.h>

namespace chip::app {

DeviceEnergyManagementDevice::DeviceEnergyManagementDevice(TimerDelegate & timerDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDeviceEnergyManagement, 1)),
    mTimerDelegate(timerDelegate)
{}

CHIP_ERROR DeviceEnergyManagementDevice::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                  EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    Clusters::DeviceEnergyManagementCluster::Config config(endpoint, BitMask<Clusters::DeviceEnergyManagement::Feature>(), *this);
    mDemCluster.Create(config);
    ReturnErrorOnFailure(provider.AddCluster(mDemCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void DeviceEnergyManagementDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mDemCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mDemCluster.Cluster()));
        mDemCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

Protocols::InteractionModel::Status
DeviceEnergyManagementDevice::PowerAdjustRequest(const int64_t power, const uint32_t duration,
                                                 Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementDevice::CancelPowerAdjustRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
DeviceEnergyManagementDevice::StartTimeAdjustRequest(const uint32_t requestedStartTime,
                                                     Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
DeviceEnergyManagementDevice::PauseRequest(const uint32_t duration, Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementDevice::ResumeRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementDevice::ModifyForecastRequest(
    const uint32_t forecastID,
    const DataModel::DecodableList<Clusters::DeviceEnergyManagement::Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
    Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementDevice::RequestConstraintBasedForecast(
    const DataModel::DecodableList<Clusters::DeviceEnergyManagement::Structs::ConstraintsStruct::Type> & constraints,
    Clusters::DeviceEnergyManagement::AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementDevice::CancelRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Clusters::DeviceEnergyManagement::ESATypeEnum DeviceEnergyManagementDevice::GetESAType()
{
    return Clusters::DeviceEnergyManagement::ESATypeEnum::kEvse;
}

bool DeviceEnergyManagementDevice::GetESACanGenerate()
{
    return false;
}

Clusters::DeviceEnergyManagement::ESAStateEnum DeviceEnergyManagementDevice::GetESAState()
{
    return mESAState;
}

int64_t DeviceEnergyManagementDevice::GetAbsMinPower()
{
    return 0;
}

int64_t DeviceEnergyManagementDevice::GetAbsMaxPower()
{
    return 1000;
}

Clusters::DeviceEnergyManagement::OptOutStateEnum DeviceEnergyManagementDevice::GetOptOutState()
{
    return Clusters::DeviceEnergyManagement::OptOutStateEnum::kNoOptOut;
}

const DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::PowerAdjustCapabilityStruct::Type> &
DeviceEnergyManagementDevice::GetPowerAdjustmentCapability()
{
    return mPowerAdjustmentCapability;
}

const DataModel::Nullable<Clusters::DeviceEnergyManagement::Structs::ForecastStruct::Type> &
DeviceEnergyManagementDevice::GetForecast()
{
    return mForecast;
}

CHIP_ERROR DeviceEnergyManagementDevice::SetESAState(Clusters::DeviceEnergyManagement::ESAStateEnum state)
{
    mESAState = state;
    return CHIP_NO_ERROR;
}

} // namespace chip::app
