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

#include <device/types/microwave-oven/MicrowaveOven.h>
#include <devices/Types.h>

namespace chip::app {

MicrowaveOven::MicrowaveOven(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kMicrowaveOven, 1)),
    mDiagnosticDataProvider(config.diagnosticDataProvider)
{}

CHIP_ERROR MicrowaveOven::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mOperationalStateCluster.Create(endpoint, &mDelegate);
    mDelegate.SetCluster(&mOperationalStateCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    mMicrowaveOvenControlCluster.Create(
        endpoint,
        Clusters::MicrowaveOvenControlCluster::Config{
            .feature             = BitFlags<Clusters::MicrowaveOvenControl::Feature>(Clusters::MicrowaveOvenControl::Feature::kPowerInWatts),
            .supportsAddMoreTime = true,
            .integrationDelegate = mControlIntegrationDelegate,
            .appDelegate         = mControlAppDelegate,
        });
    ReturnErrorOnFailure(provider.AddCluster(mMicrowaveOvenControlCluster.Registration()));

    mMicrowaveOvenModeCluster.Create(
        endpoint, Clusters::MicrowaveOvenMode::Id,
        Clusters::ModeBaseCluster::Config{
            .feature                = BitFlags<Clusters::ModeBase::Feature>(),
            .appDelegate            = mMicrowaveOvenModeDelegate,
            .diagnosticDataProvider = mDiagnosticDataProvider,
        });
    ReturnErrorOnFailure(provider.AddCluster(mMicrowaveOvenModeCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void MicrowaveOven::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mMicrowaveOvenModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mMicrowaveOvenModeCluster.Cluster()));
        mMicrowaveOvenModeCluster.Destroy();
    }
    if (mMicrowaveOvenControlCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mMicrowaveOvenControlCluster.Cluster()));
        mMicrowaveOvenControlCluster.Destroy();
    }
    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
    }
}

} // namespace chip::app
