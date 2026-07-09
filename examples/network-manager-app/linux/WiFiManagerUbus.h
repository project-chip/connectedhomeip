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

#pragma once

#include "UbusManager.h"
#include <app/clusters/wifi-network-management-server/wifi-network-management-server.h>
#include <string>

namespace chip {

class WiFiManagerUbus
{
public:
    WiFiManagerUbus(ubus::UbusManager & ubusManager, app::Clusters::WiFiNetworkManagementServer & server) :
        mUbusManager(ubusManager), mServer(server), mDesiredRadio("")
    {}

    void Init();

private:
    void InvokeUciGetWifiIfaces(void);
    void OnWirelessNetworksUpdate(blob_attr * msg);
    void OnPreferencesUpdate(blob_attr * msg);
    bool GetUciBlob(const char * config, const char * type, blob_attr ** blob);

    ubus::UbusManager & mUbusManager;
    app::Clusters::WiFiNetworkManagementServer & mServer;
    ubus::UbusWatch mUci{ "uci", this };
    ubus::UbusWatch mService{ "service", this };
    std::string mDesiredRadio;
};

} // namespace chip
