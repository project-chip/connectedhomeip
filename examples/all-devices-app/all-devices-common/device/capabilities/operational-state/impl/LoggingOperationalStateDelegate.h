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

#include <app/clusters/operational-state-server/OperationalStateCluster.h>

namespace chip::app::Clusters::OperationalState {

class LoggingOperationalStateDelegate : public OperationalStateCluster::Delegate
{
public:
    LoggingOperationalStateDelegate() = default;

    // -- Delegate Interface Implementation --
    DataModel::Nullable<uint32_t> GetCountdownTime() override { return mCountdownTime; }
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;

    void HandlePauseStateCallback(GenericOperationalError & err) override;
    void HandleResumeStateCallback(GenericOperationalError & err) override;
    void HandleStartStateCallback(GenericOperationalError & err) override;
    void HandleStopStateCallback(GenericOperationalError & err) override;

    // Application API to bind the cluster instance to this delegate
    void SetCluster(OperationalStateCluster * cluster) { mCluster = cluster; }

protected:
    OperationalStateCluster * mCluster = nullptr;
    DataModel::Nullable<uint32_t> mCountdownTime;
};

} // namespace chip::app::Clusters::OperationalState
