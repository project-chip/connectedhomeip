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

#include <clusters/RvcCleanMode/Metadata.h>
#include <clusters/RvcRunMode/Metadata.h>
#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <devices/Types.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

RoboticVacuumCleaner::RoboticVacuumCleaner() :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRoboticVacuumCleaner, 1))
{}

CHIP_ERROR RoboticVacuumCleaner::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                          EndpointComposition composition)
{
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mOperationalStateCluster.Create(endpoint, mDelegate);
    mDelegate.SetCluster(&mOperationalStateCluster.Cluster());
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    mServiceAreaCluster.Create(endpoint, mServiceAreaStorageDelegate, mServiceAreaDelegate,
                               BitMask<Clusters::ServiceArea::Feature>());
    ReturnErrorOnFailure(provider.AddCluster(mServiceAreaCluster.Registration()));

    mRunModeCluster.Create(endpoint, Clusters::RvcRunMode::Id,
                           Clusters::ModeBaseCluster::Config{
                               .feature                = BitMask<Clusters::ModeBase::Feature>(),
                               .optionalAttributeSet   = {},
                               .appDelegate            = mRunModeDelegate,
                               .onOffValueForStartUp   = false,
                               .diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider(),
                               .clusterRevision        = Clusters::RvcRunMode::kRevision,
                           });
    ReturnErrorOnFailure(provider.AddCluster(mRunModeCluster.Registration()));

    mCleanModeCluster.Create(endpoint, Clusters::RvcCleanMode::Id,
                             Clusters::ModeBaseCluster::Config{
                                 .feature                = BitMask<Clusters::ModeBase::Feature>(),
                                 .optionalAttributeSet   = {},
                                 .appDelegate            = mCleanModeDelegate,
                                 .onOffValueForStartUp   = false,
                                 .diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider(),
                                 .clusterRevision        = Clusters::RvcCleanMode::kRevision,
                             });
    ReturnErrorOnFailure(provider.AddCluster(mCleanModeCluster.Registration()));

    mRunModeDelegate.SetCluster(&mRunModeCluster.Cluster());
    mRunModeDelegate.SetOperationalStateCluster(&mOperationalStateCluster.Cluster());
    mCleanModeDelegate.SetRunModeCluster(&mRunModeCluster.Cluster());

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    transaction.Commit();
    return CHIP_NO_ERROR;
}

void RoboticVacuumCleaner::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mCleanModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mCleanModeCluster.Cluster()));
        mCleanModeCluster.Destroy();
    }
    if (mRunModeCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mRunModeCluster.Cluster()));
        mRunModeCluster.Destroy();
    }
    if (mServiceAreaCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mServiceAreaCluster.Cluster()));
        mServiceAreaCluster.Destroy();
    }
    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
    }
}

} // namespace chip::app
