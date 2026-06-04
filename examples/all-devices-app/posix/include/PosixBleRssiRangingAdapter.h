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

#include <vector>

/**
 * Posix base class for BLE RSSI ranging adapters.
 *
 * Provides common session tracking (GetActiveSessionIds,
 * StopAllSessions). Platform subclasses implement StartSession and StopSession
 * with actual BLE hardware interaction.
 */
class PosixBleRssiRangingAdapter : public BleRssiRangingAdapter
{
public:
    ~PosixBleRssiRangingAdapter() override;

    CHIP_ERROR GetActiveSessionIds(std::vector<uint8_t> & sessionIds) override;
    void StopAllSessions() override;

protected:
    void AddSession(uint8_t sessionId);
    bool RemoveSession(uint8_t sessionId);

    std::vector<uint8_t> mActiveSessions;
};
