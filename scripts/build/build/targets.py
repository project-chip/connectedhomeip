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
import re
from itertools import combinations
from typing import List, Any, Optional

from .target import BuildTarget, TargetPart

from builders.ameba import AmebaApp, AmebaBoard, AmebaBuilder
from builders.android import AndroidApp, AndroidBoard, AndroidBuilder
from builders.cc13x2x7_26x2x7 import cc13x2x7_26x2x7App, cc13x2x7_26x2x7Builder
from builders.cyw30739 import Cyw30739App, Cyw30739Board, Cyw30739Builder
from builders.efr32 import Efr32App, Efr32Board, Efr32Builder
from builders.esp32 import Esp32App, Esp32Board, Esp32Builder
from builders.host import HostApp, HostBoard, HostBuilder, HostCryptoLibrary
from builders.infineon import InfineonApp, InfineonBoard, InfineonBuilder
from builders.k32w import K32WApp, K32WBuilder
from builders.mbed import MbedApp, MbedBoard, MbedBuilder, MbedProfile
from builders.mw320 import MW320App, MW320Builder
from builders.nrf import NrfApp, NrfBoard, NrfConnectBuilder
from builders.qpg import QpgApp, QpgBoard, QpgBuilder
from builders.telink import TelinkApp, TelinkBoard, TelinkBuilder
from builders.tizen import TizenApp, TizenBoard, TizenBuilder
from builders.bouffalolab import BouffalolabApp, BouffalolabBoard, BouffalolabBuilder
from builders.imx import IMXApp, IMXBuilder
from builders.genio import GenioApp, GenioBuilder


def BuildHostTestRunnerTarget():
    target = BuildTarget(HostBoard.NATIVE.PlatformName(), HostBuilder)

    target.AppendFixedTargets([
        TargetPart(HostBoard.NATIVE.BoardName(), board=HostBoard.NATIVE),
    ])

    target.AppendFixedTargets([
        TargetPart('efr32-test-runner', app=HostApp.EFR32_TEST_RUNNER)
    ])

    target.AppendModifier('clang', use_clang=True)

    return target


def BuildHostFakeTarget():
    target = BuildTarget(HostBoard.NATIVE.PlatformName(), HostBuilder)

    target.AppendFixedTargets([
        TargetPart('fake', board=HostBoard.FAKE),
    ])

    target.AppendFixedTargets([
        TargetPart('tests', app=HostApp.TESTS),
    ])

    target.AppendModifier("mbedtls", crypto_library=HostCryptoLibrary.MBEDTLS).ExceptIfRe('-mbedtls')
    target.AppendModifier("boringssl", crypto_library=HostCryptoLibrary.BORINGSSL).ExceptIfRe('-boringssl')
    target.AppendModifier("asan", use_asan=True).ExceptIfRe("-tsan")
    target.AppendModifier("tsan", use_tsan=True).ExceptIfRe("-asan")
    target.AppendModifier("libfuzzer", use_tsan=True).OnlyIfRe("-clang")
    target.AppendModifier('coverage', use_coverage=True).OnlyIfRe('-(chip-tool|all-clusters)')
    target.AppendModifier('dmalloc', use_dmalloc=True)
    target.AppendModifier('clang', use_clang=True)

    return target


