/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/proximity-ranging-server/ProximityRangingDriver.h>

#include "RangingTechnologyController.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

/**
 * ProximityRangingDriver implementation that routes ranging operations through
 * a RangingTechnologyController and forwards async results back to the cluster.
 */
class DefaultProximityRangingDriver : public ProximityRangingDriver, public RangingTechnologyController::Listener
{
public:
    explicit DefaultProximityRangingDriver(RangingTechnologyController & controller) : mController(controller) {}

    // ProximityRangingDriver implementation
    CHIP_ERROR Init(Callback & callback) override;
    void Shutdown() override;
    ResultCodeEnum HandleStartRanging(uint8_t sessionId, const Commands::StartRangingRequest::DecodableType & request) override;
    CHIP_ERROR HandleStopRanging(uint8_t sessionId) override;
    CHIP_ERROR GetRangingCapabilities(AttributeValueEncoder & encoder) override;
    std::optional<BleRbcConfig> GetBleRbcConfig() override;
    size_t GetNumActiveSessionIds() override { return mController.GetNumActiveSessionIds(); }
    CHIP_ERROR GetActiveSessionIds(Span<uint8_t> & sessionIds) override;

    // RangingTechnologyController::Listener implementation
    void OnSessionStopped(uint8_t sessionId, RangingSessionStatusEnum status) override;
    void OnSessionMeasurement(uint8_t sessionId, const Structs::RangingMeasurementDataStruct::Type & measurement) override;
    void OnAttributeChanged(AttributeId attributeId) override;

private:
    RangingTechnologyController & mController;
    Callback * mCallback = nullptr;
};

} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
