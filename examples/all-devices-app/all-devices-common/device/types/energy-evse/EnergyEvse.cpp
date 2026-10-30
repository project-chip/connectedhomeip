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

#include <device/types/energy-evse/EnergyEvse.h>

#include <devices/Types.h>

namespace chip::app {

EnergyEvse::EnergyEvse(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kEvse, 1)), mConfig(config)
{}

CHIP_ERROR EnergyEvse::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mEvseCluster.Create(Clusters::EnergyEvse::EnergyEvseCluster::Config(
        endpoint, mConfig.evseDelegate, mConfig.evseFeatures, mConfig.evseOptionalAttrs, mConfig.evseOptionalCmds));
    ReturnErrorOnFailure(provider.AddCluster(mEvseCluster.Registration()));

    mEvseModeCluster.Create(endpoint, Clusters::ModeBase::kEnergyEvseMode,
                            Clusters::ModeBaseCluster::Config{
                                .feature                = BitMask<Clusters::ModeBase::Feature>(),
                                .optionalAttributeSet   = {},
                                .appDelegate            = mConfig.evseModeDelegate,
                                .onOffValueForStartUp   = false,
                                .diagnosticDataProvider = mConfig.diagnosticDataProvider,
                            });
    ReturnErrorOnFailure(provider.AddCluster(mEvseModeCluster.Registration()));

    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void EnergyEvse::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    UnregisterOptionalClusters(provider);
    if (mEvseModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mEvseModeCluster.Cluster()));
        mEvseModeCluster.Destroy();
    }
    if (mEvseCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mEvseCluster.Cluster()));
        mEvseCluster.Destroy();
    }
}

Clusters::EnergyEvse::EnergyEvseCluster & EnergyEvse::EnergyEvseCluster()
{
    VerifyOrDie(mEvseCluster.IsConstructed());
    return mEvseCluster.Cluster();
}

Clusters::ModeBaseCluster & EnergyEvse::EnergyEvseModeCluster()
{
    VerifyOrDie(mEvseModeCluster.IsConstructed());
    return mEvseModeCluster.Cluster();
}

} // namespace chip::app