def BuildHostTarget():
    native_board_name = HostBoard.NATIVE.BoardName()
    cross_compile = (HostBoard.NATIVE.PlatformName() == 'linux') and (native_board_name != HostBoard.ARM64.BoardName())

    target = BuildTarget(HostBoard.NATIVE.PlatformName(), HostBuilder)

    board_parts = [
        TargetPart(native_board_name, board=HostBoard.NATIVE),
    ]

    if cross_compile:
        board_parts.append(TargetPart('arm64', board=HostBoard.ARM64).OnlyIfRe('-(clang|nodeps)'))

    target.AppendFixedTargets(board_parts)

    # Add all the applications
    app_parts = [
        TargetPart('rpc-console', app=HostApp.RPC_CONSOLE).OnlyIfRe(f'{native_board_name}-'),
        TargetPart('all-clusters', app=HostApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=HostApp.ALL_CLUSTERS),
        TargetPart('chip-tool', app=HostApp.CHIP_TOOL),
        TargetPart('thermostat', app=HostApp.THERMOSTAT),
        TargetPart('minmdns', app=HostApp.MIN_MDNS),
        TargetPart('light', app=HostApp.LIGHT),
        TargetPart('lock', app=HostApp.LOCK),
        TargetPart('shell', app=HostApp.SHELL),
        TargetPart('ota-provider', app=HostApp.OTA_PROVIDER, enable_ble=False),
        TargetPart('ota-requestor', app=HostApp.OTA_REQUESTOR, enable_ble=False),
        TargetPart('python-bindings', app=HostApp.PYTHON_BINDINGS),
        TargetPart('tv-app', app=HostApp.TV_APP),
        TargetPart('tv-casting-app', app=HostApp.TV_CASTING),
        TargetPart('bridge', app=HostApp.BRIDGE),
        TargetPart('dynamic-bridge', app=HostApp.DYNAMIC_BRIDGE),
        TargetPart('tests', app=HostApp.TESTS),
        TargetPart('chip-cert', app=HostApp.CERT_TOOL),
        TargetPart('address-resolve-tool', app=HostApp.ADDRESS_RESOLVE),
    ]

    if (HostBoard.NATIVE.PlatformName() == 'darwin'):
        app_parts.append(TargetPart('darwin-framework-tool', app=HostApp.CHIP_TOOL_DARWIN))

    target.AppendFixedTargets(app_parts)

    target.AppendModifier('nodeps', enable_ble=False, enable_wifi=False, enable_thread=False,
                          crypto_library=HostCryptoLibrary.MBEDTLS, use_clang=True).ExceptIfRe('-(clang|noble|boringssl|mbedtls)')

    target.AppendModifier('minmdns-verbose', minmdns_high_verbosity=True)
    target.AppendModifier('libnl', minmdns_address_policy="libnl")
    target.AppendModifier('same-event-loop', separate_event_loop=False).OnlyIfRe('-(chip-tool|darwin-framework-tool)')
    target.AppendModifier('no-interactive', interactive_mode=False).OnlyIfRe('-chip-tool')
    target.AppendModifier("ipv6only", enable_ipv4=False)
    target.AppendModifier("no-ble", enable_ble=False)
    target.AppendModifier("no-wifi", enable_wifi=False)
    target.AppendModifier("no-thread", enable_thread=False)
    target.AppendModifier("mbedtls", crypto_library=HostCryptoLibrary.MBEDTLS).ExceptIfRe('-mbedtls')
    target.AppendModifier("boringssl", crypto_library=HostCryptoLibrary.BORINGSSL).ExceptIfRe('-boringssl')
    target.AppendModifier("asan", use_asan=True).ExceptIfRe("-tsan")
    target.AppendModifier("tsan", use_tsan=True).ExceptIfRe("-asan")
    target.AppendModifier("libfuzzer", use_tsan=True).OnlyIfRe("-clang")
    target.AppendModifier('coverage', use_coverage=True).OnlyIfRe('-(chip-tool|all-clusters)')
    target.AppendModifier('dmalloc', use_dmalloc=True)
    target.AppendModifier('clang', use_clang=True)
    target.AppendModifier('test', extra_tests=True)
    target.AppendModifier('rpc', enable_rpcs=True)

    return target


def BuildEsp32Target():
    target = BuildTarget('esp32', Esp32Builder)

    # boards
    target.AppendFixedTargets([
        TargetPart('m5stack', board=Esp32Board.M5Stack).OnlyIfRe('-(all-clusters|ota-requestor)'),
        TargetPart('c3devkit', board=Esp32Board.C3DevKit),
        TargetPart('devkitc', board=Esp32Board.DevKitC),
        TargetPart('qemu', board=Esp32Board.QEMU).OnlyIfRe('-tests'),
    ])

    # applications
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=Esp32App.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=Esp32App.ALL_CLUSTERS_MINIMAL),
        TargetPart('ota-requestor', app=Esp32App.OTA_REQUESTOR),
        TargetPart('ota-requestor', app=Esp32App.OTA_REQUESTOR),
        TargetPart('shell', app=Esp32App.SHELL),
        TargetPart('light', app=Esp32App.LIGHT),
        TargetPart('lock', app=Esp32App.LOCK),
        TargetPart('bridge', app=Esp32App.BRIDGE),
        TargetPart('temperature-measurement', app=Esp32App.TEMPERATURE_MEASUREMENT),
        TargetPart('ota-requestor', app=Esp32App.OTA_REQUESTOR),
        TargetPart('tests', app=Esp32App.TESTS).OnlyIfRe('-qemu-'),
    ])

    target.AppendModifier('rpc', enable_rpcs=True)
    target.AppendModifier('ipv6only', enable_ipv4=False)

    return target


