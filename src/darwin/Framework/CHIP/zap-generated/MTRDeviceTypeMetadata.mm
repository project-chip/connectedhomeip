/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRDeviceTypeMetadata.h"

using namespace chip;

namespace {
enum class DeviceTypeClass {
    Utility,
    Simple,
    Node, // Might not be a real class, but we have it for Root Node for now.
    // If new classes get added, plase audit MTRIsKnownUtilityDeviceType below.
};

struct DeviceTypeData {
    DeviceTypeId id;
    DeviceTypeClass deviceClass;
    const char * name;
};

constexpr DeviceTypeData knownDeviceTypes[] = {
    { 0x0000000A, DeviceTypeClass::Simple, "Door Lock" },
    { 0x0000000B, DeviceTypeClass::Simple, "Door Lock Controller" },
    { 0x0000000E, DeviceTypeClass::Simple, "Aggregator" },
    { 0x0000000F, DeviceTypeClass::Simple, "Generic Switch" },
    { 0x00000011, DeviceTypeClass::Utility, "Power Source" },
    { 0x00000012, DeviceTypeClass::Utility, "OTA Requestor" },
    { 0x00000013, DeviceTypeClass::Utility, "Bridged Node" },
    { 0x00000014, DeviceTypeClass::Utility, "OTA Provider" },
    { 0x00000015, DeviceTypeClass::Simple, "Contact Sensor" },
    { 0x00000016, DeviceTypeClass::Node, "Root Node" },
    { 0x00000019, DeviceTypeClass::Utility, "Secondary Network Interface" },
    { 0x00000022, DeviceTypeClass::Simple, "Speaker" },
    { 0x00000023, DeviceTypeClass::Simple, "Casting Video Player" },
    { 0x00000024, DeviceTypeClass::Simple, "Content App" },
    { 0x00000027, DeviceTypeClass::Simple, "Mode Select" },
    { 0x00000028, DeviceTypeClass::Simple, "Basic Video Player" },
    { 0x00000029, DeviceTypeClass::Simple, "Casting Video Client" },
    { 0x0000002A, DeviceTypeClass::Simple, "Video Remote Control" },
    { 0x0000002B, DeviceTypeClass::Simple, "Fan" },
    { 0x0000002C, DeviceTypeClass::Simple, "Air Quality Sensor" },
    { 0x0000002D, DeviceTypeClass::Simple, "Air Purifier" },
    { 0x00000041, DeviceTypeClass::Simple, "Water Freeze Detector" },
    { 0x00000042, DeviceTypeClass::Simple, "Water Valve" },
    { 0x00000043, DeviceTypeClass::Simple, "Water Leak Detector" },
    { 0x00000044, DeviceTypeClass::Simple, "Rain Sensor" },
    { 0x00000070, DeviceTypeClass::Simple, "Refrigerator" },
    { 0x00000071, DeviceTypeClass::Simple, "Temperature Controlled Cabinet" },
    { 0x00000072, DeviceTypeClass::Simple, "Room Air Conditioner" },
    { 0x00000073, DeviceTypeClass::Simple, "Laundry Washer" },
    { 0x00000074, DeviceTypeClass::Simple, "Robotic Vacuum Cleaner" },
    { 0x00000075, DeviceTypeClass::Simple, "Dishwasher" },
    { 0x00000076, DeviceTypeClass::Simple, "Smoke CO Alarm" },
    { 0x00000077, DeviceTypeClass::Simple, "Cook Surface" },
    { 0x00000078, DeviceTypeClass::Simple, "Cooktop" },
    { 0x00000079, DeviceTypeClass::Simple, "Microwave Oven" },
    { 0x0000007A, DeviceTypeClass::Simple, "Extractor Hood" },
    { 0x0000007B, DeviceTypeClass::Simple, "Oven" },
    { 0x0000007C, DeviceTypeClass::Simple, "Laundry Dryer" },
    { 0x00000090, DeviceTypeClass::Simple, "Network Infrastructure Manager" },
    { 0x00000091, DeviceTypeClass::Simple, "Thread Border Router" },
    { 0x00000100, DeviceTypeClass::Simple, "On/Off Light" },
    { 0x00000101, DeviceTypeClass::Simple, "Dimmable Light" },
    { 0x00000103, DeviceTypeClass::Simple, "On/Off Light Switch" },
    { 0x00000104, DeviceTypeClass::Simple, "Dimmer Switch" },
    { 0x00000105, DeviceTypeClass::Simple, "Color Dimmer Switch" },
    { 0x00000106, DeviceTypeClass::Simple, "Light Sensor" },
    { 0x00000107, DeviceTypeClass::Simple, "Occupancy Sensor" },
    { 0x0000010A, DeviceTypeClass::Simple, "On/Off Plug-in Unit" },
    { 0x0000010B, DeviceTypeClass::Simple, "Dimmable Plug-in Unit" },
    { 0x0000010C, DeviceTypeClass::Simple, "Color Temperature Light" },
    { 0x0000010D, DeviceTypeClass::Simple, "Extended Color Light" },
    { 0x00000202, DeviceTypeClass::Simple, "Window Covering" },
    { 0x00000203, DeviceTypeClass::Simple, "Window Covering Controller" },
    { 0x00000300, DeviceTypeClass::Simple, "Heating/Cooling Unit" },
    { 0x00000301, DeviceTypeClass::Simple, "Thermostat" },
    { 0x00000302, DeviceTypeClass::Simple, "Temperature Sensor" },
    { 0x00000303, DeviceTypeClass::Simple, "Pump" },
    { 0x00000304, DeviceTypeClass::Simple, "Pump Controller" },
    { 0x00000305, DeviceTypeClass::Simple, "Pressure Sensor" },
    { 0x00000306, DeviceTypeClass::Simple, "Flow Sensor" },
    { 0x00000307, DeviceTypeClass::Simple, "Humidity Sensor" },
    { 0x0000050C, DeviceTypeClass::Simple, "EVSE" },
    { 0x0000050D, DeviceTypeClass::Simple, "Device Energy Management" },
    { 0x00000510, DeviceTypeClass::Utility, "Electrical Sensor" },
    { 0x00000840, DeviceTypeClass::Simple, "Control Bridge" },
    { 0x00000850, DeviceTypeClass::Simple, "On/Off Sensor" },
};

static_assert(ExtractVendorFromMEI(0xFFF10001) != 0, "Must have class defined for \"Orphan Clusters\" if it's a standard device type");
static_assert(ExtractVendorFromMEI(0xFFF10003) != 0, "Must have class defined for \"All-clusters-app Server Example\" if it's a standard device type");

} // anonymous namespace

BOOL MTRIsKnownUtilityDeviceType(DeviceTypeId aDeviceTypeId)
{
    for (auto & deviceType : knownDeviceTypes) {
        if (deviceType.id == aDeviceTypeId) {
            return deviceType.deviceClass != DeviceTypeClass::Simple;
        }
    }
    return NO;
}
