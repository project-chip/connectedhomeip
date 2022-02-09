/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 * @file StatusScreen.cpp
 *
 * Screen which displays device state and status:
 *      - status : Operational | Commissionable
 *      - IP : IPv6 address, UDP port
 */

#include "StatusScreen.h"

#if CONFIG_HAVE_DISPLAY

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

class StatusListModel : public ListScreen::Model
{
public:
    StatusListModel()
    {
        options.emplace_back("state : commissioning");
        options.emplace_back("IPv4 : none");
        options.emplace_back("IPv6 : none");
        options.emplace_back("UDP port : 5540");
        options.emplace_back("Node ID : none");
        options.emplace_back("Fabric ID : none");
    }

    virtual std::string GetTitle() { return "Status"; }

    virtual int GetItemCount() { return options.size(); }
    virtual std::string GetItemText(int i) { 
        std::string itemString = options.at(i);
        //case (i) {}
        ESP_LOGI("M5 UI", "Display status %d: %s", i, itemString.c_str());
        return itemString;
    }

    virtual void ItemAction(int i)
    {
    }

private:
    std::vector<std::string> options;
};

StatusScreen::StatusScreen() : ListScreen(chip::Platform::New<StatusListModel>())
{
}


#endif // CONFIG_HAVE_DISPLAY
