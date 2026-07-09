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
#ifdef CHIP_WIFI_MANAGER_UBUS_DEBUGGING
#define ChipWifiDebug(...) ChipLogProgress(AppServer, __VA_ARGS__)
#else
#define ChipWifiDebug(...)
#endif

namespace chip {

static constexpr int kInvokeTimeout = 2000;

static inline std::string debug_blob_msg(blob_attr * attr)
{
    std::string s;
    const char * json = blobmsg_format_json(msg, true);
    if (json)
    {
        s = std::string(json);
        free(json);
    }

    return std::string{};
}

bool WiFiManagerUbus::GetUciBlob(const char * config, const char * type, blob_attr ** blob)
{
    BlobMsgBuf buf;
    buf.Add("config", config);
    buf.Add("type", type);
    if (buf.HasError())
    {
        ChipLogError(AppServer, "WiFiManagerUbus: failed to build uci get request");
        return false;
    }
    int status = mUbusManager.Invoke(
        mUci, "get", buf.head,
        [](ubus_request * req, int /*type*/, blob_attr * msg) {
            blob_attr ** blob_ptr = static_cast<blob_attr **>(req->priv);
            *blob_ptr             = blob_memdup(msg);
        },
        blob, kInvokeTimeout);
    if (status != UBUS_STATUS_OK)
    {
        ChipLogError(AppServer, "WiFiManagerUbus: uci get failed: %s", ubus_strerror(status));
        return false;
    }
    if (!*blob)
    {
        ChipLogError(AppServer, "WiFiManagerUbus: ubus Invoke failed or memory issue");
        return false;
    }
    return true;
}

void WiFiManagerUbus::InvokeUciGetWifiIfaces(void)
{
    blob_attr * matter_config   = nullptr;
    blob_attr * wireless_config = nullptr;

    if (!GetUciBlob("matter", "struct", &matter_config))
    {
        ChipLogError(AppServer, "Unable to fetch matter config, not setting default interface");
    }
    else
    {
        OnPreferencesUpdate(matter_config);
    }
    if (!GetUciBlob("wireless", "wifi-iface", &wireless_config))
    {
        ChipLogError(AppServer, "Unable to fetch wireless config, can't configure matter!");
    }
    else
    {
        OnWirelessNetworksUpdate(wireless_config);
    }
    free(matter_config);
    free(wireless_config);
}

void WiFiManagerUbus::Init()
{
    mUci.SetResolvedCallback(
        [](UbusWatch & /*watch*/, void * appState) { static_cast<WiFiManagerUbus *>(appState)->InvokeUciGetWifiIfaces(); });

    mService.SetResolvedCallback(
        [](UbusWatch & /*watch*/, void * /*appState*/) { ChipLogDetail(AppServer, "WiFiManagerUbus: service object resolved"); });

    mService.SetNotificationCallback(
        [](UbusWatch & /*watch*/, void * appState, ubus_request_data * /*req*/, const char * notification, blob_attr * msg) {
#ifdef CHIP_WIFI_MANAGER_UBUS_DEBUGGING
            ChipWifiDebug("WiFiManagerUbus: service notification '%s'", notification);
            blob_attr * dbgCur;
            int dbgRem;
            blobmsg_for_each_attr(dbgCur, msg, dbgRem)
            {
                if (blobmsg_type(dbgCur) == BLOBMSG_TYPE_STRING)
                {
                    ChipWifiDebug("  [string] %s = %s", blobmsg_name(dbgCur), blobmsg_get_string(dbgCur));
                }
                else
                {
                    ChipWifiDebug("  [type=%d] %s", blobmsg_type(dbgCur), blobmsg_name(dbgCur));
                }
            }
#endif

            BlobMsgRequiredField<const char *, CHIP_CTST("type")> eventType;
            if (!BlobMsgParse(msg, eventType))
            {
                ChipLogError(AppServer, "WiFiManagerUbus: failed to parse service event");
                return;
            }
            ChipWifiDebug("WiFiManagerUbus: event type = '%s'", eventType.value());
            if (strcmp(eventType.value(), "netifd.wireless.done") != 0)
                return;

            static_cast<WiFiManagerUbus *>(appState)->InvokeUciGetWifiIfaces();
        });

    mUbusManager.Register(mUci);
    mUbusManager.Register(mService);
    ChipLogProgress(AppServer, "WiFiManagerUbus: init done");
}

void WiFiManagerUbus::OnPreferencesUpdate(blob_attr * msg)
{
    // {"values":{"values":{".anonymous":false,".type":"struct",".name":"values",".index":1,"wifi_iface":"default_radio0"}}}

    blob_attr * values = nullptr;
    blob_attr * cur    = nullptr;
    blob_attr * i      = nullptr;
    int rem            = 0;
    int r              = 0;

    ChipLogProgress(AppServer, "preferences = %s", debug_blob_msg(msg).c_str());

    blobmsg_for_each_attr(cur, msg, rem)
    {
        if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE || strcmp(blobmsg_name(cur), "values") != 0)
        {
            ChipLogError(AppServer, "Invalid data received (%s)", blobmsg_name(cur));
            return;
        }
        values = cur;
        break;
    }
    VerifyOrReturn(values != nullptr, ChipLogError(AppServer, "Invalid uci return value (no 'values' table found)"));

