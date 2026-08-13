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

#include <device/types/laundry-washer/LaundryWasher.h>
#include <devices/Types.h>

namespace chip::app {

LaundryWasher::LaundryWasher(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kLaundryWasher, 1)),
    mDiagnosticDataProvider(config.diagnosticDataProvider != nullptr ? *config.diagnosticDataProvider
                                                                      : DeviceLayer::GetDiagnosticDataProvider())
{}

CHIP_ERROR LaundryWasher::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mOperationalStateCluster.Create(
        endpoint, &mDelegate,
        Clusters::OperationalState::OperationalStateCluster::Config{
            .optionalAttributes = Clusters::OperationalState::OperationalStateCluster::OptionalAttributeSet()
                                      .Set<Clusters::OperationalState::Attributes::CountdownTime::Id>(),
        });
    mDelegate.SetCluster(&mOperationalStateCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    mLaundryWasherControlsCluster.Create(
        endpoint,
        Clusters::LaundryWasherControlsCluster::Config(
            BitFlags<Clusters::LaundryWasherControls::Feature>(Clusters::LaundryWasherControls::Feature::kSpin,
                                                               Clusters::LaundryWasherControls::Feature::kRinse),
            mLaundryWasherControlsDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mLaundryWasherControlsCluster.Registration()));

    mLaundryWasherModeCluster.Create(
        endpoint, Clusters::LaundryWasherMode::Id,
        Clusters::ModeBaseCluster::Config{
            .feature                = BitFlags<Clusters::ModeBase::Feature>(),
            .appDelegate            = mLaundryWasherModeDelegate,
            .diagnosticDataProvider = mDiagnosticDataProvider,
        });
    ReturnErrorOnFailure(provider.AddCluster(mLaundryWasherModeCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void LaundryWasher::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mLaundryWasherModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mLaundryWasherModeCluster.Cluster()));
        mLaundryWasherModeCluster.Destroy();
    }
    if (mLaundryWasherControlsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mLaundryWasherControlsCluster.Cluster()));
        mLaundryWasherControlsCluster.Destroy();
    }
    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
    }
}

} // namespace chip::app