def BuildEfr32Target():
    target = BuildTarget('efr32', Efr32Builder)

    # board
    target.AppendFixedTargets([
        TargetPart('brd4161a', board=Efr32Board.BRD4161A),
        TargetPart('brd4187c', board=Efr32Board.BRD4187C),
        TargetPart('brd4163a', board=Efr32Board.BRD4163A),
        TargetPart('brd4164a', board=Efr32Board.BRD4164A),
        TargetPart('brd4166a', board=Efr32Board.BRD4166A),
        TargetPart('brd4170a', board=Efr32Board.BRD4170A),
        TargetPart('brd4186a', board=Efr32Board.BRD4186A),
        TargetPart('brd4187a', board=Efr32Board.BRD4187A),
        TargetPart('brd4304a', board=Efr32Board.BRD4304A),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('window-covering', app=Efr32App.WINDOW_COVERING),
        TargetPart('switch', app=Efr32App.SWITCH),
        TargetPart('unit-test', app=Efr32App.UNIT_TEST),
        TargetPart('light', app=Efr32App.LIGHT),
        TargetPart('lock', app=Efr32App.LOCK),
    ])

    target.AppendModifier('rpc', enable_rpcs=True)
    target.AppendModifier('with-ota-requestor', enable_ota_requestor=True)
    target.AppendModifier('sed', enable_sed=True)
    target.AppendModifier('low-power', enable_low_power=True)

    return target


def BuildNrfNativeTarget():
    target = BuildTarget('nrf', NrfConnectBuilder)

    target.AppendFixedTargets([
        TargetPart('native-posix-64-tests', board=NrfBoard.NATIVE_POSIX_64, app=NrfApp.UNIT_TESTS),
    ])

    return target


def BuildNrfTarget():
    target = BuildTarget('nrf', NrfConnectBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('nrf5340dk', board=NrfBoard.NRF5340DK),
        TargetPart('nrf52840dk', board=NrfBoard.NRF52840DK),
        TargetPart('nrf52840dongle').OnlyIfRe('-(all-clusters|light)'),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=NrfApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=NrfApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('lock', app=NrfApp.LOCK),
        TargetPart('light', app=NrfApp.LIGHT),
        TargetPart('shell', app=NrfApp.SHELL),
        TargetPart('pump', app=NrfApp.PUMP),
        TargetPart('pump-controller', app=NrfApp.PUMP_CONTROLLER),
    ])

    target.AppendModifier('rpc', enable_rpcs=True)

    return target


def BuildAndroidTarget():
    target = BuildTarget('android', AndroidBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('arm', board=AndroidBoard.ARM),
        TargetPart('arm64', board=AndroidBoard.ARM64),
        TargetPart('x86', board=AndroidBoard.X86),
        TargetPart('x64', board=AndroidBoard.X64),
        TargetPart('androidstudio-arm', board=AndroidBoard.AndroidStudio_ARM).OnlyIfRe('chip-tool'),
        TargetPart('androidstudio-arm64', board=AndroidBoard.AndroidStudio_ARM64).OnlyIfRe('chip-tool'),
        TargetPart('androidstudio-x86', board=AndroidBoard.AndroidStudio_X86).OnlyIfRe('chip-tool'),
        TargetPart('androidstudio-x64', board=AndroidBoard.AndroidStudio_X64).OnlyIfRe('chip-tool'),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('chip-tool', app=AndroidApp.CHIP_TOOL),
        TargetPart('chip-test', app=AndroidApp.CHIP_TEST),
        TargetPart('tv-server', app=AndroidApp.TV_SERVER),
        TargetPart('tv-casting-app', app=AndroidApp.TV_CASTING_APP),
        TargetPart('java-matter-controller', app=AndroidApp.JAVA_MATTER_CONTROLLER),
    ])

    return target


