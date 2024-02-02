#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

MATTER_APPLICATION_DEVICE_TYPES = {
    # lighting
    "0x100": "On/Off Light",
    "0x101": "Dimmable Light",
    "0x10C": "Color Temperature Light",
    "0x10D": "Extended Color Light",
    # smart plugs/outlets and other actuators
    "0x10A": "On/Off Plug-in Unit",
    "0x10B": "Dimmable Plug-In Unit",
    "0x303": "Pump",
    # switches and controls
    "0x103": "On/Off Light Switch",
    "0x104": "Dimmer Switch",
    "0x105": "Color Dimmer Switch",
    "0x840": "Control Bridge",
    "0x304": "Pump Controller",
    "0xF": "Generic Switch",
    # sensors
    "0x15": "Contact Sensor",
    "0x106": "Light Sensor",
    "0x107": "Occupancy Sensor",
    "0x302": "Temperature Sensor",
    "0x305": "Pressure Sensor",
    "0x306": "Flow Sensor",
    "0x307": "Humidity Sensor",
    "0x850": "On/Off Sensor",
    # closures
    "0xA": "Door Lock",
    "0xB": "Door Lock Controller",
    "0x202": "Window Covering",
    "0x203": "Window Covering Controller",
    # HVAC
    "0x300": "Heating/Cooling Unit",
    "0x301": "Thermostat",
    "0x2B": "Fan",
    # media
    "0x28": "Basic Video Player",
    "0x23": "Casting Video Player",
    "0x22": "Speaker",
    "0x24": "Content App",
    "0x29": "Casting Video Client",
    "0x2A": "Video Remote Control",
    # generic
    "0x27": "Mode Select",
}

MATTER_COMMISSIONING_MODE_DESCRIPTIONS = [
    "Not in commissioning mode",
    "In passcode commissioning mode (standard mode)",
    "In dynamic passcode commissioning mode",
]

MATTER_PAIRING_HINTS = [
    "Power Cycle",
    "Custom commissioning flow",
    "Use existing administrator (already commissioned)",
    "Use settings menu on device",
    "Use the PI TXT record hint",
    "Read the manual",
    "Press the reset button",
    "Press Reset Button with application of power",
    "Press Reset Button for N seconds",
    "Press Reset Button until light blinks",
    "Press Reset Button for N seconds with application of power",
    "Press Reset Button until light blinks with application of power",
    "Press Reset Button N times",
    "Press Setup Button",
    "Press Setup Button with application of power",
    "Press Setup Button for N seconds",
    "Press Setup Button until light blinks",
    "Press Setup Button for N seconds with application of power",
    "Press Setup Button until light blinks with application of power",
    "Press Setup Button N times",
]
