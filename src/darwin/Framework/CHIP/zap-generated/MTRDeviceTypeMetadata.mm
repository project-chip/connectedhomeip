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
    { 0x0000000A, DeviceTypeClass::Simple, "Matter Door Lock" },
    { 0x0000000B, DeviceTypeClass::Simple, "Matter Door Lock Controller" },
    { 0x0000000E, DeviceTypeClass::Utility, "Matter Aggregator" },
    { 0x0000000F, DeviceTypeClass::Simple, "Matter Generic Switch" },
    { 0x00000011, DeviceTypeClass::Utility, "Matter Power Source" },
    { 0x00000012, DeviceTypeClass::Utility, "Matter OTA Requestor" },
    { 0x00000013, DeviceTypeClass::Utility, "Matter Bridged Device" },
    { 0x00000014, DeviceTypeClass::Utility, "Matter OTA Provider" },
    { 0x00000015, DeviceTypeClass::Simple, "Matter Contact Sensor" },
    { 0x00000016, DeviceTypeClass::Node, "Matter Root Node" },
    { 0x00000019, DeviceTypeClass::Utility, "Matter Secondary Network Interface Device Type" },
    { 0x00000022, DeviceTypeClass::Simple, "Matter Speaker" },
    { 0x00000023, DeviceTypeClass::Simple, "Matter Casting Video Player" },
    { 0x00000024, DeviceTypeClass::Simple, "Matter Content App" },
    { 0x00000027, DeviceTypeClass::Simple, "Matter Mode Select" },
    { 0x00000028, DeviceTypeClass::Simple, "Matter Basic Video Player" },
    { 0x00000029, DeviceTypeClass::Simple, "Matter Casting Video Client" },
    { 0x0000002A, DeviceTypeClass::Simple, "Matter Video Remote Control" },
    { 0x0000002B, DeviceTypeClass::Simple, "Matter Fan" },
    { 0x0000002C, DeviceTypeClass::Simple, "Matter Air Quality Sensor" },
    { 0x0000002D, DeviceTypeClass::Simple, "Matter Air Purifier" },
    { 0x00000041, DeviceTypeClass::Simple, "Matter Water Freeze Detector" },
    { 0x00000042, DeviceTypeClass::Simple, "Matter Water Valve" },
    { 0x00000043, DeviceTypeClass::Simple, "Matter Water Leak Detector" },
    { 0x00000044, DeviceTypeClass::Simple, "Matter Rain Sensor" },
    { 0x00000070, DeviceTypeClass::Simple, "Matter Refrigerator" },
    { 0x00000071, DeviceTypeClass::Simple, "Matter Temperature Controlled Cabinet" },
    { 0x00000072, DeviceTypeClass::Simple, "Matter Room Air Conditioner" },
    { 0x00000073, DeviceTypeClass::Simple, "Matter Laundry Washer" },
    { 0x00000074, DeviceTypeClass::Simple, "Matter Robotic Vacuum Cleaner" },
    { 0x00000075, DeviceTypeClass::Simple, "Matter Dishwasher" },
    { 0x00000076, DeviceTypeClass::Simple, "Matter Smoke CO Alarm" },
    { 0x00000077, DeviceTypeClass::Simple, "Matter Cook Surface" },
    { 0x00000078, DeviceTypeClass::Simple, "Matter Cooktop" },
    { 0x00000079, DeviceTypeClass::Simple, "Matter Microwave Oven" },
    { 0x0000007A, DeviceTypeClass::Simple, "Matter Extractor Hood" },
    { 0x0000007B, DeviceTypeClass::Simple, "Matter Oven" },
    { 0x0000007C, DeviceTypeClass::Simple, "Matter Laundry Dryer" },
    { 0x00000090, DeviceTypeClass::Simple, "Matter Network Infrastructure Manager" },
    { 0x00000091, DeviceTypeClass::Simple, "Matter Thread Border Router" },
    { 0x00000100, DeviceTypeClass::Simple, "Matter On/Off Light" },
    { 0x00000101, DeviceTypeClass::Simple, "Matter Dimmable Light" },
    { 0x00000103, DeviceTypeClass::Simple, "Matter On/Off Light Switch" },
    { 0x00000104, DeviceTypeClass::Simple, "Matter Dimmer Switch" },
    { 0x00000105, DeviceTypeClass::Simple, "Matter Color Dimmer Switch" },
    { 0x00000106, DeviceTypeClass::Simple, "Matter Light Sensor" },
    { 0x00000107, DeviceTypeClass::Simple, "Matter Occupancy Sensor" },
    { 0x0000010A, DeviceTypeClass::Simple, "Matter On/Off Plug-in Unit" },
    { 0x0000010B, DeviceTypeClass::Simple, "Matter Dimmable Plug-in Unit" },
    { 0x0000010C, DeviceTypeClass::Simple, "Matter Color Temperature Light" },
    { 0x0000010D, DeviceTypeClass::Simple, "Matter Extended Color Light" },
    { 0x00000202, DeviceTypeClass::Simple, "Matter Window Covering" },
    { 0x00000203, DeviceTypeClass::Simple, "Matter Window Covering Controller" },
    { 0x00000300, DeviceTypeClass::Simple, "Matter Heating/Cooling Unit" },
    { 0x00000301, DeviceTypeClass::Simple, "Matter Thermostat" },
    { 0x00000302, DeviceTypeClass::Simple, "Matter Temperature Sensor" },
    { 0x00000303, DeviceTypeClass::Simple, "Matter Pump" },
    { 0x00000304, DeviceTypeClass::Simple, "Matter Pump Controller" },
    { 0x00000305, DeviceTypeClass::Simple, "Matter Pressure Sensor" },
    { 0x00000306, DeviceTypeClass::Simple, "Matter Flow Sensor" },
    { 0x00000307, DeviceTypeClass::Simple, "Matter Humidity Sensor" },
    { 0x0000050C, DeviceTypeClass::Simple, "Matter EVSE" },
    { 0x00000510, DeviceTypeClass::Utility, "Matter Electrical Sensor" },
    { 0x00000840, DeviceTypeClass::Simple, "Matter Control Bridge" },
    { 0x00000850, DeviceTypeClass::Simple, "Matter On/Off Sensor" },
};

static_assert(ExtractVendorFromMEI(0xFFF10001) != 0, "Must have class defined for \"Matter Orphan Clusters\" if it's a standard device type");
static_assert(ExtractVendorFromMEI(0xFFF10003) != 0, "Must have class defined for \"Matter All-clusters-app Server Example\" if it's a standard device type");

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
