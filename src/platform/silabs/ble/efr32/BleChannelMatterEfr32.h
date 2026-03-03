/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 *    @file
 *      Matter BLE channel implementation for EFR32.
 */

#pragma once

#include "BleChannel.h"
#include "BlePlatformTypes.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BleChannelMatterEfr32 : public BleChannel
{
public:
    BleChannelMatterEfr32()          = default;
    ~BleChannelMatterEfr32() override = default;

    CHIP_ERROR Init() override;
    CHIP_ERROR Shutdown() override;

    CHIP_ERROR StartAdvertising(const BleAdvertisingParams & params) override;
    CHIP_ERROR StopAdvertising() override;
    CHIP_ERROR SetAdvertisingData(const uint8_t * data, size_t length) override;

    CHIP_ERROR CloseConnection(BleConnectionHandle conId) override;
    CHIP_ERROR UpdateConnectionParams(BleConnectionHandle conId,
                                      const BleConnectionParams & params) override;

    CHIP_ERROR SendIndication(BleConnectionHandle conId, uint16_t charHandle,
                              const uint8_t * data, size_t length) override;
    uint16_t GetMTU(BleConnectionHandle conId) const override;

    void SetEventCallback(BleEventCallback callback, void * context) override;
    bool CanHandleEvent(uint32_t eventId) const override;
    void ParseEvent(void * platformEvent) override;

    CHIP_ERROR GetAddress(uint8_t * addr, size_t * length) override;
    CHIP_ERROR SetDeviceName(const char * name) override;

    CHIP_ERROR StartIndicationTimer(BleConnectionHandle conId, uint32_t timeoutMs,
                                    void (*callback)(void *), void * context) override;
    CHIP_ERROR CancelIndicationTimer(BleConnectionHandle conId) override;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
