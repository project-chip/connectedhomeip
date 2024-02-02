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

# MT:Y.K90EDH025-LI0IQ00
# Version:             0
# VendorID:            65521
# ProductID:           32768
# Custom flow:         0    (STANDARD)
# Discovery Bitmask:   0x00 (NONE)
# Long discriminator:  10   (0xa)
# Passcode:            32966599
# https://project-chip.github.io/connectedhomeip/qrcode.html?data=MT:Y.K90EDH025-LI0IQ00

from utils.host import current_platform

DEFAULT_VID = 0xFFF1
DEFAULT_PID = 0x8000
DEFAULT_DISCRIMINATOR = 0xA
DEFAULT_DEVICE_NAME = "IDT fake device"
DEFAULT_DEVICE_TYPE = 0x302  # Temperature Sensor
DEFAULT_PORT = 5540
DEFAULT_COMMISSIONING_OPEN = "t"
DEFAULT_MAC_ADDR = current_platform.get_mac_addr()
DEFAULT_ALLOW_V6_GUA = "f"
