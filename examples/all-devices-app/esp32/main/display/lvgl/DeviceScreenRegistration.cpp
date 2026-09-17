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
#include "devices/BooleanStateSensorScreen.h"
#include "devices/DimmableLightScreen.h"
#include "devices/OccupancySensorScreen.h"
#include "devices/OnOffLightScreen.h"
#include "devices/TemperatureSensorScreen.h"

#include <devices/Ids.h>

namespace chip::app {

namespace {

constexpr DeviceTypeId kInvalidDeviceTypeId = 0xFFFF'FFFF;

struct BooleanSensorDescriptor
{
    DeviceTypeId id;
    const char * title;
    const char * deviceTypeKey;
    const char * trueLabel;
    const char * falseLabel;
};

constexpr BooleanSensorDescriptor kBooleanSensorDescriptors[] = {
    { kInvalidDeviceTypeId, "Boolean Sensor", "boolean-state-sensor", "Active (True)", "Inactive (False)" },
    { Device::kContactSensorDeviceTypeId, "Contact Sensor", "contact-sensor", "Closed (True)", "Open (False)" },
    { Device::kWaterLeakDetectorDeviceTypeId, "Water Leak Detector", "water-leak-detector", "Leak Detected", "Dry (Normal)" },
    { Device::kWaterFreezeDetectorDeviceTypeId, "Freeze Detector", "water-freeze-detector", "Freeze Detected", "Normal" },
    { Device::kRainSensorDeviceTypeId, "Rain Sensor", "rain-sensor", "Raining", "Dry" },
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
        .deviceType = "on-off-light",
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLoadScreen(parent, "On/Off Light", device); },
    });
}

void RegisterDeviceScreen(DimmableLight & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Dimmable Light",
        .endpointId = device.GetEndpointId(),
        .deviceType = "dimmable-light",
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLoadScreen(parent, "Dimmable Light", device); },
    });
}

void RegisterDeviceScreen(OnOffPlugInUnit & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "On/Off Plug-In Unit",
        .endpointId = device.GetEndpointId(),
        .deviceType = "on-off-plug-in-unit",
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLoadScreen(parent, "On/Off Plug-In Unit", device); },
    });
}

void RegisterDeviceScreen(DimmablePlugInUnit & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Dimmable Plug-In Unit",
        .endpointId = device.GetEndpointId(),
        .deviceType = "dimmable-plug-in-unit",
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLoadScreen(parent, "Dimmable Plug-In Unit", device); },
    });
}

void RegisterDeviceScreen(MountedOnOffControl & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Mounted On/Off",
        .endpointId = device.GetEndpointId(),
        .deviceType = "mounted-on-off-control",
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLoadScreen(parent, "Mounted On/Off", device); },
    });
}

void RegisterDeviceScreen(MountedDimmableLoadControl & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Mounted Dimmable",
        .endpointId = device.GetEndpointId(),
        .deviceType = "mounted-dimmable-load-control",
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLoadScreen(parent, "Mounted Dimmable", device); },
    });
}

void RegisterDeviceScreen(BooleanStateSensor & device, DeviceScreenRegistry & registry)
{
    const auto & desc = GetBooleanSensorDescriptor(device);
    registry.Register({
        .title      = desc.title,
        .endpointId = device.GetEndpointId(),
        .deviceType = desc.deviceTypeKey,
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
        .deviceType = "occupancy-sensor",
        .renderFn   = [&device](lv_obj_t * parent) { ShowOccupancySensorScreen(parent, device); },
    });
}

void RegisterDeviceScreen(TemperatureSensor & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Temperature Sensor",
        .endpointId = device.GetEndpointId(),
        .deviceType = "temperature-sensor",
        .renderFn   = [&device](lv_obj_t * parent) { ShowTemperatureSensorScreen(parent, device); },
    });
}

} // namespace chip::app
