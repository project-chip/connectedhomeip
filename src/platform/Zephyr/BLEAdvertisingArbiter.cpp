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

#include "BLEAdvertisingArbiter.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemError.h>
#include <zephyr/bluetooth/conn.h>

namespace chip {
namespace DeviceLayer {
namespace BLEAdvertisingArbiter {
namespace {

// List of advertising requests ordered by priority
sys_slist_t sRequests;

bool sIsInitialized = false;
uint8_t sBtId       = 0;

// TODO: Remove this if when all vendors have updated to Zephyr 3.6 or later.
#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
bool sWasDisconnection = false; // Tracks if a recent disconnection might require an advertising restart.
#endif                          // CHIP_DEVICE_LAYER_TARGET_NRFCONNECT

// Cast an intrusive list node to the containing request object
const BLEAdvertisingArbiter::Request & ToRequest(const sys_snode_t * node)
{
    return *static_cast<const BLEAdvertisingArbiter::Request *>(node);
}

// Notify application about stopped advertising if the callback has been provided
void NotifyAdvertisingStopped(const sys_snode_t * node)
{
    VerifyOrReturn(node);

    const Request & request = ToRequest(node);

    if (request.onStopped != nullptr)
    {
        request.onStopped();
    }
}

// Restart advertising using the top-priority request
CHIP_ERROR RestartAdvertising()
{
    // Note: bt_le_adv_stop() returns success when the advertising was not started
    ReturnErrorOnFailure(System::MapErrorZephyr(bt_le_adv_stop()));
    VerifyOrReturnError(!sys_slist_is_empty(&sRequests), CHIP_NO_ERROR);

    const Request & top    = ToRequest(sys_slist_peek_head(&sRequests));
    bt_le_adv_param params = BT_LE_ADV_PARAM_INIT(top.options, top.minInterval, top.maxInterval, nullptr);
    params.id              = sBtId;
    const int result = bt_le_adv_start(&params, top.advertisingData.data(), top.advertisingData.size(), top.scanResponseData.data(),
                                       top.scanResponseData.size());

    if (result == -ENOMEM)
    {
        ChipLogProgress(DeviceLayer, "Advertising start failed, will retry once connection is released");
    }

    if (top.onStarted != nullptr)
    {
        top.onStarted(result);
    }

    return System::MapErrorZephyr(result);
}

// In nrfconnect we must use the recycled callback to restart advertising after a disconnection.
// The callback is available since Zephyr 3.6.
// TODO: Remove this if when all vendors have updated to Zephyr 3.6 or later.
#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .disconnected = [](struct bt_conn * conn, uint8_t reason) { sWasDisconnection = true; },
    .recycled =
        []() {
            // In this callback the connection object was returned to the pool and we can try to re-start connectable
            // advertising, but only if the disconnection was detected.
            if (sWasDisconnection)
            {
                SystemLayer().ScheduleLambda([] {
                    if (!sys_slist_is_empty(&sRequests))
                    {
                        // Starting from Zephyr 4.0 Automatic advertiser resumption is deprecated,
                        // so the BLE Advertising Arbiter has to take over the responsibility of restarting the advertiser.
                        // Restart advertising in this callback if there are pending requests after the connection is released.
                        CHIP_ERROR advRestartErr = RestartAdvertising();
                        if (advRestartErr != CHIP_NO_ERROR)
                        {
                            ChipLogError(DeviceLayer, "BLE advertising restart failed: %" CHIP_ERROR_FORMAT,
                                         advRestartErr.Format());
                        }
                    }
                });
                // Reset the disconnection flag to avoid restarting advertising multiple times
                sWasDisconnection = false;
            }
        },
};
#endif // CHIP_DEVICE_LAYER_TARGET_NRFCONNECT

} // namespace

CHIP_ERROR Init(uint8_t btId)
{
    if (sIsInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    sBtId          = btId;
    sIsInitialized = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR InsertRequest(Request & request)
{
    if (!sIsInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CancelRequest(request);

    sys_snode_t * prev = nullptr;
    sys_snode_t * node = nullptr;

    // Find position of the request in the list that preserves ordering by priority
    SYS_SLIST_FOR_EACH_NODE(&sRequests, node)
    {
        if (request.priority < ToRequest(node).priority)
        {
            break;
        }

        prev = node;
    }

    if (prev == nullptr)
    {
        NotifyAdvertisingStopped(sys_slist_peek_head(&sRequests));
        sys_slist_prepend(&sRequests, &request);
    }
    else
    {
        sys_slist_insert(&sRequests, prev, &request);
    }

    // If the request is top-priority, restart the advertising
    if (sys_slist_peek_head(&sRequests) == &request)
    {
        return RestartAdvertising();
    }

    return CHIP_NO_ERROR;
}

void CancelRequest(Request & request)
{
    if (!sIsInitialized)
    {
        return;
    }

    const bool isTopPriority = (sys_slist_peek_head(&sRequests) == &request);
    VerifyOrReturn(sys_slist_find_and_remove(&sRequests, &request));

    // If cancelled request was top-priority, restart the advertising.
    if (isTopPriority)
    {
        TEMPORARY_RETURN_IGNORED RestartAdvertising();
    }
}

} // namespace BLEAdvertisingArbiter
} // namespace DeviceLayer
} // namespace chip
