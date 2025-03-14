# Copyright (c) 2021-2024 Project CHIP Authors
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

from builders.ameba import AmebaApp, AmebaBoard, AmebaBuilder
from builders.android import AndroidApp, AndroidBoard, AndroidBuilder, AndroidProfile
from builders.asr import ASRApp, ASRBoard, ASRBuilder
from builders.bouffalolab import BouffalolabApp, BouffalolabBoard, BouffalolabBuilder, BouffalolabThreadType
from builders.cc32xx import cc32xxApp, cc32xxBuilder
from builders.cyw30739 import Cyw30739App, Cyw30739Board, Cyw30739Builder
from builders.efr32 import Efr32App, Efr32Board, Efr32Builder
from builders.esp32 import Esp32App, Esp32Board, Esp32Builder
from builders.genio import GenioApp, GenioBuilder
from builders.host import HostApp, HostBoard, HostBuilder, HostCryptoLibrary, HostFuzzingType
from builders.imx import IMXApp, IMXBuilder
from builders.infineon import InfineonApp, InfineonBoard, InfineonBuilder
from builders.mbed import MbedApp, MbedBoard, MbedBuilder, MbedProfile
from builders.mw320 import MW320App, MW320Builder
from builders.nrf import NrfApp, NrfBoard, NrfConnectBuilder
from builders.nuttx import NuttXApp, NuttXBoard, NuttXBuilder
from builders.nxp import NxpApp, NxpBoard, NxpBoardVariant, NxpBuilder, NxpBuildSystem, NxpLogLevel, NxpOsUsed
from builders.openiotsdk import OpenIotSdkApp, OpenIotSdkBuilder, OpenIotSdkCryptoBackend
from builders.qpg import QpgApp, QpgBoard, QpgBuilder
from builders.stm32 import stm32App, stm32Board, stm32Builder
from builders.telink import TelinkApp, TelinkBoard, TelinkBuilder
from builders.ti import TIApp, TIBoard, TIBuilder
from builders.tizen import TizenApp, TizenBoard, TizenBuilder

from .target import BuildTarget, TargetPart


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

    target.AppendModifier(
        "mbedtls", crypto_library=HostCryptoLibrary.MBEDTLS).ExceptIfRe('-boringssl')
    target.AppendModifier(
        "boringssl", crypto_library=HostCryptoLibrary.BORINGSSL).ExceptIfRe('-mbedtls')
    target.AppendModifier("asan", use_asan=True).ExceptIfRe("-tsan")
    target.AppendModifier("tsan", use_tsan=True).ExceptIfRe("-asan")
    target.AppendModifier("ubsan", use_ubsan=True)
    target.AppendModifier("libfuzzer", fuzzing_type=HostFuzzingType.LIB_FUZZER).OnlyIfRe(
        "-clang").ExceptIfRe('-ossfuzz')
    target.AppendModifier("ossfuzz", fuzzing_type=HostFuzzingType.OSS_FUZZ).OnlyIfRe(
        "-clang").ExceptIfRe('-libfuzzer')
    target.AppendModifier("pw-fuzztest", fuzzing_type=HostFuzzingType.PW_FUZZTEST).OnlyIfRe(
        "-clang").ExceptIfRe('-(libfuzzer|ossfuzz|asan)')
    target.AppendModifier('coverage', use_coverage=True)
    target.AppendModifier('dmalloc', use_dmalloc=True)
    target.AppendModifier('clang', use_clang=True)

    return target


