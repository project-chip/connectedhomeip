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

#include <device/types/device-energy-management/EnergyManagement.h>
#include <devices/Types.h>

using namespace chip::app::Clusters::DeviceEnergyManagement;

namespace chip::app {

EnergyManagement::EnergyManagement(TimerDelegate & timerDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kDeviceEnergyManagement, 1)), mTimerDelegate(timerDelegate)
{}

CHIP_ERROR EnergyManagement::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(SingleEndpoint::mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    mProvider = &provider;
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    Clusters::DeviceEnergyManagementCluster::Config config(endpoint, BitMask<Feature>(), *this);
    mDemCluster.Create(config);
    ReturnErrorOnFailure(provider.AddCluster(mDemCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void EnergyManagement::Unregister(CodeDrivenDataModelProvider & provider)
{
    mProvider = nullptr;
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

Protocols::InteractionModel::Status EnergyManagement::PowerAdjustRequest(const int64_t power, const uint32_t duration,
                                                                         AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::CancelPowerAdjustRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::StartTimeAdjustRequest(const uint32_t requestedStartTime,
                                                                             AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::ResumeRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
EnergyManagement::ModifyForecastRequest(const uint32_t forecastID,
                                        const DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
                                        AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
EnergyManagement::RequestConstraintBasedForecast(const DataModel::DecodableList<Structs::ConstraintsStruct::Type> & constraints,
                                                 AdjustmentCauseEnum cause)
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status EnergyManagement::CancelRequest()
{
    return Protocols::InteractionModel::Status::Success;
}

ESATypeEnum EnergyManagement::GetESAType()
{
    return ESATypeEnum::kEvse;
}

bool EnergyManagement::GetESACanGenerate()
{
    return false;
}

ESAStateEnum EnergyManagement::GetESAState()
{
    return mESAState;
}

int64_t EnergyManagement::GetAbsMinPower()
{
    return 0;
}

int64_t EnergyManagement::GetAbsMaxPower()
{
    return 1000;
}

OptOutStateEnum EnergyManagement::GetOptOutState()
{
    return OptOutStateEnum::kNoOptOut;
}

const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> & EnergyManagement::GetPowerAdjustmentCapability()
{
    return mPowerAdjustmentCapability;
}

const DataModel::Nullable<Structs::ForecastStruct::Type> & EnergyManagement::GetForecast()
{
    return mForecast;
}

CHIP_ERROR EnergyManagement::SetESAState(ESAStateEnum state)
{
    VerifyOrReturnValue(mESAState != state, CHIP_NO_ERROR);
    mESAState = state;

    VerifyOrReturnValue(mProvider != nullptr, CHIP_NO_ERROR);
    mProvider->NotifyAttributeChanged({ SingleEndpoint::GetEndpointId(), Id, Attributes::ESAState::Id },
                                      DataModel::AttributeChangeType::kReportable);

    return CHIP_NO_ERROR;
}

} // namespace chip::app
