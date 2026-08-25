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
#include <clusters/RvcRunMode/Enums.h>
#include <functional>

namespace chip::app::Clusters::ServiceArea {
class LoggingServiceAreaDelegate;
} // namespace chip::app::Clusters::ServiceArea

namespace chip::app::Clusters::RvcRunMode {

class LoggingRvcRunModeDelegate : public ModeBase::AppDelegate
{
public:
    // The RVC Operational State and Service Area clusters are mandatory for this device type and
    // always exist before this delegate is constructed, so they are injected by reference.
    LoggingRvcRunModeDelegate(OperationalState::OperationalStateCluster & operationalStateCluster,
                              ServiceArea::LoggingServiceAreaDelegate & serviceAreaDelegate) :
        mOperationalStateCluster(operationalStateCluster), mServiceAreaDelegate(serviceAreaDelegate)
    {}

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<detail::Structs::ModeTagStruct::Type> & modeTags) override;
    void HandleChangeToMode(uint8_t newMode, ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    // Bound after construction: this delegate is constructed before the RunMode cluster it backs,
    // so the self-reference cannot be injected at construction time without a cycle.
    void SetCluster(ModeBaseCluster * cluster) { mCluster = cluster; }
    void SetClearDockChargingTrackingHandler(std::function<void()> handler)
    {
        mClearDockChargingTrackingHandler = std::move(handler);
    }

private:
    ModeBaseCluster * mCluster = nullptr;
    OperationalState::OperationalStateCluster & mOperationalStateCluster;
    ServiceArea::LoggingServiceAreaDelegate & mServiceAreaDelegate;
    std::function<void()> mClearDockChargingTrackingHandler;
};

} // namespace chip::app::Clusters::RvcRunMode