def BuildHostTarget():
    native_board_name = HostBoard.NATIVE.BoardName()
    cross_compile = (HostBoard.NATIVE.PlatformName() == 'linux') and (
        native_board_name != HostBoard.ARM64.BoardName())

    target = BuildTarget(HostBoard.NATIVE.PlatformName(), HostBuilder)

    board_parts = [
        TargetPart(native_board_name, board=HostBoard.NATIVE),
    ]

    if cross_compile:
        board_parts.append(TargetPart(
            'arm64', board=HostBoard.ARM64).OnlyIfRe('-(clang|nodeps)'))

    target.AppendFixedTargets(board_parts)

    # Add all the applications
    app_parts = [
        TargetPart('rpc-console',
                   app=HostApp.RPC_CONSOLE).OnlyIfRe(f'{native_board_name}-'),
        TargetPart('all-clusters', app=HostApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=HostApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('chip-tool', app=HostApp.CHIP_TOOL),
        TargetPart('thermostat', app=HostApp.THERMOSTAT),
        # TODO: controllers depending on a datamodel is odd. For now fix compile dependencies on ember.
        TargetPart('java-matter-controller', app=HostApp.JAVA_MATTER_CONTROLLER),
        TargetPart('kotlin-matter-controller', app=HostApp.KOTLIN_MATTER_CONTROLLER),
        TargetPart('minmdns', app=HostApp.MIN_MDNS),
        TargetPart('light', app=HostApp.LIGHT),
        TargetPart('light-data-model-no-unique-id', app=HostApp.LIGHT_DATA_MODEL_NO_UNIQUE_ID),
        TargetPart('lock', app=HostApp.LOCK),
        TargetPart('shell', app=HostApp.SHELL),
        TargetPart('ota-provider', app=HostApp.OTA_PROVIDER, enable_ble=False),
        TargetPart('ota-requestor', app=HostApp.OTA_REQUESTOR,
                   enable_ble=False),
        TargetPart('simulated-app1', app=HostApp.SIMULATED_APP1,
                   enable_ble=False),
        TargetPart('simulated-app2', app=HostApp.SIMULATED_APP2,
                   enable_ble=False),
        TargetPart('python-bindings', app=HostApp.PYTHON_BINDINGS),
        TargetPart('tv-app', app=HostApp.TV_APP),
        TargetPart('tv-casting-app', app=HostApp.TV_CASTING),
        TargetPart('bridge', app=HostApp.BRIDGE),
        TargetPart('fabric-admin', app=HostApp.FABRIC_ADMIN).OnlyIfRe("-rpc"),
        TargetPart('fabric-bridge', app=HostApp.FABRIC_BRIDGE).OnlyIfRe("-rpc"),
        TargetPart('fabric-sync', app=HostApp.FABRIC_SYNC),
        TargetPart('tests', app=HostApp.TESTS),
        TargetPart('chip-cert', app=HostApp.CERT_TOOL),
        TargetPart('address-resolve-tool', app=HostApp.ADDRESS_RESOLVE),
        TargetPart('contact-sensor', app=HostApp.CONTACT_SENSOR),
        TargetPart('dishwasher', app=HostApp.DISHWASHER),
        TargetPart('microwave-oven', app=HostApp.MICROWAVE_OVEN),
        TargetPart('refrigerator', app=HostApp.REFRIGERATOR),
        TargetPart('rvc', app=HostApp.RVC),
        TargetPart('air-purifier', app=HostApp.AIR_PURIFIER),
        TargetPart('lit-icd', app=HostApp.LIT_ICD),
        TargetPart('air-quality-sensor', app=HostApp.AIR_QUALITY_SENSOR),
        TargetPart('network-manager', app=HostApp.NETWORK_MANAGER),
        TargetPart('energy-management', app=HostApp.ENERGY_MANAGEMENT),
        TargetPart('water-leak-detector', app=HostApp.WATER_LEAK_DETECTOR),
        TargetPart('terms-and-conditions', app=HostApp.TERMS_AND_CONDITIONS),
        TargetPart('camera', app=HostApp.CAMERA),
        TargetPart('camera-controller', app=HostApp.CAMERA_CONTROLLER),
    ]

    if (HostBoard.NATIVE.PlatformName() == 'darwin'):
        app_parts.append(TargetPart('darwin-framework-tool',
                         app=HostApp.CHIP_TOOL_DARWIN))

    target.AppendFixedTargets(app_parts)

    target.AppendModifier('nodeps', enable_ble=False, enable_wifi=False, enable_thread=False,
                          crypto_library=HostCryptoLibrary.MBEDTLS, use_clang=True).ExceptIfRe('-(clang|noble|boringssl|mbedtls)')

    target.AppendModifier('nlfaultinject', use_nl_fault_injection=True)
    target.AppendModifier('platform-mdns', use_platform_mdns=True)
    target.AppendModifier('minmdns-verbose', minmdns_high_verbosity=True)
    target.AppendModifier('libnl', minmdns_address_policy="libnl")
    target.AppendModifier(
        'same-event-loop', separate_event_loop=False).OnlyIfRe('-(chip-tool|darwin-framework-tool)')
    target.AppendModifier(
        'no-interactive', interactive_mode=False).OnlyIfRe('-chip-tool')
    target.AppendModifier("ipv6only", enable_ipv4=False)
    target.AppendModifier("no-ble", enable_ble=False)
    target.AppendModifier("no-wifi", enable_wifi=False)
    target.AppendModifier("no-thread", enable_thread=False)
    target.AppendModifier('no-shell', disable_shell=True)
    target.AppendModifier(
        "mbedtls", crypto_library=HostCryptoLibrary.MBEDTLS).ExceptIfRe('-boringssl')
    target.AppendModifier(
        "boringssl", crypto_library=HostCryptoLibrary.BORINGSSL).ExceptIfRe('-mbedtls')
    target.AppendModifier("asan", use_asan=True).ExceptIfRe("-tsan")
    target.AppendModifier("tsan", use_tsan=True).ExceptIfRe("-asan")
    target.AppendModifier("ubsan", use_ubsan=True)
    target.AppendModifier("libfuzzer", fuzzing_type=HostFuzzingType.LIB_FUZZER).OnlyIfRe(
        "-clang").ExceptIfRe('-ossfuzz')
    target.AppendModifier("ossfuzz", fuzzing_type=HostFuzzingType.OSS_FUZZ).OnlyIfRe(
        "-clang").ExceptIfRe('-libfuzzer')
    target.AppendModifier("pw-fuzztest", fuzzing_type=HostFuzzingType.PW_FUZZTEST).OnlyIfRe(
        "-clang").ExceptIfRe('-(libfuzzer|ossfuzz|asan)')
    target.AppendModifier('coverage', use_coverage=True)
    target.AppendModifier('dmalloc', use_dmalloc=True)
    target.AppendModifier('clang', use_clang=True)
    target.AppendModifier('test', extra_tests=True)
    target.AppendModifier('rpc', enable_rpcs=True)
    target.AppendModifier('with-ui', imgui_ui=True)
    target.AppendModifier('evse-test-event', enable_test_event_triggers=['EVSE']).OnlyIfRe('-energy-management')
    target.AppendModifier('enable-dnssd-tests', enable_dnssd_tests=True).OnlyIfRe('-tests')
    target.AppendModifier('disable-dnssd-tests', enable_dnssd_tests=False).OnlyIfRe('-tests')
    target.AppendModifier('chip-casting-simplified', chip_casting_simplified=True).OnlyIfRe('-tv-casting-app')
    target.AppendModifier('googletest', use_googletest=True).OnlyIfRe('-tests')
    target.AppendModifier('terms-and-conditions', terms_and_conditions_required=True)

    return target


def BuildEsp32Target():
    target = BuildTarget('esp32', Esp32Builder)

    # boards
    target.AppendFixedTargets([
        TargetPart('m5stack', board=Esp32Board.M5Stack),
        TargetPart('c3devkit', board=Esp32Board.C3DevKit),
        TargetPart('devkitc', board=Esp32Board.DevKitC),
        TargetPart('qemu', board=Esp32Board.QEMU).OnlyIfRe('-tests'),
    ])

    # applications
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=Esp32App.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=Esp32App.ALL_CLUSTERS_MINIMAL),
        TargetPart('energy-management', app=Esp32App.ENERGY_MANAGEMENT),
        TargetPart('ota-provider', app=Esp32App.OTA_PROVIDER),
        TargetPart('ota-requestor', app=Esp32App.OTA_REQUESTOR),
        TargetPart('shell', app=Esp32App.SHELL),
        TargetPart('light', app=Esp32App.LIGHT),
        TargetPart('lock', app=Esp32App.LOCK),
        TargetPart('bridge', app=Esp32App.BRIDGE),
        TargetPart('temperature-measurement',
                   app=Esp32App.TEMPERATURE_MEASUREMENT),
        TargetPart('ota-requestor', app=Esp32App.OTA_REQUESTOR),
        TargetPart('tests', app=Esp32App.TESTS).OnlyIfRe('-qemu-'),
    ])

    target.AppendModifier('rpc', enable_rpcs=True)
    target.AppendModifier('ipv6only', enable_ipv4=False)
    target.AppendModifier('tracing', enable_insights_trace=True).OnlyIfRe("light")

    return target


