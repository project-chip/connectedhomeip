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
#include <system/SystemError.h>

namespace chip {
namespace DeviceLayer {
namespace BLEAdvertisingArbiter {
namespace {

// List of advertising requests ordered by priority
sys_slist_t sRequests;

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
    ReturnErrorCodeIf(sys_slist_is_empty(&sRequests), CHIP_NO_ERROR);

    const Request & top          = ToRequest(sys_slist_peek_head(&sRequests));
    const bt_le_adv_param params = BT_LE_ADV_PARAM_INIT(top.options, top.minInterval, top.maxInterval, nullptr);
    const int result = bt_le_adv_start(&params, top.advertisingData.data(), top.advertisingData.size(), top.scanResponseData.data(),
                                       top.scanResponseData.size());

    if (top.onStarted != nullptr)
    {
        top.onStarted(result);
    }

    return System::MapErrorZephyr(result);
}

} // namespace

CHIP_ERROR InsertRequest(Request & request)
{
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
    const bool isTopPriority = (sys_slist_peek_head(&sRequests) == &request);
    VerifyOrReturn(sys_slist_find_and_remove(&sRequests, &request));

    // If cancelled request was top-priority, restart the advertising.
    if (isTopPriority)
    {
        RestartAdvertising();
    }
}

} // namespace BLEAdvertisingArbiter
} // namespace DeviceLayer
} // namespace chip
