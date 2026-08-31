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

#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

RoboticVacuumCleaner::RoboticVacuumCleaner(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRoboticVacuumCleaner, 1)),
    mOperationalStateDelegate(config.operationalStateDelegate), mRunModeDelegate(config.runModeDelegate),
    mRunModeStartupValue(config.runModeStartupValue), mDiagnosticDataProvider(config.diagnosticDataProvider)
{}

CHIP_ERROR RoboticVacuumCleaner::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mOperationalStateCluster.Create(endpoint, mOperationalStateDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    mRunModeCluster.Create(endpoint, Clusters::ModeBase::kRvcRunMode,
                           Clusters::ModeBaseCluster::Config{
                               .feature                = BitMask<Clusters::ModeBase::Feature>(),
                               .optionalAttributeSet   = {},
                               .appDelegate            = mRunModeDelegate,
                               .onOffValueForStartUp   = false,
                               .diagnosticDataProvider = mDiagnosticDataProvider,
                           });
    ReturnErrorOnFailure(provider.AddCluster(mRunModeCluster.Registration()));

    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));

    mRunModeCluster.Cluster().UpdateCurrentMode(mRunModeStartupValue);
    LogErrorOnFailure(mOperationalStateCluster.Cluster().SetOperationalState(
        to_underlying(Clusters::OperationalState::OperationalStateEnum::kStopped)));
    LogErrorOnFailure(mOperationalStateCluster.Cluster().SetCurrentPhase(0));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void RoboticVacuumCleaner::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    UnregisterOptionalClusters(provider);
    if (mRunModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mRunModeCluster.Cluster()));
        mRunModeCluster.Destroy();
    }
    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
    }
}

} // namespace chip::app