def BuildEfr32Target():
    target = BuildTarget('efr32', Efr32Builder)

    # board
    target.AppendFixedTargets([
        TargetPart('brd2704b', board=Efr32Board.BRD2704B),
        TargetPart('brd4316a', board=Efr32Board.BRD4316A),
        TargetPart('brd4317a', board=Efr32Board.BRD4317A),
        TargetPart('brd4318a', board=Efr32Board.BRD4318A),
        TargetPart('brd4319a', board=Efr32Board.BRD4319A),
        TargetPart('brd4186a', board=Efr32Board.BRD4186A),
        TargetPart('brd4187a', board=Efr32Board.BRD4187A),
        TargetPart('brd2601b', board=Efr32Board.BRD2601B),
        TargetPart('brd4187c', board=Efr32Board.BRD4187C),
        TargetPart('brd4186c', board=Efr32Board.BRD4186C),
        TargetPart('brd2703a', board=Efr32Board.BRD2703A),
        TargetPart('brd4338a', board=Efr32Board.BRD4338A, enable_wifi=True, enable_917_soc=True),
        TargetPart('brd2605a', board=Efr32Board.BRD2605A, enable_wifi=True, enable_917_soc=True),
        TargetPart('brd4343a', board=Efr32Board.BRD4343A, enable_wifi=True, enable_917_soc=True),
        TargetPart('brd4342a', board=Efr32Board.BRD4342A, enable_wifi=True, enable_917_soc=True),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('window-covering', app=Efr32App.WINDOW_COVERING),
        TargetPart('switch', app=Efr32App.SWITCH),
        TargetPart('unit-test', app=Efr32App.UNIT_TEST),
        TargetPart('light', app=Efr32App.LIGHT),
        TargetPart('lock', app=Efr32App.LOCK),
        TargetPart('thermostat', app=Efr32App.THERMOSTAT),
        TargetPart('pump', app=Efr32App.PUMP),
        TargetPart('air-quality-sensor-app', app=Efr32App.AIR_QUALITY_SENSOR)
    ])

    target.AppendModifier('rpc', enable_rpcs=True)
    target.AppendModifier('with-ota-requestor', enable_ota_requestor=True)
    target.AppendModifier('icd', enable_icd=True)
    target.AppendModifier('low-power', enable_low_power=True).OnlyIfRe('-icd')
    target.AppendModifier('shell', chip_build_libshell=True)
    target.AppendModifier('no-logging', chip_logging=False)
    target.AppendModifier('openthread-mtd', chip_openthread_ftd=False)
    target.AppendModifier('heap-monitoring',
                          enable_heap_monitoring=True)
    target.AppendModifier('no-openthread-cli', enable_openthread_cli=False)
    target.AppendModifier(
        'show-qr-code', show_qr_code=True).ExceptIfRe('-low-power')
    target.AppendModifier('wifi', enable_wifi=True)
    target.AppendModifier('rs9116', enable_rs9116=True).OnlyIfRe('-wifi')
    target.AppendModifier('wf200', enable_wf200=True).OnlyIfRe('-wifi')
    target.AppendModifier('siwx917', enable_917_ncp=True).OnlyIfRe('-wifi')
    target.AppendModifier('ipv4', enable_wifi_ipv4=True).OnlyIfRe('-wifi')
    target.AppendModifier('additional-data-advertising',
                          enable_additional_data_advertising=True)
    target.AppendModifier('use-ot-lib', enable_ot_lib=True).ExceptIfRe(
        '-(wifi|use-ot-coap-lib)')
    target.AppendModifier('use-ot-coap-lib', enable_ot_coap_lib=True).ExceptIfRe(
        '-(wifi|use-ot-lib)')
    target.AppendModifier('no-version', no_version=True)
    target.AppendModifier('skip-rps-generation', use_rps_extension=False)

    return target