def BuildMbedTarget():
    target = BuildTarget('mbed', MbedBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('CY8CPROTO_062_4343W', board=MbedBoard.CY8CPROTO_062_4343W),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('lock', app=MbedApp.LOCK),
        TargetPart('light', app=MbedApp.LIGHT),
        TargetPart('all-clusters', app=MbedApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=MbedApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('pigweed', app=MbedApp.PIGWEED),
        TargetPart('shell', app=MbedApp.SHELL),
    ])

    # Modifiers
    target.AppendModifier('release', profile=MbedProfile.RELEASE).ExceptIfRe('-(develop|debug)')
    target.AppendModifier('develop', profile=MbedProfile.DEVELOP).ExceptIfRe('-(release|debug)')
    target.AppendModifier('debug', profile=MbedProfile.DEBUG).ExceptIfRe('-(release|develop)')

    return target


def BuildInfineonTarget():
    target = BuildTarget('infineon', InfineonBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('psoc6', board=InfineonBoard.PSOC6BOARD)
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('lock', app=InfineonApp.LOCK),
        TargetPart('light', app=InfineonApp.LIGHT),
        TargetPart('all-clusters', app=InfineonApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=InfineonApp.ALL_CLUSTERS_MINIMAL),
    ])

    # modifiers
    target.AppendModifier('ota', enable_ota_requestor=True)
    target.AppendModifier('updateimage', update_image=True)

    return target


def BuildAmebaTarget():
    target = BuildTarget('ameba', AmebaBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('amebad', board=AmebaBoard.AMEBAD),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=AmebaApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=AmebaApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('light', app=AmebaApp.LIGHT),
        TargetPart('pigweed', app=AmebaApp.PIGWEED),
    ])

    return target


def BuildK32WTarget():
    target = BuildTarget('k32w', K32WBuilder)

    # apps
    target.AppendFixedTargets([
        TargetPart('light', app=K32WApp.LIGHT, release=True),
        TargetPart('shell', app=K32WApp.SHELL, release=True),
        TargetPart('lock', app=K32WApp.LOCK, release=True),
        TargetPart('contact', app=K32WApp.CONTACT, release=True),
    ])

    target.AppendModifier(name="se05x", se05x=True)
    target.AppendModifier(name="no-ble", disable_ble=True)
    target.AppendModifier(name="no-ota", disable_ota=True)
    target.AppendModifier(name="low-power", low_power=True).OnlyIfRe("-nologs")
    target.AppendModifier(name="nologs", disable_logs=True)

    return target


def Buildcc13x2x7_26x2x7Target():
    target = BuildTarget('cc13x2x7_26x2x7', cc13x2x7_26x2x7Builder)

    # apps
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=cc13x2x7_26x2x7App.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=cc13x2x7_26x2x7App.ALL_CLUSTERS_MINIMAL),
        TargetPart('lock', app=cc13x2x7_26x2x7App.LOCK),
        TargetPart('pump', app=cc13x2x7_26x2x7App.PUMP),
        TargetPart('pump-controller', app=cc13x2x7_26x2x7App.PUMP_CONTROLLER),
        TargetPart('shell', app=cc13x2x7_26x2x7App.SHELL),
    ])

    target.AppendModifier(name="ftd", openthread_ftd=True).ExceptIfRe("-mtd")
    target.AppendModifier(name="mtd", openthread_ftd=False).ExceptIfRe("-ftd")

    return target


def BuildCyw30739Target():
    target = BuildTarget('cyw30739', Cyw30739Builder)

    # board
    target.AppendFixedTargets([
        TargetPart('cyw930739m2evb_01', board=Cyw30739Board.CYW930739M2EVB_01),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('light', app=Cyw30739App.LIGHT),
        TargetPart('lock',  app=Cyw30739App.LOCK),
        TargetPart('ota-requestor',  app=Cyw30739App.OTA_REQUESTOR),
    ])

    target.AppendModifier(name="no-progress-logging", progress_logging=False)

    return target


def BuildQorvoTarget():
    target = BuildTarget('qpg', QpgBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('qpg6105', board=QpgBoard.QPG6105),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('lock', app=QpgApp.LOCK),
        TargetPart('light', app=QpgApp.LIGHT),
        TargetPart('shell', app=QpgApp.SHELL),
        TargetPart('persistent-storage', app=QpgApp.PERSISTENT_STORAGE),
    ])

    return target


