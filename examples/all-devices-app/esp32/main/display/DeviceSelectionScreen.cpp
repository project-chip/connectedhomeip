/*
 *
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

#include "DeviceSelectionScreen.h"

#if CONFIG_HAVE_DISPLAY

#include "ScreenManager.h"
#include <device-factory/DeviceFactory.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

CHIP_ERROR SetDeviceTypeAndRestart(const std::string & deviceType);
const std::string & GetActiveDeviceType();

DeviceSelectionListModel::DeviceSelectionListModel()
{
    mItems.push_back({ "< Back", []() { ScreenManager::PopScreen(); } });

    const std::string & activeDev = GetActiveDeviceType();
    bool isAllBridged             = (activeDev == "*" || activeDev == "aggregator");

    // Put current active device first so user sees it immediately
    if (isAllBridged)
    {
        mItems.push_back({ "[*] All Bridged (*)", []() { ScreenManager::PopScreen(); } });
    }
    else
    {
        std::string activeLabel = "[*] " + activeDev;
        mItems.push_back({ activeLabel, []() { ScreenManager::PopScreen(); } });

        // Include All Bridged option
        mItems.push_back({ "    All Bridged (*)", []() { LogErrorOnFailure(SetDeviceTypeAndRestart("*")); } });
    }

    auto & deviceFactory = chip::app::NoHooksDeviceFactory::GetInstance();
    for (const auto & deviceType : deviceFactory.SupportedDeviceTypes())
    {
        if (deviceType == "aggregator" || deviceType == "bridged-node" || deviceType == activeDev)
        {
            continue;
        }

        std::string label = "    " + deviceType;
        std::string dev   = deviceType;

        mItems.push_back({ label, [dev]() { LogErrorOnFailure(SetDeviceTypeAndRestart(dev)); } });
    }
}

std::string DeviceSelectionListModel::GetItemText(int i)
{
    if (i >= 0 && i < static_cast<int>(mItems.size()))
    {
        return mItems[i].text;
    }
    return "";
}

void DeviceSelectionListModel::ItemAction(int i)
{
    if (i >= 0 && i < static_cast<int>(mItems.size()) && mItems[i].action)
    {
        mItems[i].action();
    }
}

void PushDeviceSelectionScreen()
{
    auto * model = chip::Platform::New<DeviceSelectionListModel>();
    if (model == nullptr)
    {
        return;
    }
    auto * screen = chip::Platform::New<ListScreen>(model);
    if (screen == nullptr)
    {
        chip::Platform::Delete(model);
        return;
    }
    ScreenManager::PushScreen(screen);
}

#endif // CONFIG_HAVE_DISPLAY
