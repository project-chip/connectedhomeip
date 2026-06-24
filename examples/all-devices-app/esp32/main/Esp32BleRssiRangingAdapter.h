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

#include <devices/proximity-ranger/impl/BleRssiRangingAdapter.h>
#include <lib/support/logging/CHIPLogging.h>

/**
 * ESP32-specific BLE RSSI ranging adapter.
 * Stub implementation until platform BLE support is ready.
 */
class Esp32BleRssiRangingAdapter : public BleRssiRangingAdapter
{
public:
    chip::app::Clusters::ProximityRanging::ResultCodeEnum
    StartSession(uint8_t sessionId,
                 const chip::app::Clusters::ProximityRanging::Commands::StartRangingRequest::DecodableType & request) override
    {
        ChipLogProgress(AppServer, "Esp32BleRssiRangingAdapter: StartSession stub (session %u)", sessionId);
        return chip::app::Clusters::ProximityRanging::ResultCodeEnum::kAccepted;
    }

    CHIP_ERROR StopSession(uint8_t sessionId) override
    {
        ChipLogProgress(AppServer, "Esp32BleRssiRangingAdapter: StopSession stub (session %u)", sessionId);
        if (mCallback != nullptr)
        {
            // kHardwareError is used as a placeholder for all stop reasons until the
            // specification defines a distinct status for deliberate stops.
            mCallback->OnRangingSessionStopped(sessionId,
                                               chip::app::Clusters::ProximityRanging::RangingSessionStatusEnum::kHardwareError);
        }
        return CHIP_NO_ERROR;
    }

    void StopAllSessions() override { ChipLogProgress(AppServer, "Esp32BleRssiRangingAdapter: StopAllSessions stub"); }

    CHIP_ERROR GetActiveSessionIds(std::vector<uint8_t> & sessionIds) override { return CHIP_NO_ERROR; }
};