def BuildTizenTarget():
    target = BuildTarget('tizen', TizenBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('arm', board=TizenBoard.ARM),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=TizenApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=TizenApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('chip-tool', app=TizenApp.CHIP_TOOL),
        TargetPart('light', app=TizenApp.LIGHT),
    ])

    target.AppendModifier(name="no-ble", enable_ble=False)
    target.AppendModifier(name="no-wifi", enable_wifi=False)
    target.AppendModifier(name="asan", use_asan=True)

    return target


def BuildBouffalolabTarget():
    target = BuildTarget('bouffalolab', BouffalolabBuilder)

    # Boards
    target.AppendFixedTargets([
        TargetPart('BL602-IoT-Matter-V1', board=BouffalolabBoard.BL602_IoT_Matter_V1, module_type="BL602"),
        TargetPart('BL602-IOT-DVK-3S', board=BouffalolabBoard.BL602_IOT_DVK_3S, module_type="BL602"),
        TargetPart('BL602-NIGHT-LIGHT', board=BouffalolabBoard.BL602_NIGHT_LIGHT, module_type="BL602"),
        TargetPart('XT-ZB6-DevKit', board=BouffalolabBoard.BL706_IoT_DVK, module_type="BL706C-22"),
        TargetPart('BL706-IoT-DVK', board=BouffalolabBoard.BL706_IoT_DVK, module_type="BL706C-22"),
        TargetPart('BL706-NIGHT-LIGHT', board=BouffalolabBoard.BL706_NIGHT_LIGHT, module_type="BL702"),
    ])

    # Apps
    target.AppendFixedTargets([
        TargetPart('light', app=BouffalolabApp.LIGHT),
    ])

    target.AppendModifier('shell', enable_shell=True)
    target.AppendModifier('115200', baudrate=115200)
    target.AppendModifier('rpc', enable_rpcs=True)

    return target


def BuildIMXTarget():
    target = BuildTarget('imx', IMXBuilder)

    target.AppendFixedTargets([
        TargetPart('chip-tool', app=IMXApp.CHIP_TOOL),
        TargetPart('lighting-app', app=IMXApp.LIGHT),
        TargetPart('thermostat', app=IMXApp.THERMOSTAT),
        TargetPart('all-clusters-app', app=IMXApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal-app', app=IMXApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('ota-provider-app', app=IMXApp.OTA_PROVIDER),
    ])

    target.AppendModifier('release', release=True)

    return target


def BuildMW320Target():
    target = BuildTarget('mw320', MW320Builder)
    target.AppendFixedTargets([TargetPart('all-clusters-app', app=MW320App.ALL_CLUSTERS)])
    return target


def BuildGenioTarget():
    target = BuildTarget('genio', GenioBuilder)
    target.AppendFixedTargets([TargetPart('lighting-app', app=GenioApp.LIGHT)])
    return target


def BuildTelinkTarget():
    target = BuildTarget('telink', TelinkBuilder)
    target.AppendFixedTargets([TargetPart('tlsr9518adk80d', board=TelinkBoard.TLSR9518ADK80D)])

    target.AppendFixedTargets([
        TargetPart('all-clusters', app=TelinkApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=TelinkApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('light', app=TelinkApp.LIGHT),
        TargetPart('light-switch', app=TelinkApp.SWITCH),
        TargetPart('ota-requestor', app=TelinkApp.OTA_REQUESTOR),
    ])

    return target


BUILD_TARGETS = [
    BuildAmebaTarget(),
    BuildAndroidTarget(),
    BuildBouffalolabTarget(),
    Buildcc13x2x7_26x2x7Target(),
    BuildCyw30739Target(),
    BuildEfr32Target(),
    BuildEsp32Target(),
    BuildGenioTarget(),
    BuildHostFakeTarget(),
    BuildHostTarget(),
    BuildHostTestRunnerTarget(),
    BuildIMXTarget(),
    BuildInfineonTarget(),
    BuildK32WTarget(),
    BuildMbedTarget(),
    BuildMW320Target(),
    BuildNrfTarget(),
    BuildNrfNativeTarget(),
    BuildQorvoTarget(),
    BuildTizenTarget(),
    BuildTelinkTarget(),
]
