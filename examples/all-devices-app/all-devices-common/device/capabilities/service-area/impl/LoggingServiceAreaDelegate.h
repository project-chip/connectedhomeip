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
#include <app/clusters/service-area-server/ServiceAreaCluster.h>
#include <app/clusters/service-area-server/service-area-delegate.h>
#include <functional>

namespace chip::app::Clusters::ServiceArea {

// Service Area delegate for the Robotic Vacuum Cleaner device type.
// Business logic is ported from examples/rvc-app/rvc-common/src/rvc-service-area-delegate.cpp.
class LoggingServiceAreaDelegate : public Delegate
{
public:
    LoggingServiceAreaDelegate() = default;

    CHIP_ERROR Init() override;

    bool IsSetSelectedAreasAllowed(MutableCharSpan & statusText) override;
    bool IsValidSelectAreasSet(const Span<const uint32_t> & selectedAreas, SelectAreasStatus & locationStatus,
                               MutableCharSpan & statusText) override;
    bool HandleSkipArea(uint32_t skippedArea, MutableCharSpan & skipStatusText) override;
    bool IsSupportedAreasChangeAllowed() override;
    bool IsSupportedMapChangeAllowed() override;

    void SetCluster(ServiceAreaCluster * cluster) { mCluster = cluster; }
    void SetOperationalStateCluster(OperationalState::OperationalStateCluster * cluster) { mOperationalStateCluster = cluster; }

    // Called when cleaning finishes after the last area is skipped/completed.
    void SetActivityCompleteHandler(std::function<void()> handler) { mActivityCompleteHandler = std::move(handler); }

    void SetMapTopology();
    void SetAttributesAtCleanStart();
    void GoToNextArea(OperationalStatusEnum currentAreaOpState, bool & finished);
    void UpdateProgressOnExit();

private:
    ServiceAreaCluster * mCluster                                           = nullptr;
    OperationalState::OperationalStateCluster * mOperationalStateCluster    = nullptr;
    std::function<void()> mActivityCompleteHandler;
};

} // namespace chip::app::Clusters::ServiceArea
