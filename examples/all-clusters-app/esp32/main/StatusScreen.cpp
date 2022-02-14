/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      - Fabric Index : none | #
 *      - Fabric ID : none | #
 *      - Node ID : none | #
 *      - UDP Port : #
 *      - IPv4 : none | addr
 *      - IPv6 LL : none | addr
 *      - IPv6 ULA : none | addr
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

#include <app/server/Server.h>
#include <lib/core/DataModelTypes.h>

#define MAX_LENGTH_SMALL_FONT 30

using namespace chip;

class StatusListModel : public ListScreen::Model
{
public:
    enum
    {
        kStatusItemFabricIndex,
        kStatusItemFabricId,
        kStatusItemNodeId,
        kStatusItemPort,
        kStatusItemIp4,
        kStatusItemIp6LinkLocal,
        kStatusItemIp6Ula,
    };

    StatusListModel()
    {
        options.emplace_back("FabricIndex : none");
        options.emplace_back("FabricID : none");
        options.emplace_back("NodeID : none");
        options.emplace_back("UDP port : " + std::to_string(CHIP_PORT));
        options.emplace_back("IPv4 : none");
        options.emplace_back("IPv6 LL : none");
        options.emplace_back("IPv6 ULA : none");
    }

    virtual std::string GetTitle() { return "Status"; }

    virtual int GetItemCount() { return options.size(); }
    virtual std::string GetItemText(int i)
    {
        std::string itemString = options.at(i);
        switch (i)
        {
        case kStatusItemFabricIndex: {
            for (const auto & fb : Server::GetInstance().GetFabricTable())
            {
                FabricIndex fabricIndex = fb.GetFabricIndex();
                itemString              = "FabricIdx : " + std::to_string(fabricIndex);
                break; // Only print first fabric for now
            }
            break;
        }

        case kStatusItemFabricId: {
            for (const auto & fb : Server::GetInstance().GetFabricTable())
            {
                FabricId fabricId = fb.GetFabricId();
                itemString        = "FabricID : " + std::to_string(fabricId);
                break; // Only print first fabric for now
            }
            break;
        }

        case kStatusItemNodeId: {
            for (const auto & fb : Server::GetInstance().GetFabricTable())
            {
                NodeId nodeId = fb.GetNodeId();
                itemString    = "NodeID : " + std::to_string(nodeId);
                break; // Only print first fabric for now
            }
            break;
        }

        case kStatusItemIp4: {
            chip::Inet::IPAddress addr;
            for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
            {
                if ((it.GetAddress(addr) == CHIP_NO_ERROR) && addr.IsIPv4())
                {
                    char buf[Inet::IPAddress::kMaxStringLength];
                    addr.ToString(buf);
                    itemString = std::string(buf);
                    break; // Only print first IPv4 address for now
                }
            }
            break;
        }

        case kStatusItemIp6LinkLocal: {
            chip::Inet::IPAddress addr;
            for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
            {
                if ((it.GetAddress(addr) == CHIP_NO_ERROR) && addr.IsIPv6LinkLocal())
                {
                    char buf[Inet::IPAddress::kMaxStringLength];
                    addr.ToString(buf);
                    itemString = std::string(buf);
                    if (itemString.length() < MAX_LENGTH_SMALL_FONT)
                    {
                        TFT_setFont(SMALL_FONT, nullptr);
                    }
                    else
                    {
                        TFT_setFont(DEF_SMALL_FONT, nullptr);
                    }
                    break; // Only print first IPv6 LL for now
                }
            }
            break;
        }

        case kStatusItemIp6Ula: {
            chip::Inet::IPAddress addr;
            for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
            {
                if ((it.GetAddress(addr) == CHIP_NO_ERROR) && addr.IsIPv6ULA())
                {
                    char buf[Inet::IPAddress::kMaxStringLength];
                    addr.ToString(buf);
                    itemString = std::string(buf);
                    if (itemString.length() < MAX_LENGTH_SMALL_FONT)
                    {
                        TFT_setFont(SMALL_FONT, nullptr);
                    }
                    else
                    {
                        TFT_setFont(DEF_SMALL_FONT, nullptr);
                    }
                    break; // Only print first IPv6 ULA for now
                }
            }
            break;
        }
        }
        ESP_LOGI("M5 UI", "Display status %d: %s", i, itemString.c_str());

        return itemString;
    }

    virtual void ItemAction(int i) {}

private:
    std::vector<std::string> options;
};

StatusScreen::StatusScreen() : ListScreen(chip::Platform::New<StatusListModel>()) {}

#endif // CONFIG_HAVE_DISPLAY
