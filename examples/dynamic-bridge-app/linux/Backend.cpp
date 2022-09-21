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

#include "Backend.h"

#include "main.h"

std::vector<std::unique_ptr<Device>> g_devices;
std::vector<std::unique_ptr<DynamicDevice>> g_device_impls;

bool RemoveDeviceAt(uint32_t index)
{
    if (index >= g_devices.size() || !g_devices[index])
    {
        return false;
    }

    RemoveDeviceEndpoint(g_devices[index].get());

    for (auto & room : gRooms)
        room.RemoveEndpoint(g_devices[index]->GetEndpointId());

    g_devices[index]      = nullptr;
    g_device_impls[index] = nullptr;

    return true;
}

int AddDevice(std::unique_ptr<DynamicDevice> device)
{
    auto dev = std::make_unique<Device>(device->CreateDevice());
    int ep   = AddDeviceEndpoint(dev.get());
    if (ep < 0)
    {
        return -1;
    }

    size_t index = (size_t) ep;
    if (g_devices.size() <= index)
    {
        g_devices.resize(index + 1);
        g_device_impls.resize(index + 1);
    }
    g_devices[index]      = std::move(dev);
    g_device_impls[index] = std::move(device);
    return ep;
}
