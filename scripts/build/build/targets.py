# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from enum import IntEnum, auto


class Platform(IntEnum):
    """Represents a supported build platform for compilation."""
    HOST = auto()
    QPG = auto()
    ESP32 = auto()
    EFR32 = auto()
    NRF = auto()
    ANDROID = auto()
    INFINEON = auto()
    TELINK = auto()

    @property
    def ArgName(self):
        return self.name.lower()

    @staticmethod
    def FromArgName(name):
        for value in Platform:
            if name == value.ArgName:
                return value
        raise KeyError()


class Board(IntEnum):
    """Represents Specific boards within a platform."""
    # Host builds
    NATIVE = auto()

    # QPG platform
    QPG6100 = auto()

    # ESP32 platform
    M5STACK = auto()
    DEVKITC = auto()
    C3DEVKIT = auto()

    # EFR32 platform
    BRD4161A = auto()

    # NRF platform
    NRF52840 = auto()
    NRF5340 = auto()

    # Telink platform
    TLSR9518ADK80D = auto()

    # Android platform
    ARM = auto()
    ARM64 = auto()
    X64 = auto()

    # Infineon board
    P6BOARD = auto()

    @property
    def ArgName(self):
        return self.name.lower()

    @staticmethod
    def FromArgName(name):
        for value in Board:
            if name == value.ArgName:
                return value
        raise KeyError()


class Application(IntEnum):
    """Example applications that can be built."""
    ALL_CLUSTERS = auto()
    LIGHT = auto()
    LOCK = auto()
    WINDOW_COVERING = auto()
    SHELL = auto()
    CHIP_TOOL = auto()
    BRIDGE = auto()
    TEMPERATURE_MEASUREMENT = auto()
    THERMOSTAT = auto()
    PUMP = auto()
    PUMP_CONTROLLER = auto()

    @property
    def ArgName(self):
        return self.name.lower().replace('_', '-')

    @staticmethod
    def FromArgName(name):
        for value in Application:
            if name == value.ArgName:
                return value
        raise KeyError()