def BuildNrfNativeTarget():
    target = BuildTarget('nrf', NrfConnectBuilder)

    target.AppendFixedTargets([
        TargetPart('native-sim-tests',
                   board=NrfBoard.NATIVE_SIM, app=NrfApp.UNIT_TESTS),
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
        TargetPart('light-switch', app=NrfApp.SWITCH),
        TargetPart('shell', app=NrfApp.SHELL),
        TargetPart('pump', app=NrfApp.PUMP),
        TargetPart('pump-controller', app=NrfApp.PUMP_CONTROLLER),
        TargetPart('window-covering', app=NrfApp.WINDOW_COVERING),
    ])

    target.AppendModifier('rpc', enable_rpcs=True)

    return target


def BuildNuttXTarget():
    target = BuildTarget('nuttx', NuttXBuilder)

    # Boards
    target.AppendFixedTargets([
        TargetPart('x64', board=NuttXBoard.SIM),
    ])

    # Apps
    target.AppendFixedTargets([
        TargetPart('light', app=NuttXApp.LIGHT),
    ])

    return target


def BuildAndroidTarget():
    target = BuildTarget('android', AndroidBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('arm', board=AndroidBoard.ARM),
        TargetPart('arm64', board=AndroidBoard.ARM64),
        TargetPart('x86', board=AndroidBoard.X86),
        TargetPart('x64', board=AndroidBoard.X64),
        TargetPart('androidstudio-arm',
                   board=AndroidBoard.AndroidStudio_ARM).OnlyIfRe('chip-tool'),
        TargetPart('androidstudio-arm64',
                   board=AndroidBoard.AndroidStudio_ARM64).OnlyIfRe('chip-tool'),
        TargetPart('androidstudio-x86',
                   board=AndroidBoard.AndroidStudio_X86).OnlyIfRe('chip-tool'),
        TargetPart('androidstudio-x64',
                   board=AndroidBoard.AndroidStudio_X64).OnlyIfRe('chip-tool'),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('chip-tool', app=AndroidApp.CHIP_TOOL),
        TargetPart('chip-test', app=AndroidApp.CHIP_TEST),
        TargetPart('tv-server', app=AndroidApp.TV_SERVER),
        TargetPart('tv-casting-app', app=AndroidApp.TV_CASTING_APP),
        TargetPart('java-matter-controller',
                   app=AndroidApp.JAVA_MATTER_CONTROLLER),
        TargetPart('kotlin-matter-controller',
                   app=AndroidApp.KOTLIN_MATTER_CONTROLLER),
        TargetPart('virtual-device-app',
                   app=AndroidApp.VIRTUAL_DEVICE_APP),
    ])

    # Modifiers
    target.AppendModifier('no-debug', profile=AndroidProfile.RELEASE)

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
        TargetPart('ota-requestor', app=MbedApp.OTA_REQUESTOR),
        TargetPart('shell', app=MbedApp.SHELL),
    ])

    # Modifiers
    target.AppendModifier('release', profile=MbedProfile.RELEASE).ExceptIfRe(
        '-(develop|debug)')
    target.AppendModifier('develop', profile=MbedProfile.DEVELOP).ExceptIfRe(
        '-(release|debug)')
    target.AppendModifier('debug', profile=MbedProfile.DEBUG).ExceptIfRe(
        '-(release|develop)')

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
        TargetPart('all-clusters-minimal',
                   app=InfineonApp.ALL_CLUSTERS_MINIMAL),
    ])

    # modifiers
    target.AppendModifier('ota', enable_ota_requestor=True)
    target.AppendModifier('updateimage', update_image=True)
    target.AppendModifier('trustm', enable_trustm=True)

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
        TargetPart('light-switch', app=AmebaApp.LIGHT_SWITCH),
        TargetPart('pigweed', app=AmebaApp.PIGWEED),
    ])

    return target


