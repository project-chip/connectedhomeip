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
 *      Abstract interface for a BLE channel (Matter BLE or side channel).
 *      BLEManagerImpl holds a Matter channel and an optional side channel;
 *      all platform BLE operations are delegated to the channel.
 *      No native stack headers in this interface.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "BlePlatformTypes.h"
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Abstract base for any BLE channel (Matter BLE or side channel).
 * Platform implementations: BleChannelMatterEfr32, BleChannelMatterSiWx (Matter);
 * BleSideChannelEfr32, BleSideChannelSiWx (side channel).
 */
class BleChannel
{
public:
    virtual ~BleChannel() = default;

    // ----- Initialization -----
    virtual CHIP_ERROR Init()                                             = 0;
    virtual CHIP_ERROR Shutdown()                                        = 0;

    // ----- Advertising -----
    virtual CHIP_ERROR StartAdvertising(const BleAdvertisingParams & params) = 0;
    virtual CHIP_ERROR StopAdvertising()                                 = 0;
    virtual CHIP_ERROR SetAdvertisingData(const uint8_t * data, size_t length) = 0;

    // ----- Connection -----
    virtual CHIP_ERROR CloseConnection(BleConnectionHandle conId)         = 0;
    virtual CHIP_ERROR UpdateConnectionParams(BleConnectionHandle conId,
                                              const BleConnectionParams & params) = 0;

    // ----- GATT -----
    virtual CHIP_ERROR SendIndication(BleConnectionHandle conId, uint16_t charHandle,
                                     const uint8_t * data, size_t length) = 0;
    virtual uint16_t GetMTU(BleConnectionHandle conId) const             = 0;

    // ----- Event handling -----
    /** Callback invoked when channel parses an event and produces a BleEvent (e.g. for Matter BLE). */
    virtual void SetEventCallback(BleEventCallback callback, void * context) = 0;
    /** Returns true if this channel handles the given platform event id. */
    virtual bool CanHandleEvent(uint32_t eventId) const                  = 0;
    /** Single entry point: parse platform event, optionally build BleEvent and invoke callback. */
    virtual void ParseEvent(void * platformEvent)                        = 0;

    // ----- Address / identity -----
    virtual CHIP_ERROR GetAddress(uint8_t * addr, size_t * length)        = 0;
    virtual CHIP_ERROR SetDeviceName(const char * name)                   = 0;

    // ----- Indication timer -----
    virtual CHIP_ERROR StartIndicationTimer(BleConnectionHandle conId, uint32_t timeoutMs,
                                            void (*callback)(void *), void * context) = 0;
    virtual CHIP_ERROR CancelIndicationTimer(BleConnectionHandle conId)  = 0;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
