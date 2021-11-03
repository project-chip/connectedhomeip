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

import os

from builders.android import AndroidBoard, AndroidApp, AndroidBuilder
from builders.efr32 import Efr32Builder, Efr32App, Efr32Board
from builders.esp32 import Esp32Builder, Esp32Board, Esp32App
from builders.host import HostBuilder, HostApp, HostBoard
from builders.nrf import NrfApp, NrfBoard, NrfConnectBuilder
from builders.qpg import QpgBuilder
from builders.infineon import InfineonBuilder, InfineonApp, InfineonBoard
from builders.telink import TelinkApp, TelinkBoard, TelinkBuilder
from builders.tizen import TizenApp, TizenBoard, TizenBuilder
from builders.ameba import AmebaApp, AmebaBoard, AmebaBuilder
from builders.mbed import MbedApp, MbedBoard, MbedProfile, MbedBuilder


class Target:
    """Represents a build target:

        Has a name identifier plus parameters on how to build it (what
        builder class to use and what arguments are required to produce
        the specified build)
    """

    def __init__(self, name, builder_class, **kwargs):
        self.name = name
        self.builder_class = builder_class
        self.glob_blacklist_reason = None

        self.create_kw_args = kwargs

    def Clone(self):
        """Creates a clone of self."""

        clone = Target(self.name, self.builder_class,
                       **self.create_kw_args.copy())
        clone.glob_blacklist_reason = self.glob_blacklist_reason

        return clone

    def Extend(self, suffix, **kargs):
        """Creates a clone of the current object extending its build parameters.

        Arguments:
           suffix: appended with a "-" as separator to the clone name
           **kargs: arguments needed to produce the new build variant
        """
        clone = self.Clone()
        clone.name += "-" + suffix
        clone.create_kw_args.update(kargs)
        return clone

    def Create(self, runner, repository_path: str, output_prefix: str, enable_flashbundle: bool):
        builder = self.builder_class(
            repository_path, runner=runner, **self.create_kw_args)

        builder.target = self
        builder.identifier = self.name
        builder.output_dir = os.path.join(output_prefix, self.name)
        builder.enable_flashbundle(enable_flashbundle)

        return builder

    def GlobBlacklist(self, reason):
        clone = self.Clone()
        if clone.glob_blacklist_reason:
            clone.glob_blacklist_reason += ", "
            clone.glob_blacklist_reason += reason
        else:
            clone.glob_blacklist_reason = reason

        return clone

    @property
    def IsGlobBlacklisted(self):
        return self.glob_blacklist_reason is not None

    @property
    def GlobBlacklistReason(self):
        return self.glob_blacklist_reason


def HostTargets():
    target = Target(HostBoard.NATIVE.PlatformName(), HostBuilder)
    targets = [
        target.Extend(HostBoard.NATIVE.BoardName(), board=HostBoard.NATIVE)
    ]

    # x64 linux  supports cross compile
    if (HostBoard.NATIVE.PlatformName() == 'linux') and (
            HostBoard.NATIVE.BoardName() != HostBoard.ARM64.BoardName()):
        targets.append(target.Extend('arm64', board=HostBoard.ARM64))

    app_targets = []
    for target in targets:
        app_targets.append(target.Extend(
            'all-clusters', app=HostApp.ALL_CLUSTERS))
        app_targets.append(target.Extend('chip-tool', app=HostApp.CHIP_TOOL))
        app_targets.append(target.Extend('thermostat', app=HostApp.THERMOSTAT))
        app_targets.append(target.Extend(
            'rpc-console', app=HostApp.RPC_CONSOLE))

    for target in app_targets:
        yield target
        if ('rpc-console' not in target.name):
            yield target.Extend('ipv6only', enable_ipv4=False)


def Esp32Targets():
    esp32_target = Target('esp32', Esp32Builder)

    yield esp32_target.Extend('m5stack-all-clusters', board=Esp32Board.M5Stack, app=Esp32App.ALL_CLUSTERS)
    yield esp32_target.Extend('m5stack-all-clusters-ipv6only', board=Esp32Board.M5Stack, app=Esp32App.ALL_CLUSTERS, enable_ipv4=False)
    yield esp32_target.Extend('m5stack-all-clusters-rpc', board=Esp32Board.M5Stack, app=Esp32App.ALL_CLUSTERS, enable_rpcs=True)
    yield esp32_target.Extend('m5stack-all-clusters-rpc-ipv6only', board=Esp32Board.M5Stack, app=Esp32App.ALL_CLUSTERS, enable_rpcs=True, enable_ipv4=False)
    yield esp32_target.Extend('c3devkit-all-clusters', board=Esp32Board.C3DevKit, app=Esp32App.ALL_CLUSTERS)

    devkitc = esp32_target.Extend('devkitc', board=Esp32Board.DevKitC)

    yield devkitc.Extend('all-clusters', app=Esp32App.ALL_CLUSTERS)
    yield devkitc.Extend('all-clusters-ipv6only', app=Esp32App.ALL_CLUSTERS, enable_ipv4=False)
    yield devkitc.Extend('shell', app=Esp32App.SHELL)
    yield devkitc.Extend('lock', app=Esp32App.LOCK)
    yield devkitc.Extend('bridge', app=Esp32App.BRIDGE)
    yield devkitc.Extend('temperature-measurement', app=Esp32App.TEMPERATURE_MEASUREMENT)


