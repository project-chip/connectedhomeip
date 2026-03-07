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
 *      Platform-agnostic BLE types used by BleChannel and BLEManagerImpl.
 *      Shared across Matter BLE and side channel implementations.
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Connection handle type for BLE operations. Platform code casts to/from
 * BLE_CONNECTION_OBJECT where required by the Matter BleLayer.
 */
using BleConnectionHandle = void *;

/**
 * BLE event types delivered from platform stack to channel (ParseEvent)
 * and then to BLEManagerImpl via callback (OnMatterBleEvent).
 */
enum class BleEventType : uint32_t
{
    kConnectionOpened,
    kConnectionClosed,
    kConnectionParameters,
    kMtuExchanged,
    kGattWrite,
    kGattRead,
    kIndicationConfirmation,
    kSoftTimerExpired,
    kBoot,
    kUnknown
};

/**
 * Unified BLE event passed from channel to BLEManagerImpl.
 */
struct BleEvent
{
    BleEventType type              = BleEventType::kUnknown;
    BleConnectionHandle connection = nullptr;
    void * data                    = nullptr;
    size_t dataLength              = 0;
    uint16_t attributeHandle       = 0;
    uint32_t timerId               = 0;
};

/**
 * Advertising parameters for StartAdvertising.
 */
struct BleAdvertisingParams
{
    uint16_t minInterval    = 0;
    uint16_t maxInterval    = 0;
    bool connectable        = true;
    uint8_t * customData    = nullptr;
    size_t customDataLength = 0;
};

/**
 * Connection parameters for UpdateConnectionParams.
 */
struct BleConnectionParams
{
    uint16_t minInterval = 0;
    uint16_t maxInterval = 0;
    uint16_t latency     = 0;
    uint16_t timeout     = 0;
};

/**
 * Event callback type: channel calls this when it has parsed a platform event
 * and produced a BleEvent (e.g. for Matter BLE).
 */
using BleEventCallback = void (*)(const BleEvent & event, void * context);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
