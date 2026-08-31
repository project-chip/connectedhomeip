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
#include <device/api/SingleEndpoint.h>
#include <platform/DiagnosticDataProvider.h>

#include <cstdint>

namespace chip::app {

// Generic RVC device: owns only the clusters the Robotic Vacuum Cleaner device type mandates
// (RVC Operational State, RVC Run Mode) and wires them to whatever delegate implementation is
// supplied. RVC Clean Mode and Service Area are optional per the device type definition, so this
// base class does not assume their presence or configuration; subclasses that want them override
// RegisterOptionalClusters()/UnregisterOptionalClusters() to add and configure their own (see
// impl/SimulatedRoboticVacuumCleaner.h for an example).
class RoboticVacuumCleaner : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::OperationalState::OperationalStateCluster::Delegate & operationalStateDelegate;
        Clusters::ModeBase::AppDelegate & runModeDelegate;
        // Mode value to report on startup, using the numbering of the concrete run mode option
        // list supplied via runModeDelegate.
        uint8_t runModeStartupValue;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit RoboticVacuumCleaner(const Config & config);
    ~RoboticVacuumCleaner() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::RvcOperationalState::RvcOperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::ModeBaseCluster & RunMode() { return mRunModeCluster.Cluster(); }

protected:
    DeviceLayer::DiagnosticDataProvider & GetDiagnosticDataProvider() { return mDiagnosticDataProvider; }

    // Hook for subclasses to register device-specific optional clusters (RVC Clean Mode, Service
    // Area, ...) after the mandatory clusters above have been registered. Default: none added.
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }

    // Counterpart to RegisterOptionalClusters(): a subclass that overrides one must override the
    // other, to tear down whatever optional clusters it added.
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

private:
    Clusters::OperationalState::OperationalStateCluster::Delegate & mOperationalStateDelegate;
    LazyRegisteredServerCluster<Clusters::RvcOperationalState::RvcOperationalStateCluster> mOperationalStateCluster;

    Clusters::ModeBase::AppDelegate & mRunModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mRunModeCluster;
    uint8_t mRunModeStartupValue;

    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
};

} // namespace chip::app