def BuildASRTarget():
    target = BuildTarget('asr', ASRBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('asr582x', board=ASRBoard.ASR582X),
        TargetPart('asr595x', board=ASRBoard.ASR595X),
        TargetPart('asr550x', board=ASRBoard.ASR550X),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('all-clusters', app=ASRApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=ASRApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('lighting', app=ASRApp.LIGHT),
        TargetPart('light-switch', app=ASRApp.LIGHT_SWITCH),
        TargetPart('lock', app=ASRApp.LOCK),
        TargetPart('bridge', app=ASRApp.BRIDGE),
        TargetPart('temperature-measurement', app=ASRApp.TEMPERATURE_MEASUREMENT),
        TargetPart('thermostat', app=ASRApp.THERMOSTAT),
        TargetPart('ota-requestor', app=ASRApp.OTA_REQUESTOR),
        TargetPart('dishwasher', app=ASRApp.DISHWASHER),
        TargetPart('refrigerator', app=ASRApp.REFRIGERATOR),
    ])

    # modifiers
    target.AppendModifier('ota', enable_ota_requestor=True)
    target.AppendModifier('shell', chip_build_libshell=True)
    target.AppendModifier('no_logging', chip_logging=False)
    target.AppendModifier('factory', enable_factory=True)
    target.AppendModifier('rotating_id', enable_rotating_device_id=True)
    target.AppendModifier('rio', enable_lwip_ip6_hook=True)

    return target


