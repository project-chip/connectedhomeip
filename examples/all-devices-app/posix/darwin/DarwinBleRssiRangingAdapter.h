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

#include <PosixBleRssiRangingAdapter.h>
#include <lib/support/logging/CHIPLogging.h>

/**
 * Darwin-specific BLE RSSI ranging adapter.
 * Platform-specific BLE logic will be added here.
 */
class DarwinBleRssiRangingAdapter : public PosixBleRssiRangingAdapter
{
public:
    chip::app::Clusters::ProximityRanging::ResultCodeEnum
    StartSession(uint8_t sessionId,
                 const chip::app::Clusters::ProximityRanging::Commands::StartRangingRequest::DecodableType & request) override
    {
        // TODO: Start Darwin BLE advertising/scanning
        AddSession(sessionId);
        ChipLogProgress(AppServer, "DarwinBleRssiRangingAdapter: StartSession %u", sessionId);
        return chip::app::Clusters::ProximityRanging::ResultCodeEnum::kAccepted;
    }

    CHIP_ERROR StopSession(uint8_t sessionId) override
    {
        VerifyOrReturnError(RemoveSession(sessionId), CHIP_ERROR_NOT_FOUND);
        ChipLogProgress(AppServer, "DarwinBleRssiRangingAdapter: StopSession %u", sessionId);
        // TODO: Stop Darwin BLE advertising/scanning if no sessions remain
        return CHIP_NO_ERROR;
    }
};
