/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/support/Span.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/sys/slist.h>

#include <cstdint>

/**
 * @file
 *   Bluetooth LE advertising arbiter.
 *
 * The purpose for this module is to coordinate BLE advertising between
 * different application components.
 *
 * An application component that wants to advertise BLE services is expected to
 * define a request with a desired priority, and pass it to the BLE advertising
 * arbiter. If there are multiple components that request BLE advertising at the
 * same time, the arbiter selects the one with the highest priority (represented
 * by the lowest numeric value) and starts the BLE advertising using parameters
 * defined in the winning request.
 *
 * The BLE arbiter does not take ownership of a submitted request, so the
 * request object must be sustained until it is cancelled by the application.
 */

namespace chip {
namespace DeviceLayer {
namespace BLEAdvertisingArbiter {

using OnAdvertisingStarted = void (*)(int result);
using OnAdvertisingStopped = void (*)();

struct Request : public sys_snode_t
{
    uint8_t priority;                     ///< Advertising request priority. Lower value means higher priority
    uint32_t options;                     ///< Advertising options: bitmask of BT_LE_ADV_OPT_XXX constants from Zephyr
    uint16_t minInterval;                 ///< Minimum advertising interval in 0.625 ms units
    uint16_t maxInterval;                 ///< Maximum advertising interval in 0.625 ms units
    Span<const bt_data> advertisingData;  ///< Advertising data fields
    Span<const bt_data> scanResponseData; ///< Scan response data fields
    OnAdvertisingStarted onStarted;       ///< (Optional) Callback invoked when the request becomes top-priority.
    OnAdvertisingStopped onStopped;       ///< (Optional) Callback invoked when the request stops being top-priority.
};

/**
 * @brief Request BLE advertising
 *
 * Add the request to the internal list of competing requests. If the request
 * has higher priority than other requests in the list, restart the BLE
 * advertising immediately using parameters defined in the new request.
 *
 * Inserting a request object that is already registered at the advertising
 * arbiter automatically cancels the previous request.
 *
 * @note This method does not take ownership of the request object so the object
 *       must not get destroyed before it is cancelled.
 *
 * @param request   Reference to advertising request that contains priority and
 *                  other advertising parameters.
 * @return error    If the request is top-priority and failed to restart the
 *                  advertising.
 * @return success  Otherwise.
 */
CHIP_ERROR InsertRequest(Request & request);

/**
 * @brief Cancel BLE advertising request
 *
 * Remove the request from the internal list of competing requests. If the
 * request is the winning (top-priority) one at the time of calling this
 * function, restart the BLE advertising using parameters defined in the 2nd
 * top-priority request in the list, or stop the BLE advertising completely if
 * this is the last request in the list.
 *
 * An attempt to cancel a request that has not been registered at the
 * advertising arbiter is a no-op. That is, it returns immediately.
 *
 * @param request   Reference to advertising request that contains priority and
 *                  other advertising parameters.
 */
void CancelRequest(Request & request);

} // namespace BLEAdvertisingArbiter
} // namespace DeviceLayer
} // namespace chip