def BuildNxpTarget():
    target = BuildTarget('nxp', NxpBuilder)

    # boards
    target.AppendFixedTargets([
        TargetPart('k32w0', board=NxpBoard.K32W0),
        TargetPart('k32w1', board=NxpBoard.K32W1),
        TargetPart('rt1060', board=NxpBoard.RT1060),
        TargetPart('rt1170', board=NxpBoard.RT1170),
        TargetPart('rw61x', board=NxpBoard.RW61X),
        TargetPart('rw61x_eth', board=NxpBoard.RW61X_ETH),
        TargetPart('mcxw71', board=NxpBoard.MCXW71)
    ])

    # OS
    target.AppendFixedTargets([
        TargetPart('zephyr', os_env=NxpOsUsed.ZEPHYR).OnlyIfRe('rw61x'),
        TargetPart('freertos', os_env=NxpOsUsed.FREERTOS),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('lighting', app=NxpApp.LIGHTING).OnlyIfRe('(k32w0|k32w1|mcxw71)'),
        TargetPart('contact-sensor', app=NxpApp.CONTACT).OnlyIfRe('(k32w0|k32w1|mcxw71)'),
        TargetPart('lock-app', app=NxpApp.LOCK_APP).OnlyIfRe('(k32w1|mcxw71)'),
        TargetPart('all-clusters', app=NxpApp.ALLCLUSTERS).OnlyIfRe('rt1060|rt1170|rw61x'),
        TargetPart('laundry-washer', app=NxpApp.LAUNDRYWASHER).OnlyIfRe('rt1060|rt1170|rw61x'),
        TargetPart('thermostat', app=NxpApp.THERMOSTAT).OnlyIfRe('rt1060|rt1170|rw61x')
    ])

    target.AppendModifier(name="factory", enable_factory_data=True)
    target.AppendModifier(name="low-power", low_power=True).OnlyIfRe('contact-sensor')
    target.AppendModifier(name="lit", enable_lit=True).OnlyIfRe('contact-sensor')
    target.AppendModifier(name="fro32k", use_fro32k=True).OnlyIfRe('k32w0')
    target.AppendModifier(name="smu2", smu2=True).OnlyIfRe('(k32w1|mcxw71)-freertos-lighting')
    target.AppendModifier(name="dac-conversion", convert_dac_pk=True).OnlyIfRe('factory').ExceptIfRe('(k32w0|rw61x)')
    target.AppendModifier(name="rotating-id", enable_rotating_id=True).ExceptIfRe('rw61x')
    target.AppendModifier(name="sw-v2", has_sw_version_2=True)
    target.AppendModifier(name="ota", enable_ota=True).ExceptIfRe('zephyr')
    target.AppendModifier(name="wifi", enable_wifi=True).OnlyIfRe('rt1060|rt1170|rw61x')
    target.AppendModifier(name="ethernet", enable_ethernet=True).OnlyIfRe('rw61x_eth-zephyr')
    target.AppendModifier(name="thread", enable_thread=True).ExceptIfRe('zephyr')
    target.AppendModifier(name="matter-shell", enable_shell=True).ExceptIfRe('k32w0|k32w1')
    target.AppendModifier(name="factory-build", enable_factory_data_build=True).OnlyIfRe('rt1060|rt1170|rw61x')
    target.AppendModifier(name="frdm", board_variant=NxpBoardVariant.FRDM).OnlyIfRe('rw61x')
    target.AppendModifier(name="cmake", build_system=NxpBuildSystem.CMAKE).OnlyIfRe('rw61x')
    target.AppendModifier(name="evkc", board_variant=NxpBoardVariant.EVKC).OnlyIfRe('rt1060')
    target.AppendModifier(name="iw416", iw416_transceiver=True).OnlyIfRe('rt1060')
    target.AppendModifier(name="w8801", w8801_transceiver=True).OnlyIfRe('rt1060')
    target.AppendModifier(name="iwx12", iwx12_transceiver=True).OnlyIfRe('rt1060|rt1170')
    target.AppendModifier(name="log-all", log_level=NxpLogLevel.ALL).ExceptIfRe("-log-(progress|error|none)")
    target.AppendModifier(name="log-progress", log_level=NxpLogLevel.PROGRESS).ExceptIfRe("-log-(all|error|none)")
    target.AppendModifier(name="log-error", log_level=NxpLogLevel.ERROR).ExceptIfRe("-log-(progress|all|none)")
    target.AppendModifier(name="log-none", log_level=NxpLogLevel.NONE).ExceptIfRe("-log-(progress|error|all)")

    return target


def BuildCC13x4Target():
    target = BuildTarget('ti', TIBuilder)

    # board
    target.AppendFixedTargets([
        TargetPart('cc13x4_26x4', board=TIBoard.LP_EM_CC1354P10_6)
    ])

    target.AppendFixedTargets([
        TargetPart('lighting', app=TIApp.LIGHTING),
        TargetPart('lock', app=TIApp.LOCK, openthread_ftd=True),
        TargetPart('pump', app=TIApp.PUMP, openthread_ftd=False),
        TargetPart('pump-controller', app=TIApp.PUMP_CONTROLLER,
                   openthread_ftd=False),
    ])

    target.AppendModifier(name="mtd", openthread_ftd=False)
    target.AppendModifier(name="ftd", openthread_ftd=True)

    return target


def Buildcc32xxTarget():
    target = BuildTarget('cc32xx', cc32xxBuilder)

    # apps
    target.AppendFixedTargets([
        TargetPart('lock', app=cc32xxApp.LOCK),
        TargetPart('air-purifier', app=cc32xxApp.AIR_PURIFIER),

    ])

    return target


def BuildCyw30739Target():
    target = BuildTarget('cyw30739', Cyw30739Builder)
    # board
    target.AppendFixedTargets([
        TargetPart('cyw30739b2_p5_evk_01', board=Cyw30739Board.CYW30739B2_P5_EVK_01),
        TargetPart('cyw30739b2_p5_evk_02', board=Cyw30739Board.CYW30739B2_P5_EVK_02),
        TargetPart('cyw30739b2_p5_evk_03', board=Cyw30739Board.CYW30739B2_P5_EVK_03),
        TargetPart('cyw930739m2evb_01', board=Cyw30739Board.CYW930739M2EVB_01),
        TargetPart('cyw930739m2evb_02', board=Cyw30739Board.CYW930739M2EVB_02),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('light', app=Cyw30739App.LIGHT),
        TargetPart('light-switch', app=Cyw30739App.LIGHT_SWITCH),
        TargetPart('lock', app=Cyw30739App.LOCK),
        TargetPart('thermostat', app=Cyw30739App.THERMOSTAT),
    ])

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
        TargetPart('light-switch', app=QpgApp.LIGHT_SWITCH),
        TargetPart('thermostat', app=QpgApp.THERMOSTAT),
    ])

    target.AppendModifier('updateimage', update_image=True)

    return target


