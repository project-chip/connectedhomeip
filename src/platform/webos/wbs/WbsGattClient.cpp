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

#include "WbsGattClient.h"

#include <glib.h>

#include "Helper.h"
#include "lsrequester.h"
#include <ble/Ble.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#define API_BLUETOOTH_GATT_GETSTATUS "luna://com.webos.service.bluetooth2/gatt/getStatus"
#define API_BLUETOOTH_GATT_CONNECT "luna://com.webos.service.bluetooth2/gatt/connect"
#define API_BLUETOOTH_GATT_DISCONNECT "luna://com.webos.service.bluetooth2/gatt/disconnect"

namespace chip {
namespace DeviceLayer {
namespace Internal {

std::string WbsGattConnect(const std::string & address, int timeoutSec)
{
    pbnjson::JValue param = pbnjson::JObject();
    pbnjson::JValue response;
    param.put("address", address);
    if (!LsRequester::getInstance()->lsCallSync(API_BLUETOOTH_GATT_CONNECT, param.stringify().c_str(), response, timeoutSec))
    {
        ChipLogError(Ble, "Pre-connect gatt/connect to %s: no response within %d s", address.c_str(), timeoutSec);
        return std::string();
    }
    if (!response.hasKey(STR_RETURN_VALUE) || !response[STR_RETURN_VALUE].asBool() || !response.hasKey("clientId"))
    {
        ChipLogError(Ble, "Pre-connect gatt/connect to %s failed: %s", address.c_str(), response.stringify().c_str());
        return std::string();
    }
    return response["clientId"].asString();
}

void WbsGattDisconnect(const std::string & clientId)
{
    pbnjson::JValue param = pbnjson::JObject();
    pbnjson::JValue response;
    param.put("clientId", clientId);
    if (!LsRequester::getInstance()->lsCallSync(API_BLUETOOTH_GATT_DISCONNECT, param.stringify().c_str(), response) ||
        !response.hasKey(STR_RETURN_VALUE) || !response[STR_RETURN_VALUE].asBool())
    {
        ChipLogError(Ble, "gatt/disconnect of WBS client %s failed: %s", clientId.c_str(), response.stringify().c_str());
        return;
    }
    ChipLogProgress(Ble, "Released WBS client %s", clientId.c_str());
}

void WbsGattLogStatus(const std::string & address)
{
    // Tells whether WBS still holds a connection after a failed/timed out gatt/connect
    // (e.g. a late connect completion), which makes the following gatt/connect retries fail immediately.
    pbnjson::JValue param = pbnjson::JObject();
    pbnjson::JValue response;
    param.put("address", address);
    if (LsRequester::getInstance()->lsCallSync(API_BLUETOOTH_GATT_GETSTATUS, param.stringify().c_str(), response))
    {
        ChipLogError(Ble, "gatt/getStatus after connect failure (%s): %s", address.c_str(), response.stringify().c_str());
    }
    else
    {
        ChipLogError(Ble, "gatt/getStatus after connect failure (%s): no response", address.c_str());
    }
}

void WbsGattDisconnectAll(WbsEndpoint * endpoint)
{
    VerifyOrReturn(endpoint != nullptr);
    // The connection table is owned by the wbs layer on the Matter GLib context.
    LogErrorOnFailure(PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](WbsEndpoint * ep) {
            VerifyOrReturnError(ep->mConnectionMap != nullptr, CHIP_NO_ERROR);
            GHashTableIter iter;
            gpointer value;
            g_hash_table_iter_init(&iter, ep->mConnectionMap);
            while (g_hash_table_iter_next(&iter, nullptr, &value))
            {
                auto * conn = static_cast<WbsConnection *>(value);
                if (!conn->clientId.empty())
                {
                    WbsGattDisconnect(conn->clientId);
                }
            }
            return CHIP_NO_ERROR;
        },
        endpoint));
}

void WbsPreconnectClients::Add(const std::string & clientId)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mClientIds.push_back(clientId);
}

void WbsPreconnectClients::ReleaseAll()
{
    std::vector<std::string> clientIds;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        clientIds.swap(mClientIds);
    }
    // Disconnect outside of the lock: the LS2 calls block.
    for (const auto & clientId : clientIds)
    {
        WbsGattDisconnect(clientId);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
