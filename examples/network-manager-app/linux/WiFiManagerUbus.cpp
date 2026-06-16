/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "WiFiManagerUbus.h"

#include "UboxUtils.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

#include <libubus.h>

using namespace chip::ubus;

// Optionally enable verbose notification and UCI response debugging
// #define CHIP_WIFI_MANAGER_UBUS_DEBUGGING

namespace chip {

static constexpr int kInvokeTimeout = 2000;

void WiFiManagerUbus::InvokeUciGetWifiIfaces()
{
    BlobMsgBuf buf;
    buf.Add("config", "wireless");
    buf.Add("type", "wifi-iface");
    if (buf.HasError())
    {
        ChipLogError(AppServer, "WiFiManagerUbus: failed to build uci get request");
        return;
    }
    int status = mUbusManager.Invoke(
        mUci, "get", buf.head,
        [](ubus_request * req, int /*type*/, blob_attr * msg) {
            static_cast<WiFiManagerUbus *>(req->priv)->OnUciGetResponse(msg);
        },
        this, kInvokeTimeout);
    if (status != UBUS_STATUS_OK)
    {
        ChipLogError(AppServer, "WiFiManagerUbus: uci get failed: %s", ubus_strerror(status));
    }
}

void WiFiManagerUbus::Init()
{
    mUci.SetResolvedCallback([](UbusWatch & /*watch*/, void * appState) {
        static_cast<WiFiManagerUbus *>(appState)->InvokeUciGetWifiIfaces();
    });

    mService.SetResolvedCallback([](UbusWatch & /*watch*/, void * /*appState*/) {
        ChipLogDetail(AppServer, "WiFiManagerUbus: service object resolved");
    });

    mService.SetNotificationCallback(
        [](UbusWatch & /*watch*/, void * appState, ubus_request_data * /*req*/, const char * notification, blob_attr * msg) {
#ifdef CHIP_WIFI_MANAGER_UBUS_DEBUGGING
            ChipLogProgress(AppServer, "WiFiManagerUbus: service notification '%s'", notification);
            blob_attr * dbgCur;
            int dbgRem;
            blobmsg_for_each_attr(dbgCur, msg, dbgRem)
            {
                if (blobmsg_type(dbgCur) == BLOBMSG_TYPE_STRING)
                {
                    ChipLogProgress(AppServer, "  [string] %s = %s", blobmsg_name(dbgCur), blobmsg_get_string(dbgCur));
                }
                else
                {
                    ChipLogProgress(AppServer, "  [type=%d] %s", blobmsg_type(dbgCur), blobmsg_name(dbgCur));
                }
            }
#endif

            BlobMsgRequiredField<const char *, CHIP_CTST("type")> eventType;
            if (!BlobMsgParse(msg, eventType))
            {
                ChipLogError(AppServer, "WiFiManagerUbus: failed to parse service event");
                return;
            }
#ifdef CHIP_WIFI_MANAGER_UBUS_DEBUGGING
            ChipLogProgress(AppServer, "WiFiManagerUbus: event type = '%s'", eventType.value());
#endif
            if (strcmp(eventType.value(), "netifd.wireless.done") != 0)
                return;

            static_cast<WiFiManagerUbus *>(appState)->InvokeUciGetWifiIfaces();
        });

    mUbusManager.Register(mUci);
    mUbusManager.Register(mService);
}

void WiFiManagerUbus::OnUciGetResponse(blob_attr * msg)
{
    // Response: { "values": { "<section>": { ".type": "wifi-device"|"wifi-iface", "ssid": "...", "key": "...", ... } } }
    blob_attr * values = nullptr;
    blob_attr * cur;
    int rem;
    blobmsg_for_each_attr(cur, msg, rem)
    {
        if (blobmsg_type(cur) == BLOBMSG_TYPE_TABLE && strcmp(blobmsg_name(cur), "values") == 0)
        {
            values = cur;
            break;
        }
    }
    VerifyOrReturn(values != nullptr);

#ifdef CHIP_WIFI_MANAGER_UBUS_DEBUGGING
    ChipLogProgress(AppServer, "uci wireless values fields:");
    blobmsg_for_each_attr(cur, static_cast<blob_attr *>(blobmsg_data(values)), rem)
    {
        if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING)
        {
            ChipLogProgress(AppServer, "  [string] %s = %s", blobmsg_name(cur), blobmsg_get_string(cur));
        }
        else
        {
            ChipLogProgress(AppServer, "  [type=%d] %s", blobmsg_type(cur), blobmsg_name(cur));
        }
    }
#endif

    // "values" is a TABLE; its data pointer points directly to the first section blob_attr
    blob_attr * section = static_cast<blob_attr *>(blobmsg_data(values));
    VerifyOrReturn(section != nullptr);

    BlobMsgField<const char *, CHIP_CTST("mode")> mode;
    BlobMsgRequiredField<const char *, CHIP_CTST("ssid")> ssid;
    BlobMsgRequiredField<const char *, CHIP_CTST("key")> key;
    if (!BlobMsgParse(section, mode, ssid, key))
    {
        ChipLogError(AppServer, "No ap-mode wifi-iface with ssid+key found in uci wireless config");
        return;
    }
    if (mode.has_value() && strcmp(mode.value(), "ap") != 0)
    {
        ChipLogError(AppServer, "wifi-iface mode is not ap");
        return;
    }

    CHIP_ERROR err = mServer.SetNetworkCredentials(
        ByteSpan(reinterpret_cast<const uint8_t *>(ssid.value()), strlen(ssid.value())),
        ByteSpan(reinterpret_cast<const uint8_t *>(key.value()), strlen(key.value())));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetNetworkCredentials failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

} // namespace chip