def BuildStm32Target():
    target = BuildTarget('stm32', stm32Builder)

    # board
    target.AppendFixedTargets([
        TargetPart('STM32WB5MM-DK', board=stm32Board.STM32WB55XX),
    ])

    # apps
    target.AppendFixedTargets([
        TargetPart('light', app=stm32App.LIGHT),
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
        TargetPart('chip-tool', app=TizenApp.CHIP_TOOL),
        TargetPart('light', app=TizenApp.LIGHT),
        TargetPart('tests', app=TizenApp.TESTS),
    ])

    target.AppendModifier("no-ble", enable_ble=False)
    target.AppendModifier("no-thread", enable_thread=False)
    target.AppendModifier("no-wifi", enable_wifi=False)
    target.AppendModifier("asan", use_asan=True)
    target.AppendModifier("ubsan", use_ubsan=True)
    target.AppendModifier('coverage', use_coverage=True).OnlyIfRe(
        '-tests')
    target.AppendModifier('with-ui', with_ui=True)

    return target


def BuildBouffalolabTarget():
    target = BuildTarget('bouffalolab', BouffalolabBuilder)

    # Boards
    target.AppendFixedTargets([
        TargetPart('BL602DK',
                   board=BouffalolabBoard.BL602DK, module_type="BL602"),
        TargetPart('BL616DK', board=BouffalolabBoard.BL616DK, module_type="BL616"),
        TargetPart('BL704LDK', board=BouffalolabBoard.BL704LDK, module_type="BL704L"),
        TargetPart('BL706DK',
                   board=BouffalolabBoard.BL706DK, module_type="BL706C-22"),
        TargetPart('BL602-NIGHT-LIGHT',
                   board=BouffalolabBoard.BL602_NIGHT_LIGHT, module_type="BL602", enable_resetCnt=True),
        TargetPart('BL706-NIGHT-LIGHT',
                   board=BouffalolabBoard.BL706_NIGHT_LIGHT, module_type="BL706C-22", enable_resetCnt=True),
        TargetPart('BL602-IoT-Matter-V1',
                   board=BouffalolabBoard.BL602_IoT_Matter_V1, module_type="BL602"),
        TargetPart('XT-ZB6-DevKit', board=BouffalolabBoard.XT_ZB6_DevKit,
                   module_type="BL706C-22"),
    ])

    target.AppendFixedTargets([
        TargetPart('light', app=BouffalolabApp.LIGHT),
        TargetPart('contact-sensor', app=BouffalolabApp.CONTACT, enable_pds=True).OnlyIfRe('-(bl704l)'),
    ])

    target.AppendFixedTargets([
        TargetPart('ethernet', enable_ethernet=True),
        TargetPart('wifi', enable_wifi=True),
        TargetPart('thread', enable_thread_type=BouffalolabThreadType.THREAD_FTD),
        TargetPart('thread-ftd', enable_thread_type=BouffalolabThreadType.THREAD_FTD),
        TargetPart('thread-mtd', enable_thread_type=BouffalolabThreadType.THREAD_MTD),
    ])

    target.AppendFixedTargets([
        TargetPart('easyflash', enable_easyflash=True),
        TargetPart('littlefs', enable_littlefs=True),
    ])

    target.AppendModifier('shell', enable_shell=True)
    target.AppendModifier('mfd', enable_mfd=True)
    target.AppendModifier('rotating_device_id', enable_rotating_device_id=True)
    target.AppendModifier('rpc', enable_rpcs=True, baudrate=115200).OnlyIfRe('-(bl602dk|bl704ldk|bl706dk)')
    target.AppendModifier('cdc', enable_cdc=True).OnlyIfRe('-(bl706dk)')
    target.AppendModifier('mot', use_matter_openthread=True).OnlyIfRe('-(thread)')
    target.AppendModifier('memmonitor', enable_heap_monitoring=True)
    target.AppendModifier('coredump', enable_debug_coredump=True)

    return target


