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

#include <app/clusters/mode-base-server/AppDelegate.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/operational-state-server/RvcOperationalStateCluster.h>
#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <app/clusters/service-area-server/service-area-delegate.h>
#include <app/clusters/service-area-server/service-area-storage-delegate.h>
#include <device/api/SingleEndpoint.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

// Generic RVC device: owns the mandatory clusters and wires them to whatever delegate
// implementation is supplied. All device-specific simulation behavior lives in the delegate
// implementation (see impl/SimulatedRoboticVacuumCleaner.h), not here.
class RoboticVacuumCleaner : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::OperationalState::OperationalStateCluster::Delegate & operationalStateDelegate;
        Clusters::ServiceArea::StorageDelegate & serviceAreaStorageDelegate;
        Clusters::ServiceArea::Delegate & serviceAreaDelegate;
        Clusters::ModeBase::AppDelegate & runModeDelegate;
        Clusters::ModeBase::AppDelegate & cleanModeDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit RoboticVacuumCleaner(const Config & config);
    ~RoboticVacuumCleaner() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::RvcOperationalState::RvcOperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::ServiceArea::ServiceAreaCluster & GetServiceAreaCluster() { return mServiceAreaCluster.Cluster(); }
    Clusters::ModeBaseCluster & RunMode() { return mRunModeCluster.Cluster(); }
    Clusters::ModeBaseCluster & CleanMode() { return mCleanModeCluster.Cluster(); }

private:
    Clusters::OperationalState::OperationalStateCluster::Delegate & mOperationalStateDelegate;
    LazyRegisteredServerCluster<Clusters::RvcOperationalState::RvcOperationalStateCluster> mOperationalStateCluster;

    Clusters::ServiceArea::StorageDelegate & mServiceAreaStorageDelegate;
    Clusters::ServiceArea::Delegate & mServiceAreaDelegate;
    LazyRegisteredServerCluster<Clusters::ServiceArea::ServiceAreaCluster> mServiceAreaCluster;

    Clusters::ModeBase::AppDelegate & mRunModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mRunModeCluster;

    Clusters::ModeBase::AppDelegate & mCleanModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mCleanModeCluster;

    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
};

} // namespace chip::app
