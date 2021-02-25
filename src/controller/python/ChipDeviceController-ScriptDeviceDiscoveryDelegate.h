/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <controller/CHIPDeviceController.h>

namespace chip {
namespace Controller {

extern "C" using DeviceDiscoveryDelegate_OnDiscoveryComplete = void(*)(NodeId nodeId, CHIP_ERROR err);

class ScriptDeviceDiscoveryDelegate final : public Controller::DeviceDiscoveryDelegate
{
public:
    void OnDiscoveryComplete(NodeId nodeId, CHIP_ERROR error);
    void SetDiscoveryCompleteCallback(DeviceDiscoveryDelegate_OnDiscoveryComplete cb) { mDiscoveryCompleteCallback = cb; }

private:
    DeviceDiscoveryDelegate_OnDiscoveryComplete mDiscoveryCompleteCallback = nullptr;
};

} // namespace Controller
} // namespace chip