def BuildIMXTarget():
    target = BuildTarget('imx', IMXBuilder)

    target.AppendFixedTargets([
        TargetPart('chip-tool', app=IMXApp.CHIP_TOOL),
        TargetPart('lighting-app', app=IMXApp.LIGHT),
        TargetPart('thermostat', app=IMXApp.THERMOSTAT),
        TargetPart('all-clusters-app', app=IMXApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal-app',
                   app=IMXApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('ota-provider-app', app=IMXApp.OTA_PROVIDER),
    ])

    target.AppendModifier('release', release=True)

    return target


def BuildMW320Target():
    target = BuildTarget('mw320', MW320Builder)
    target.AppendFixedTargets(
        [TargetPart('all-clusters-app', app=MW320App.ALL_CLUSTERS)])
    return target


def BuildGenioTarget():
    target = BuildTarget('genio', GenioBuilder)
    target.AppendFixedTargets([TargetPart('lighting-app', app=GenioApp.LIGHT)])
    return target


def BuildTelinkTarget():
    target = BuildTarget('telink', TelinkBuilder)

    target.AppendFixedTargets([
        TargetPart('tlsr9118bdk40d', board=TelinkBoard.TLRS9118BDK40D),
        TargetPart('tlsr9518adk80d', board=TelinkBoard.TLSR9518ADK80D),
        TargetPart('tlsr9528a', board=TelinkBoard.TLSR9528A),
        TargetPart('tlsr9528a_retention', board=TelinkBoard.TLSR9528A_RETENTION),
        TargetPart('tl3218x', board=TelinkBoard.TL3218X),
        TargetPart('tl3218x_retention', board=TelinkBoard.TL3218X_RETENTION),
        TargetPart('tl7218x', board=TelinkBoard.TL7218X),
        TargetPart('tl7218x_retention', board=TelinkBoard.TL7218X_RETENTION),
    ])

    target.AppendFixedTargets([
        TargetPart('air-quality-sensor', app=TelinkApp.AIR_QUALITY_SENSOR),
        TargetPart('all-clusters', app=TelinkApp.ALL_CLUSTERS),
        TargetPart('all-clusters-minimal', app=TelinkApp.ALL_CLUSTERS_MINIMAL),
        TargetPart('bridge', app=TelinkApp.BRIDGE),
        TargetPart('contact-sensor', app=TelinkApp.CONTACT_SENSOR),
        TargetPart('light', app=TelinkApp.LIGHT),
        TargetPart('light-switch', app=TelinkApp.SWITCH),
        TargetPart('lock', app=TelinkApp.LOCK),
        TargetPart('ota-requestor', app=TelinkApp.OTA_REQUESTOR),
        TargetPart('pump', app=TelinkApp.PUMP),
        TargetPart('pump-controller', app=TelinkApp.PUMP_CONTROLLER),
        TargetPart('shell', app=TelinkApp.SHELL),
        TargetPart('smoke-co-alarm', app=TelinkApp.SMOKE_CO_ALARM),
        TargetPart('temperature-measurement',
                   app=TelinkApp.TEMPERATURE_MEASUREMENT),
        TargetPart('thermostat', app=TelinkApp.THERMOSTAT),
        TargetPart('window-covering', app=TelinkApp.WINDOW_COVERING),
    ])

    target.AppendModifier('ota', enable_ota=True)
    target.AppendModifier('dfu', enable_dfu=True)
    target.AppendModifier('shell', enable_shell=True)
    target.AppendModifier('rpc', enable_rpcs=True)
    target.AppendModifier('factory-data', enable_factory_data=True)
    target.AppendModifier('4mb', enable_4mb_flash=True)
    target.AppendModifier('mars', mars_board_config=True)
    target.AppendModifier('usb', usb_board_config=True)
    target.AppendModifier('compress-lzma', compress_lzma_config=True)
    target.AppendModifier('thread-analyzer', thread_analyzer_config=True)

    return target


def BuildOpenIotSdkTargets():
    target = BuildTarget('openiotsdk', OpenIotSdkBuilder)

    target.AppendFixedTargets([
        TargetPart('shell', app=OpenIotSdkApp.SHELL),
        TargetPart('lock', app=OpenIotSdkApp.LOCK),
    ])

    # Modifiers
    target.AppendModifier('mbedtls', crypto=OpenIotSdkCryptoBackend.MBEDTLS).ExceptIfRe('-(psa)')
    target.AppendModifier('psa', crypto=OpenIotSdkCryptoBackend.PSA).ExceptIfRe('-(mbedtls)')

    return target


BUILD_TARGETS = [
    BuildAmebaTarget(),
    BuildASRTarget(),
    BuildAndroidTarget(),
    BuildBouffalolabTarget(),
    Buildcc32xxTarget(),
    BuildCC13x4Target(),
    BuildCyw30739Target(),
    BuildEfr32Target(),
    BuildEsp32Target(),
    BuildGenioTarget(),
    BuildHostFakeTarget(),
    BuildHostTarget(),
    BuildHostTestRunnerTarget(),
    BuildIMXTarget(),
    BuildInfineonTarget(),
    BuildNxpTarget(),
    BuildMbedTarget(),
    BuildMW320Target(),
    BuildNrfTarget(),
    BuildNrfNativeTarget(),
    BuildNuttXTarget(),
    BuildQorvoTarget(),
    BuildStm32Target(),
    BuildTizenTarget(),
    BuildTelinkTarget(),
    BuildOpenIotSdkTargets(),
]
