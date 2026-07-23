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

#include <app/clusters/operational-state-server/RvcOperationalStateCluster.h>
#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <device/api/SingleEndpoint.h>
#include <device/capabilities/operational-state/impl/LoggingRvcOperationalStateDelegate.h>
#include <device/capabilities/service-area/impl/LoggingServiceAreaDelegate.h>
#include <device/capabilities/service-area/impl/LoggingServiceAreaStorageDelegate.h>

namespace chip::app {

class RoboticVacuumCleaner : public SingleEndpoint
{
public:
    RoboticVacuumCleaner();
    ~RoboticVacuumCleaner() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::RvcOperationalState::RvcOperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::ServiceArea::ServiceAreaCluster & ServiceArea() { return mServiceAreaCluster.Cluster(); }

private:
    Clusters::OperationalState::LoggingRvcOperationalStateDelegate mDelegate;
    LazyRegisteredServerCluster<Clusters::RvcOperationalState::RvcOperationalStateCluster> mOperationalStateCluster;

    Clusters::ServiceArea::LoggingServiceAreaStorageDelegate mServiceAreaStorageDelegate;
    Clusters::ServiceArea::LoggingServiceAreaDelegate mServiceAreaDelegate;
    LazyRegisteredServerCluster<Clusters::ServiceArea::ServiceAreaCluster> mServiceAreaCluster;
};

} // namespace chip::app