def Efr32Targets():
    efr_target = Target('efr32-brd4161a', Efr32Builder,
                        board=Efr32Board.BRD4161A)

    yield efr_target.Extend('window-covering', app=Efr32App.WINDOW_COVERING)
    yield efr_target.Extend('lock', app=Efr32App.LOCK)

    rpc_aware_targets = [
        efr_target.Extend('light', app=Efr32App.LIGHT),
    ]

    for target in rpc_aware_targets:
        yield target
        yield target.Extend('rpc', enable_rpcs=True)


def NrfTargets():
    target = Target('nrf', NrfConnectBuilder)

    targets = [
        target.Extend('nrf5340', board=NrfBoard.NRF5340),
        target.Extend('nrf52840', board=NrfBoard.NRF52840),
    ]

    for target in targets:
        yield target.Extend('lock', app=NrfApp.LOCK)
        yield target.Extend('light', app=NrfApp.LIGHT)
        yield target.Extend('shell', app=NrfApp.SHELL)
        yield target.Extend('pump', app=NrfApp.PUMP)
        yield target.Extend('pump-controller', app=NrfApp.PUMP_CONTROLLER)

        rpc = target.Extend('light-rpc', app=NrfApp.LIGHT, enable_rpcs=True)

        if '-nrf5340-' in rpc.name:
            rpc = rpc.GlobBlacklist(
                'Compile failure due to pw_build args not forwarded to proto compiler. https://pigweed-review.googlesource.com/c/pigweed/pigweed/+/66760')

        yield rpc


def AndroidTargets():
    target = Target('android', AndroidBuilder)

    yield target.Extend('arm-chip-tool', board=AndroidBoard.ARM, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('arm64-chip-tool', board=AndroidBoard.ARM64, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('x64-chip-tool', board=AndroidBoard.X64, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('x86-chip-tool', board=AndroidBoard.X86, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('arm64-chip-test', board=AndroidBoard.ARM64, app=AndroidApp.CHIP_TEST)
    yield target.Extend('androidstudio-arm-chip-tool', board=AndroidBoard.AndroidStudio_ARM, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('androidstudio-arm64-chip-tool', board=AndroidBoard.AndroidStudio_ARM64, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('androidstudio-x86-chip-tool', board=AndroidBoard.AndroidStudio_X86, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('androidstudio-x64-chip-tool', board=AndroidBoard.AndroidStudio_X64, app=AndroidApp.CHIP_TOOL)
    yield target.Extend('arm64-chip-tvserver', board=AndroidBoard.ARM64, app=AndroidApp.CHIP_TVServer)


def MbedTargets():
    target = Target('mbed', MbedBuilder)

    targets = [
        target.Extend('CY8CPROTO_062_4343W',
                      board=MbedBoard.CY8CPROTO_062_4343W),
    ]

    app_targets = []
    for target in targets:
        app_targets.append(target.Extend('lock', app=MbedApp.LOCK))
        app_targets.append(target.Extend('light', app=MbedApp.LIGHT))
        app_targets.append(target.Extend(
            'all-clusters', app=MbedApp.ALL_CLUSTERS))
        app_targets.append(target.Extend('pigweed', app=MbedApp.PIGWEED))
        app_targets.append(target.Extend('shell', app=MbedApp.SHELL))

    for target in app_targets:
        yield target.Extend('release', profile=MbedProfile.RELEASE)
        yield target.Extend('develop', profile=MbedProfile.DEVELOP).GlobBlacklist('Compile only for debugging purpose - https://os.mbed.com/docs/mbed-os/latest/program-setup/build-profiles-and-rules.html')
        yield target.Extend('debug', profile=MbedProfile.DEBUG).GlobBlacklist('Compile only for debugging purpose - https://os.mbed.com/docs/mbed-os/latest/program-setup/build-profiles-and-rules.html')


def InfineonTargets():
    target = Target('infineon', InfineonBuilder)

    yield target.Extend('p6-lock', board=InfineonBoard.P6BOARD, app=InfineonApp.LOCK)
    yield target.Extend('p6-all-clusters', board=InfineonBoard.P6BOARD, app=InfineonApp.ALL_CLUSTERS)


ALL = []

target_generators = [
    HostTargets(),
    Esp32Targets(),
    Efr32Targets(),
    NrfTargets(),
    AndroidTargets(),
    MbedTargets(),
    InfineonTargets()

]

for generator in target_generators:
    for target in generator:
        ALL.append(target)

# Simple targets added one by one
ALL.append(Target('qpg-qpg6100-lock', QpgBuilder))
ALL.append(Target('telink-tlsr9518adk80d-light', TelinkBuilder,
                  board=TelinkBoard.TLSR9518ADK80D, app=TelinkApp.LIGHT))
ALL.append(Target('tizen-arm-light', TizenBuilder,
                  board=TizenBoard.ARM, app=TizenApp.LIGHT))
ALL.append(Target('ameba-amebad-all-clusters', AmebaBuilder,
                  board=AmebaBoard.AMEBAD, app=AmebaApp.ALL_CLUSTERS))

# have a consistent order overall
ALL.sort(key=lambda t: t.name)
