/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/server/Server.h>

namespace matter {
namespace casting {
namespace support {

/**
 * @brief Handles chip::DeviceLayer::ChipDeviceEvent events (such as kFailSafeTimerExpired, kBindingsChangedViaCluster,
 * kCommissioningComplete) sent by the Matter DeviceLayer.
 * ChipDeviceEventHandler helps the CastingPlayer class commission with and connect to a CastingPlayer
 */
class ChipDeviceEventHandler
{
public:
    /**
     * @brief Top level handler to handle chip::DeviceLayer::ChipDeviceEvent. Delegates to HandleBindingsChangedViaCluster and other
     * handlers
     */
    static void Handle(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    /**
     * @brief Sets internal ChipDeviceEventHandler state to indicate that User Directed Commissioning (UDC) is in progress.
     * If UDC was already in progress when this method was called, it will return a CHIP_ERROR_INCORRECT_STATE without changing the
     * internal state.
     */
    static CHIP_ERROR SetUdcStatus(bool udcInProgress);

private:
    /**
     * @brief if kFailSafeTimerExpired is received and a request to connect to a CastingPlayer is pending, open a basic
     * commissioning window and send the user directed commissioning request to the target CastingPlayer
     */
    static void HandleFailSafeTimerExpired();

    /**
     * @brief Handles cases where the tv-casting-app is being "re-commissioned" from a CastingPlayer that it was previously
     * commissioned by.
     */
    static void HandleBindingsChangedViaCluster(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg,
                                                bool & runPostCommissioning, chip::NodeId & targetPeerNodeId,
                                                chip::FabricIndex & targetFabricIndex);

    /**
     * @brief Handles kCommissioningComplete event which is received when commissioning completes
     */
    static void HandleCommissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg,
                                            bool & runPostCommissioning, chip::NodeId & targetPeerNodeId,
                                            chip::FabricIndex & targetFabricIndex);

    static bool sUdcInProgress;
};

}; // namespace support
}; // namespace casting
}; // namespace matter