    blobmsg_for_each_attr(cur, values, rem)
    {
        ChipWifiDebug("Found section: %s", blobmsg_name(cur));
        blobmsg_for_each_attr(i, cur, r)
        {
            if (blobmsg_type(i) == BLOBMSG_TYPE_STRING)
            {
                ChipWifiDebug("  [string] %s = %s", blobmsg_name(i), blobmsg_get_string(i));
                if (strcmp(blobmsg_name(i), "wifi_iface") == 0)
                {
                    mDesiredRadio = std::string(blobmsg_get_string(i));
                    ChipWifiDebug("Preferences: desired radio: %s", mDesiredRadio.c_str());
                }
            }
            else
            {
                ChipWifiDebug("  [type=%d] %s", blobmsg_type(i), blobmsg_name(i));
            }
        }
    }
}

void WiFiManagerUbus::OnWirelessNetworksUpdate(blob_attr * msg)
{
    // Response: { "values": { "<section>": { ".type": "wifi-device"|"wifi-iface", "ssid": "...", "key": "...", ... } } }
    blob_attr * values = nullptr;
    blob_attr * cur    = nullptr;
    blob_attr * i      = nullptr;
    blob_attr * radio  = nullptr;
    int rem            = 0;
    int r              = 0;

    ChipLogProgress(AppServer, "radios = %s", debug_blob_msg(msg).c_str());

    if (!mDesiredRadio.empty())
    {
        ChipLogProgress(AppServer, "Looking for radio '%s'", mDesiredRadio.c_str());
    }

    blobmsg_for_each_attr(cur, msg, rem)
    {
        if (blobmsg_type(cur) != BLOBMSG_TYPE_TABLE || strcmp(blobmsg_name(cur), "values") != 0)
        {
            ChipLogError(AppServer, "Invalid data received (%s)", blobmsg_name(cur));
            return;
        }
        values = cur;
        break;
    }
    VerifyOrReturn(values != nullptr, ChipLogError(AppServer, "Invalid uci return value (no 'values' table found)"));

    blobmsg_for_each_attr(cur, values, rem)
    {
        bool station_mode = false;
        ChipLogProgress(AppServer, "Found radio: %s", blobmsg_name(cur));
        blobmsg_for_each_attr(i, cur, r)
        {
            if (blobmsg_type(i) == BLOBMSG_TYPE_STRING)
            {
                ChipWifiDebug("  [string] %s = %s", blobmsg_name(i), blobmsg_get_string(i));
                if (strcmp(blobmsg_name(i), "mode") == 0 && strcmp(blobmsg_get_string(i), "ap") != 0)
                {
                    ChipLogProgress(AppServer, "Found non-AP radio, SKIPPING");
                    station_mode = true;
                }
            }
            else
            {
                ChipWifiDebug("  [type=%d] %s", blobmsg_type(i), blobmsg_name(i));
            }
        }
        if (!station_mode && (mDesiredRadio.empty() || mDesiredRadio == blobmsg_name(cur)))
        {
            radio = cur;
            break;
        }
    }
    VerifyOrReturn(radio != nullptr, ChipLogError(AppServer, "No valid radio found!"));
    char * ssid = nullptr;
    char * key  = nullptr;

    blobmsg_for_each_attr(cur, radio, rem)
    {
        if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING && strcmp(blobmsg_name(cur), "ssid") == 0)
        {
            ssid = blobmsg_get_string(cur);
        }
        if (blobmsg_type(cur) == BLOBMSG_TYPE_STRING && strcmp(blobmsg_name(cur), "key") == 0)
        {
            key = blobmsg_get_string(cur);
        }
    }
    VerifyOrReturn(ssid != nullptr, ChipLogError(AppServer, "Radio %s does not have a set SSID!", blobmsg_name(radio)));
    VerifyOrReturn(key != nullptr, ChipLogError(AppServer, "Radio %s does not have a set key!", blobmsg_name(radio)));

    ChipLogProgress(AppServer, "Chosen radio '%s' has SSID '%s'", blobmsg_name(radio), ssid);

    CHIP_ERROR err = mServer.SetNetworkCredentials(ByteSpan(reinterpret_cast<const uint8_t *>(ssid), strlen(ssid)),
                                                   ByteSpan(reinterpret_cast<const uint8_t *>(key), strlen(key)));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "SetNetworkCredentials failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

} // namespace chip
