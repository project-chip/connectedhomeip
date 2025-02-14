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

// Not constexpr in the strict sense because NSString * is not a literal
// type, but the array is in fact constant initialized by the compiler.
static /* constexpr */ const MTRDeviceTypeData knownDeviceTypes[] = {
    { 0x0000000A, MTRDeviceTypeClass::Simple, @"Door Lock" },
    { 0x0000000B, MTRDeviceTypeClass::Simple, @"Door Lock Controller" },
    { 0x0000000E, MTRDeviceTypeClass::Simple, @"Aggregator" },
    { 0x0000000F, MTRDeviceTypeClass::Simple, @"Generic Switch" },
    { 0x00000011, MTRDeviceTypeClass::Utility, @"Power Source" },
    { 0x00000012, MTRDeviceTypeClass::Utility, @"OTA Requestor" },
    { 0x00000013, MTRDeviceTypeClass::Utility, @"Bridged Node" },
    { 0x00000014, MTRDeviceTypeClass::Utility, @"OTA Provider" },
    { 0x00000015, MTRDeviceTypeClass::Simple, @"Contact Sensor" },
    { 0x00000016, MTRDeviceTypeClass::Node, @"Root Node" },
    { 0x00000017, MTRDeviceTypeClass::Simple, @"Solar Power" },
    { 0x00000018, MTRDeviceTypeClass::Simple, @"Battery Storage" },
    { 0x00000019, MTRDeviceTypeClass::Utility, @"Secondary Network Interface" },
    { 0x00000022, MTRDeviceTypeClass::Simple, @"Speaker" },
    { 0x00000023, MTRDeviceTypeClass::Simple, @"Casting Video Player" },
    { 0x00000024, MTRDeviceTypeClass::Simple, @"Content App" },
    { 0x00000027, MTRDeviceTypeClass::Simple, @"Mode Select" },
    { 0x00000028, MTRDeviceTypeClass::Simple, @"Basic Video Player" },
    { 0x00000029, MTRDeviceTypeClass::Simple, @"Casting Video Client" },
    { 0x0000002A, MTRDeviceTypeClass::Simple, @"Video Remote Control" },
    { 0x0000002B, MTRDeviceTypeClass::Simple, @"Fan" },
    { 0x0000002C, MTRDeviceTypeClass::Simple, @"Air Quality Sensor" },
    { 0x0000002D, MTRDeviceTypeClass::Simple, @"Air Purifier" },
    { 0x00000041, MTRDeviceTypeClass::Simple, @"Water Freeze Detector" },
    { 0x00000042, MTRDeviceTypeClass::Simple, @"Water Valve" },
    { 0x00000043, MTRDeviceTypeClass::Simple, @"Water Leak Detector" },
    { 0x00000044, MTRDeviceTypeClass::Simple, @"Rain Sensor" },
    { 0x00000070, MTRDeviceTypeClass::Simple, @"Refrigerator" },
    { 0x00000071, MTRDeviceTypeClass::Simple, @"Temperature Controlled Cabinet" },
    { 0x00000072, MTRDeviceTypeClass::Simple, @"Room Air Conditioner" },
    { 0x00000073, MTRDeviceTypeClass::Simple, @"Laundry Washer" },
    { 0x00000074, MTRDeviceTypeClass::Simple, @"Robotic Vacuum Cleaner" },
    { 0x00000075, MTRDeviceTypeClass::Simple, @"Dishwasher" },
    { 0x00000076, MTRDeviceTypeClass::Simple, @"Smoke CO Alarm" },
    { 0x00000077, MTRDeviceTypeClass::Simple, @"Cook Surface" },
    { 0x00000078, MTRDeviceTypeClass::Simple, @"Cooktop" },
    { 0x00000079, MTRDeviceTypeClass::Simple, @"Microwave Oven" },
    { 0x0000007A, MTRDeviceTypeClass::Simple, @"Extractor Hood" },
    { 0x0000007B, MTRDeviceTypeClass::Simple, @"Oven" },
    { 0x0000007C, MTRDeviceTypeClass::Simple, @"Laundry Dryer" },
    { 0x00000090, MTRDeviceTypeClass::Simple, @"Network Infrastructure Manager" },
    { 0x00000091, MTRDeviceTypeClass::Simple, @"Thread Border Router" },
    { 0x00000100, MTRDeviceTypeClass::Simple, @"On/Off Light" },
    { 0x00000101, MTRDeviceTypeClass::Simple, @"Dimmable Light" },
    { 0x00000103, MTRDeviceTypeClass::Simple, @"On/Off Light Switch" },
    { 0x00000104, MTRDeviceTypeClass::Simple, @"Dimmer Switch" },
    { 0x00000105, MTRDeviceTypeClass::Simple, @"Color Dimmer Switch" },
    { 0x00000106, MTRDeviceTypeClass::Simple, @"Light Sensor" },
    { 0x00000107, MTRDeviceTypeClass::Simple, @"Occupancy Sensor" },
    { 0x0000010A, MTRDeviceTypeClass::Simple, @"On/Off Plug-in Unit" },
    { 0x0000010B, MTRDeviceTypeClass::Simple, @"Dimmable Plug-in Unit" },
    { 0x0000010C, MTRDeviceTypeClass::Simple, @"Color Temperature Light" },
    { 0x0000010D, MTRDeviceTypeClass::Simple, @"Extended Color Light" },
    { 0x00000142, MTRDeviceTypeClass::Simple, @"Camera" },
    { 0x00000202, MTRDeviceTypeClass::Simple, @"Window Covering" },
    { 0x00000203, MTRDeviceTypeClass::Simple, @"Window Covering Controller" },
    { 0x00000300, MTRDeviceTypeClass::Simple, @"Heating/Cooling Unit" },
    { 0x00000301, MTRDeviceTypeClass::Simple, @"Thermostat" },
    { 0x00000302, MTRDeviceTypeClass::Simple, @"Temperature Sensor" },
    { 0x00000303, MTRDeviceTypeClass::Simple, @"Pump" },
    { 0x00000304, MTRDeviceTypeClass::Simple, @"Pump Controller" },
    { 0x00000305, MTRDeviceTypeClass::Simple, @"Pressure Sensor" },
    { 0x00000306, MTRDeviceTypeClass::Simple, @"Flow Sensor" },
    { 0x00000307, MTRDeviceTypeClass::Simple, @"Humidity Sensor" },
    { 0x00000309, MTRDeviceTypeClass::Simple, @"Heat Pump" },
    { 0x0000050C, MTRDeviceTypeClass::Simple, @"EVSE" },
    { 0x0000050D, MTRDeviceTypeClass::Utility, @"Device Energy Management" },
    { 0x0000050F, MTRDeviceTypeClass::Simple, @"Water Heater" },
    { 0x00000510, MTRDeviceTypeClass::Utility, @"Electrical Sensor" },
    { 0x00000840, MTRDeviceTypeClass::Simple, @"Control Bridge" },
    { 0x00000850, MTRDeviceTypeClass::Simple, @"On/Off Sensor" },
};

static_assert(ExtractVendorFromMEI(0xFFF10001) != 0, "Must have class defined for \"Orphan Clusters\" if it's a standard device type");
static_assert(ExtractVendorFromMEI(0xFFF10003) != 0, "Must have class defined for \"All-clusters-app Server Example\" if it's a standard device type");

} // anonymous namespace

const MTRDeviceTypeData * _Nullable MTRDeviceTypeDataForID(chip::DeviceTypeId aDeviceTypeId)
{
    for (auto & deviceType : knownDeviceTypes) {
        if (deviceType.id == aDeviceTypeId) {
            return &deviceType;
        }
    }
    return nullptr;
}
