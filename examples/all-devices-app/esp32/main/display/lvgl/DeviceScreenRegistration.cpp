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

#include "DeviceScreenRegistration.h"
#include "devices/AggregatorScreen.h"
#include "devices/BooleanStateSensorScreen.h"
#include "devices/BridgedNodeScreen.h"
#include "devices/ChimeScreen.h"
#include "devices/ColorLightScreen.h"
#include "devices/DimmableLoadScreen.h"
#include "devices/FanLoadScreen.h"
#include "devices/OccupancySensorScreen.h"
#include "devices/OnOffLoadScreen.h"
#include "devices/TemperatureSensorScreen.h"

#include <cstdio>
#include <devices/Ids.h>

namespace chip::app {

namespace {

constexpr DeviceTypeId kInvalidDeviceTypeId = 0xFFFF'FFFF;

struct BooleanSensorDescriptor
{
    DeviceTypeId id;
    const char * title;
    const char * trueLabel;
    const char * falseLabel;
};

constexpr BooleanSensorDescriptor kBooleanSensorDescriptors[] = {
    { kInvalidDeviceTypeId, "Boolean Sensor", "Active (True)", "Inactive (False)" },
    { Device::kContactSensorDeviceTypeId, "Contact Sensor", "Closed (True)", "Open (False)" },
    { Device::kWaterLeakDetectorDeviceTypeId, "Water Leak Detector", "Leak Detected", "Dry (Normal)" },
    { Device::kWaterFreezeDetectorDeviceTypeId, "Freeze Detector", "Freeze Detected", "Normal" },
    { Device::kRainSensorDeviceTypeId, "Rain Sensor", "Raining", "Dry" },
};

DeviceTypeId GetDeviceType(const DeviceInterface & device)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> builder;
    VerifyOrReturnValue(device.DeviceTypes(builder) == CHIP_NO_ERROR, kInvalidDeviceTypeId);

    auto types = builder.TakeBuffer();
    VerifyOrReturnValue(!types.empty(), kInvalidDeviceTypeId);

    return types[0].deviceTypeId;
}

const BooleanSensorDescriptor & GetBooleanSensorDescriptor(const DeviceInterface & device)
{
    DeviceTypeId id = GetDeviceType(device);
    for (const auto & desc : kBooleanSensorDescriptors)
    {
        if (desc.id == id)
        {
            return desc;
        }
    }
    return kBooleanSensorDescriptors[0];
}

} // namespace

void RegisterDeviceScreen(OnOffLight & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "On/Off Light",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLoadScreen(parent, "On/Off Light", device); },
    });
}

void RegisterDeviceScreen(DimmableLight & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Dimmable Light",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLoadScreen(parent, "Dimmable Light", device); },
    });
}

void RegisterDeviceScreen(ColorTemperatureLight & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Color Temperature Light",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowColorTemperatureLightScreen(parent, device); },
    });
}

void RegisterDeviceScreen(ExtendedColorLight & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Extended Color Light",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowExtendedColorLightScreen(parent, device); },
    });
}

void RegisterDeviceScreen(OnOffPlugInUnit & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "On/Off Plug-In Unit",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLoadScreen(parent, "On/Off Plug-In Unit", device); },
    });
}

void RegisterDeviceScreen(DimmablePlugInUnit & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Dimmable Plug-In Unit",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLoadScreen(parent, "Dimmable Plug-In Unit", device); },
    });
}

void RegisterDeviceScreen(MountedOnOffControl & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Mounted On/Off",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLoadScreen(parent, "Mounted On/Off", device); },
    });
}

void RegisterDeviceScreen(MountedDimmableLoadControl & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Mounted Dimmable",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLoadScreen(parent, "Mounted Dimmable", device); },
    });
}

void RegisterDeviceScreen(BooleanStateSensor & device, DeviceScreenRegistry & registry)
{
    const auto & desc = GetBooleanSensorDescriptor(device);
    registry.Register({
        .title      = desc.title,
        .endpointId = device.GetEndpointId(),
        .renderFn =
            [&device, &desc](lv_obj_t * parent) {
                ShowBooleanStateSensorScreen(parent, desc.title, device, desc.trueLabel, desc.falseLabel);
            },
    });
}

void RegisterDeviceScreen(OccupancySensor & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Occupancy Sensor",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowOccupancySensorScreen(parent, device); },
    });
}

void RegisterDeviceScreen(TemperatureSensor & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Temperature Sensor",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowTemperatureSensorScreen(parent, device); },
    });
}

void RegisterDeviceScreen(Fan & device, DeviceScreenRegistry & registry)
{
    const bool hasOnOff = (device.OnOffCluster() != nullptr);
    const char * title  = hasOnOff ? "Fan" : "Fan (No On/Off)";

    registry.Register({
        .title      = title,
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device, title](lv_obj_t * parent) { ShowFanLoadScreen(parent, title, device); },
    });
}

void RegisterDeviceScreen(AirPurifier & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Air Purifier",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowFanLoadScreen(parent, "Air Purifier", device); },
    });
}

void RegisterDeviceScreen(ExtractorHood & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Extractor Hood",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowFanLoadScreen(parent, "Extractor Hood", device); },
    });
}

void RegisterDeviceScreen(Chime & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Chime",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowChimeScreen(parent, device); },
    });
}

void RegisterDeviceScreen(BridgedNode & device, DeviceScreenRegistry & registry)
{
    // The node label identifies which bridged device this node carries; all bridged nodes
    // would otherwise show the same title.
    std::string title = "Bridged: " + device.BridgedDeviceBasicInformationCluster().GetNodeLabel();

    registry.Register({
        .title      = std::move(title),
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowBridgedNodeScreen(parent, device); },
    });
}

void RegisterDeviceScreen(Aggregator & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Aggregator",
        .endpointId = device.GetEndpointId(),
        .renderFn   = [&device](lv_obj_t * parent) { ShowAggregatorScreen(parent, device); },
    });
}

void RegisterMissingDeviceScreen(DeviceInterface & device, EndpointId endpointId, DeviceScreenRegistry & registry)
{
    DeviceTypeId id = GetDeviceType(device);

    char title[40];
    if (id <= 0xFFFF)
    {
        snprintf(title, sizeof(title), "No UI - device type 0x%04X", static_cast<unsigned int>(id));
    }
    else
    {
        snprintf(title, sizeof(title), "No UI - device type 0x%08X", static_cast<unsigned int>(id));
    }

    registry.Register({
        .title      = title,
        .endpointId = endpointId,
        .renderFn   = nullptr,
    });
}

} // namespace chip::app
