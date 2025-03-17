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
import shlex
from enum import Enum, auto
from platform import uname
from typing import Optional

from .builder import BuilderOutput
from .gn import GnBuilder


class HostCryptoLibrary(Enum):
    """Defines what cryptographic backend applications should use."""
    OPENSSL = auto()
    MBEDTLS = auto()
    BORINGSSL = auto()

    @property
    def gn_argument(self):
        if self == HostCryptoLibrary.OPENSSL:
            return 'chip_crypto="openssl"'
        elif self == HostCryptoLibrary.MBEDTLS:
            return 'chip_crypto="mbedtls"'
        elif self == HostCryptoLibrary.BORINGSSL:
            return 'chip_crypto="boringssl"'


class HostFuzzingType(Enum):
    """Defines fuzz target options available for host targets."""
    NONE = auto()
    LIB_FUZZER = auto()
    OSS_FUZZ = auto()
    PW_FUZZTEST = auto()


class HostApp(Enum):
    ALL_CLUSTERS = auto()
    ALL_CLUSTERS_MINIMAL = auto()
    CHIP_TOOL = auto()
    CHIP_TOOL_DARWIN = auto()
    THERMOSTAT = auto()
    RPC_CONSOLE = auto()
    MIN_MDNS = auto()
    ADDRESS_RESOLVE = auto()
    TV_APP = auto()
    TV_CASTING_APP = auto()
    LIGHT = auto()
    LIGHT_DATA_MODEL_NO_UNIQUE_ID = auto()
    LOCK = auto()
    TESTS = auto()
    SHELL = auto()
    CERT_TOOL = auto()
    OTA_PROVIDER = auto()
    OTA_REQUESTOR = auto()
    SIMULATED_APP1 = auto()
    SIMULATED_APP2 = auto()
    PYTHON_BINDINGS = auto()
    EFR32_TEST_RUNNER = auto()
    TV_CASTING = auto()
    BRIDGE = auto()
    FABRIC_ADMIN = auto()
    FABRIC_BRIDGE = auto()
    FABRIC_SYNC = auto()
    JAVA_MATTER_CONTROLLER = auto()
    KOTLIN_MATTER_CONTROLLER = auto()
    CONTACT_SENSOR = auto()
    DISHWASHER = auto()
    MICROWAVE_OVEN = auto()
    REFRIGERATOR = auto()
    RVC = auto()
    AIR_PURIFIER = auto()
    LIT_ICD = auto()
    AIR_QUALITY_SENSOR = auto()
    NETWORK_MANAGER = auto()
    ENERGY_MANAGEMENT = auto()
    WATER_LEAK_DETECTOR = auto()
    TERMS_AND_CONDITIONS = auto()
    CAMERA = auto()
    CAMERA_CONTROLLER = auto()

    def ExamplePath(self):
        if self == HostApp.ALL_CLUSTERS:
            return 'all-clusters-app/linux'
        elif self == HostApp.ALL_CLUSTERS_MINIMAL:
            return 'all-clusters-minimal-app/linux'
        elif self == HostApp.CHIP_TOOL:
            return 'chip-tool'
        elif self == HostApp.CHIP_TOOL_DARWIN:
            return 'darwin-framework-tool'
        elif self == HostApp.THERMOSTAT:
            return 'thermostat/linux'
        elif self == HostApp.RPC_CONSOLE:
            return 'common/pigweed/rpc_console'
        elif self == HostApp.MIN_MDNS:
            return 'minimal-mdns'
        elif self == HostApp.TV_APP:
            return 'tv-app/linux'
        elif self == HostApp.TV_CASTING_APP:
            return 'tv-casting-app/linux'
        elif self == HostApp.LIGHT:
            return 'lighting-app/linux'
        elif self == HostApp.LIGHT_DATA_MODEL_NO_UNIQUE_ID:
            return 'lighting-app-data-mode-no-unique-id/linux'
        elif self == HostApp.LOCK:
            return 'lock-app/linux'
        elif self == HostApp.SHELL:
            return 'shell/standalone'
        elif self == HostApp.OTA_PROVIDER:
            return 'ota-provider-app/linux'
        elif self in [HostApp.SIMULATED_APP1, HostApp.SIMULATED_APP2]:
            return 'placeholder/linux/'
        elif self == HostApp.OTA_REQUESTOR:
            return 'ota-requestor-app/linux'
        elif self in [HostApp.ADDRESS_RESOLVE, HostApp.TESTS, HostApp.PYTHON_BINDINGS, HostApp.CERT_TOOL]:
            return '../'
        elif self == HostApp.EFR32_TEST_RUNNER:
            return '../src/test_driver/efr32'
        elif self == HostApp.TV_CASTING:
            return 'tv-casting-app/linux'
        elif self == HostApp.BRIDGE:
            return 'bridge-app/linux'
        elif self == HostApp.FABRIC_ADMIN:
            return 'fabric-admin'
        elif self == HostApp.FABRIC_BRIDGE:
            return 'fabric-bridge-app/linux'
        elif self == HostApp.FABRIC_SYNC:
            return 'fabric-sync'
        elif self == HostApp.JAVA_MATTER_CONTROLLER:
            return 'java-matter-controller'
        elif self == HostApp.KOTLIN_MATTER_CONTROLLER:
            return 'kotlin-matter-controller'
        elif self == HostApp.CONTACT_SENSOR:
            return 'contact-sensor-app/linux'
        elif self == HostApp.DISHWASHER:
            return 'dishwasher-app/linux'
        elif self == HostApp.MICROWAVE_OVEN:
            return 'microwave-oven-app/linux'
        elif self == HostApp.REFRIGERATOR:
            return 'refrigerator-app/linux'
        elif self == HostApp.RVC:
            return 'rvc-app/linux'
        elif self == HostApp.AIR_PURIFIER:
            return 'air-purifier-app/linux'
        elif self == HostApp.LIT_ICD:
            return 'lit-icd-app/linux'
        elif self == HostApp.AIR_QUALITY_SENSOR:
            return 'air-quality-sensor-app/linux'
        elif self == HostApp.NETWORK_MANAGER:
            return 'network-manager-app/linux'
        elif self == HostApp.ENERGY_MANAGEMENT:
            return 'energy-management-app/linux'
        elif self == HostApp.WATER_LEAK_DETECTOR:
            return 'water-leak-detector-app/linux'
        elif self == HostApp.TERMS_AND_CONDITIONS:
            return 'terms-and-conditions-app/linux'
        elif self == HostApp.CAMERA:
            return 'camera-app/linux'
        elif self == HostApp.CAMERA_CONTROLLER:
            return 'camera-controller'
        else:
            raise Exception('Unknown app type: %r' % self)

    def OutputNames(self):
        if self == HostApp.ALL_CLUSTERS:
            yield 'chip-all-clusters-app'
            yield 'chip-all-clusters-app.map'
        elif self == HostApp.ALL_CLUSTERS_MINIMAL:
            yield 'chip-all-clusters-minimal-app'
            yield 'chip-all-clusters-minimal-app.map'
        elif self == HostApp.CHIP_TOOL:
            yield 'chip-tool'
            yield 'chip-tool.map'
        elif self == HostApp.CHIP_TOOL_DARWIN:
            yield 'darwin-framework-tool'
            yield 'darwin-framework-tool.map'
        elif self == HostApp.THERMOSTAT:
            yield 'thermostat-app'
            yield 'thermostat-app.map'
        elif self == HostApp.RPC_CONSOLE:
            yield 'chip_rpc_console_wheels'
        elif self == HostApp.MIN_MDNS:
            yield 'mdns-advertiser'
            yield 'mdns-advertiser.map'
            yield 'minimal-mdns-client'
            yield 'minimal-mdns-client.map'
            yield 'minimal-mdns-server'
            yield 'minimal-mdns-server.map'
        elif self == HostApp.ADDRESS_RESOLVE:
            yield 'address-resolve-tool'
            yield 'address-resolve-tool.map'
        elif self == HostApp.TV_APP:
            yield 'chip-tv-app'
            yield 'chip-tv-app.map'
        elif self == HostApp.TV_CASTING_APP:
            yield 'chip-tv-casting-app'
            yield 'chip-tv-casting-app.map'
        elif self == HostApp.LIGHT or self == HostApp.LIGHT_DATA_MODEL_NO_UNIQUE_ID:
            yield 'chip-lighting-app'
            yield 'chip-lighting-app.map'
        elif self == HostApp.LOCK:
            yield 'chip-lock-app'
            yield 'chip-lock-app.map'
        elif self == HostApp.TESTS:
            pass
        elif self == HostApp.SHELL:
            yield 'chip-shell'
            yield 'chip-shell.map'
        elif self == HostApp.CERT_TOOL:
            yield 'chip-cert'
            yield 'chip-cert.map'
        elif self == HostApp.SIMULATED_APP1:
            yield 'chip-app1'
            yield 'chip-app1.map'
        elif self == HostApp.SIMULATED_APP2:
            yield 'chip-app2'
            yield 'chip-app2.map'
        elif self == HostApp.OTA_PROVIDER:
            yield 'chip-ota-provider-app'
            yield 'chip-ota-provider-app.map'
        elif self == HostApp.OTA_REQUESTOR:
            yield 'chip-ota-requestor-app'
            yield 'chip-ota-requestor-app.map'
        elif self == HostApp.PYTHON_BINDINGS:
            yield 'controller/python'  # Directory containing WHL files
        elif self == HostApp.EFR32_TEST_RUNNER:
            yield 'chip_pw_test_runner_wheels'
        elif self == HostApp.TV_CASTING:
            yield 'chip-tv-casting-app'
            yield 'chip-tv-casting-app.map'
        elif self == HostApp.BRIDGE:
            yield 'chip-bridge-app'
            yield 'chip-bridge-app.map'
        elif self == HostApp.FABRIC_ADMIN:
            yield 'fabric-admin'
            yield 'fabric-admin.map'
        elif self == HostApp.FABRIC_BRIDGE:
            yield 'fabric-bridge-app'
            yield 'fabric-bridge-app.map'
        elif self == HostApp.FABRIC_SYNC:
            yield 'fabric-sync'
            yield 'fabric-sync.map'
        elif self == HostApp.JAVA_MATTER_CONTROLLER:
            yield 'java-matter-controller'
            yield 'java-matter-controller.map'
        elif self == HostApp.KOTLIN_MATTER_CONTROLLER:
            yield 'kotlin-matter-controller'
            yield 'kotlin-matter-controller.map'
        elif self == HostApp.CONTACT_SENSOR:
            yield 'contact-sensor-app'
            yield 'contact-sensor-app.map'
        elif self == HostApp.DISHWASHER:
            yield 'dishwasher-app'
            yield 'dishwasher-app.map'
        elif self == HostApp.MICROWAVE_OVEN:
            yield 'chip-microwave-oven-app'
            yield 'chip-microwave-oven-app.map'
        elif self == HostApp.REFRIGERATOR:
            yield 'refrigerator-app'
            yield 'refrigerator-app.map'
        elif self == HostApp.RVC:
            yield 'chip-rvc-app'
            yield 'chip-rvc-app.map'
        elif self == HostApp.AIR_PURIFIER:
            yield 'air-purifier-app'
            yield 'air-purifier-app.map'
        elif self == HostApp.LIT_ICD:
            yield 'lit-icd-app'
            yield 'lit-icd-app.map'
        elif self == HostApp.NETWORK_MANAGER:
            yield 'matter-network-manager-app'
            yield 'matter-network-manager-app.map'
        elif self == HostApp.ENERGY_MANAGEMENT:
            yield 'chip-energy-management-app'
            yield 'chip-energy-management-app.map'
        elif self == HostApp.WATER_LEAK_DETECTOR:
            yield 'water-leak-detector-app'
            yield 'water-leak-detector-app.map'
        elif self == HostApp.TERMS_AND_CONDITIONS:
            yield 'chip-terms-and-conditions-app'
            yield 'chip-terms-and-conditions-app.map'
        elif self == HostApp.CAMERA:
            yield 'chip-camera-app'
            yield 'chip-camera-app.map'
        elif self == HostApp.CAMERA_CONTROLLER:
            yield 'chip-camera-controller'
            yield 'chip-camera-controller.map'
        else:
            raise Exception('Unknown app type: %r' % self)


class HostBoard(Enum):
    NATIVE = auto()

    # cross-compile support
    ARM64 = auto()

    # for test support
    FAKE = auto()

    def BoardName(self):
        if self == HostBoard.NATIVE:
            uname_result = uname()
            arch = uname_result.machine

            # standardize some common platforms
            if arch == 'x86_64':
                arch = 'x64'
            elif arch == 'i386' or arch == 'i686':
                arch = 'x86'
            elif arch in ('aarch64', 'aarch64_be', 'armv8b', 'armv8l'):
                arch = 'arm64'

            return arch
        elif self == HostBoard.ARM64:
            return 'arm64'
        elif self == HostBoard.FAKE:
            return 'fake'
        else:
            raise Exception('Unknown host board type: %r' % self)

    def PlatformName(self):
        if self == HostBoard.NATIVE:
            return uname().system.lower()
        elif self == HostBoard.FAKE:
            return 'fake'
        else:
            # Cross compilation assumes linux currently
            return 'linux'


class HostBuilder(GnBuilder):

    def __init__(self, root, runner, app: HostApp, board=HostBoard.NATIVE,
                 enable_ipv4=True, enable_ble=True, enable_wifi=True,
                 enable_thread=True, use_tsan=False, use_asan=False, use_ubsan=False,
                 separate_event_loop=True, fuzzing_type: HostFuzzingType = HostFuzzingType.NONE, use_clang=False,
                 interactive_mode=True, extra_tests=False, use_nl_fault_injection=False, use_platform_mdns=False, enable_rpcs=False,
                 use_coverage=False, use_dmalloc=False, minmdns_address_policy=None,
                 minmdns_high_verbosity=False, imgui_ui=False, crypto_library: HostCryptoLibrary = None,
                 enable_test_event_triggers=None,
                 enable_dnssd_tests: Optional[bool] = None,
                 chip_casting_simplified: Optional[bool] = None,
                 disable_shell=False,
                 use_googletest=False,
                 terms_and_conditions_required: Optional[bool] = None,
                 ):
        super(HostBuilder, self).__init__(
            root=os.path.join(root, 'examples', app.ExamplePath()),
            runner=runner)

        self.app = app
        self.board = board
        self.extra_gn_options = []
        self.build_env = {}

        if enable_rpcs:
            self.extra_gn_options.append('import("//with_pw_rpc.gni")')

        if not enable_ipv4:
            self.extra_gn_options.append('chip_inet_config_enable_ipv4=false')

        if not enable_ble:
            self.extra_gn_options.append('chip_config_network_layer_ble=false')
            self.extra_gn_options.append('chip_enable_ble=false')

        if not enable_wifi:
            self.extra_gn_options.append('chip_enable_wifi=false')

        if not enable_thread:
            self.extra_gn_options.append('chip_enable_openthread=false')

        if disable_shell:
            self.extra_gn_options.append('chip_build_libshell=false')

        if use_tsan:
            self.extra_gn_options.append('is_tsan=true')

        if use_asan:
            self.extra_gn_options.append('is_asan=true')

        if use_ubsan:
            self.extra_gn_options.append('is_ubsan=true')

        if use_dmalloc:
            self.extra_gn_options.append('chip_config_memory_debug_checks=true')
            self.extra_gn_options.append('chip_config_memory_debug_dmalloc=true')

            # this is from `dmalloc -b -l DMALLOC_LOG -i 1 high`
            self.build_env['DMALLOC_OPTIONS'] = 'debug=0x4f4ed03,inter=1,log=DMALLOC_LOG'

            # glib interop with dmalloc
            self.build_env['G_SLICE'] = 'always-malloc'

        if not separate_event_loop:
            self.extra_gn_options.append('config_use_separate_eventloop=false')

        if not interactive_mode:
            self.extra_gn_options.append('config_use_interactive_mode=false')

        if fuzzing_type == HostFuzzingType.LIB_FUZZER:
            self.extra_gn_options.append('is_libfuzzer=true')
        elif fuzzing_type == HostFuzzingType.OSS_FUZZ:
            self.extra_gn_options.append('oss_fuzz=true')
        elif fuzzing_type == HostFuzzingType.PW_FUZZTEST:
            self.extra_gn_options.append('pw_enable_fuzz_test_targets=true')

        if imgui_ui:
            self.extra_gn_options.append('chip_examples_enable_imgui_ui=true')

        self.use_coverage = use_coverage
        if use_coverage:
            self.extra_gn_options.append('use_coverage=true')

        self.use_clang = use_clang  # for usage in other commands
        if use_clang:
            self.extra_gn_options.append('is_clang=true')

            if self.board == HostBoard.FAKE:
                # Fake uses "//build/toolchain/fake:fake_x64_gcc"
                # so setting clang is not correct
                raise Exception('Fake host board is always gcc (not clang)')

        if use_nl_fault_injection:
            self.extra_gn_options.append('chip_with_nlfaultinjection=true')

        if minmdns_address_policy:
            if use_platform_mdns:
                raise Exception('Address policy applies to minmdns only')
            self.extra_gn_options.append('chip_minmdns_default_policy="%s"' % minmdns_address_policy)

        if use_platform_mdns:
            self.extra_gn_options.append('chip_mdns="platform"')

        if extra_tests:
            # Flag for testing purpose
            self.extra_gn_options.append(
                'chip_im_force_fabric_quota_check=true')

        if minmdns_high_verbosity:
            self.extra_gn_options.append('chip_minmdns_high_verbosity=true')

        if app == HostApp.TESTS:
            self.extra_gn_options.append('chip_build_tests=true')
            self.build_command = 'check'

        if app == HostApp.EFR32_TEST_RUNNER:
            self.build_command = 'runner'
            # board will NOT be used, but is required to be able to properly
            # include things added by the test_runner efr32 build
            self.extra_gn_options.append('silabs_board="BRD4187C"')

        # Crypto library has per-platform defaults (like openssl for linux/mac
        # and mbedtls for android/freertos/zephyr/mbed/...)
        if crypto_library:
            self.extra_gn_options.append(crypto_library.gn_argument)

        if enable_test_event_triggers is not None:
            if 'EVSE' in enable_test_event_triggers:
                self.extra_gn_options.append('chip_enable_energy_evse_trigger=true')

        if enable_dnssd_tests is not None:
            if enable_dnssd_tests:
                self.extra_gn_options.append('chip_enable_dnssd_tests=true')
            else:
                self.extra_gn_options.append('chip_enable_dnssd_tests=false')

        if chip_casting_simplified is not None:
            self.extra_gn_options.append(f'chip_casting_simplified={str(chip_casting_simplified).lower()}')

        if terms_and_conditions_required is not None:
            if terms_and_conditions_required:
                self.extra_gn_options.append('chip_terms_and_conditions_required=true')
            else:
                self.extra_gn_options.append('chip_terms_and_conditions_required=false')

        if self.board == HostBoard.ARM64:
            if not use_clang:
                raise Exception("Cross compile only supported using clang")

        if app == HostApp.CERT_TOOL:
            # Certification only built for openssl
            if self.board == HostBoard.ARM64 and crypto_library == HostCryptoLibrary.MBEDTLS:
                raise Exception("MbedTLS not supported for cross compiling cert tool")
            self.build_command = 'src/tools/chip-cert'
        elif app == HostApp.ADDRESS_RESOLVE:
            self.build_command = 'src/lib/address_resolve:address-resolve-tool'
        elif app == HostApp.PYTHON_BINDINGS:
            self.extra_gn_options.append('enable_rtti=false')
            self.extra_gn_options.append('chip_project_config_include_dirs=["//config/python"]')
            self.build_command = 'chip-repl'

        if self.app == HostApp.SIMULATED_APP1:
            self.extra_gn_options.append('chip_tests_zap_config="app1"')

        if self.app == HostApp.SIMULATED_APP2:
            self.extra_gn_options.append('chip_tests_zap_config="app2"')

        if self.app == HostApp.TESTS and fuzzing_type != HostFuzzingType.NONE:
            self.build_command = 'fuzz_tests'

        if self.app == HostApp.TESTS and fuzzing_type == HostFuzzingType.PW_FUZZTEST:
            self.build_command = 'pw_fuzz_tests'

        if self.app == HostApp.TESTS and use_googletest:
            self.extra_gn_options.append('import("//build_overrides/pigweed.gni")')
            self.extra_gn_options.append('import("//build_overrides/googletest.gni")')
            self.extra_gn_options.append('pw_unit_test_BACKEND="$dir_pw_unit_test:googletest"')
            self.extra_gn_options.append('dir_pw_third_party_googletest="$dir_googletest"')
            self.extra_gn_options.append('chip_build_tests_googletest=true')

    def GnBuildArgs(self):
        if self.board == HostBoard.NATIVE:
            return self.extra_gn_options
        elif self.board == HostBoard.ARM64:
            self.extra_gn_options.extend(
                [
                    'target_cpu="arm64"',
                    'sysroot="%s"' % self.SysRootPath('SYSROOT_AARCH64')
                ]
            )

            return self.extra_gn_options
        elif self.board == HostBoard.FAKE:
            self.extra_gn_options.extend(
                [
                    'custom_toolchain="//build/toolchain/fake:fake_x64_gcc"',
                    'chip_link_tests=true',
                    'chip_device_platform="fake"',
                    'chip_fake_platform=true',
                ]
            )
            return self.extra_gn_options
        else:
            raise Exception('Unknown host board type: %r' % self)

    def createJavaExecutable(self, java_program):
        self._Execute(
            [
                "chmod",
                "+x",
                "%s/bin/%s" % (self.output_dir, java_program),
            ],
            title="Make Java program executable",
        )

    def GnBuildEnv(self):
        if self.board == HostBoard.ARM64:
            self.build_env['PKG_CONFIG_PATH'] = os.path.join(
                self.SysRootPath('SYSROOT_AARCH64'), 'lib/aarch64-linux-gnu/pkgconfig')
        if self.app == HostApp.TESTS and self.use_coverage and self.use_clang:
            # Every test is expected to have a distinct build ID, so `%m` will be
            # distinct.
            #
            # Output is relative to "oputput_dir" since that is where GN executs
            self.build_env['LLVM_PROFILE_FILE'] = os.path.join("coverage", "profiles", "run_%b.profraw")

        return self.build_env

    def SysRootPath(self, name):
        if name not in os.environ:
            raise Exception('Missing environment variable "%s"' % name)
        return os.environ[name]

    def generate(self):
        super(HostBuilder, self).generate()
        if 'JAVA_HOME' in os.environ:
            self._Execute(
                ["third_party/java_deps/set_up_java_deps.sh"],
                title="Setting up Java deps",
            )

            exampleName = self.app.ExamplePath()
            if exampleName == "java-matter-controller":
                self._Execute(
                    [
                        "cp",
                        os.path.join(self.root, "Manifest.txt"),
                        self.output_dir,
                    ],
                    title="Copying Manifest.txt to " + self.output_dir,
                )
            if exampleName == "kotlin-matter-controller":
                self._Execute(
                    [
                        "cp",
                        os.path.join(self.root, "Manifest.txt"),
                        self.output_dir,
                    ],
                    title="Copying Manifest.txt to " + self.output_dir,
                )

        if self.app == HostApp.TESTS and self.use_coverage:
            self.coverage_dir = os.path.join(self.output_dir, 'coverage')
            self._Execute(['mkdir', '-p', self.coverage_dir], title="Create coverage output location")

    def PreBuildCommand(self):
        if self.app == HostApp.TESTS and self.use_coverage and not self.use_clang:
            cmd = ['ninja', '-C', self.output_dir]

            if self.ninja_jobs is not None:
                cmd.append('-j' + str(self.ninja_jobs))

            cmd.append('default')

            self._Execute(cmd, title="Build-only")
            self._Execute(['lcov', '--initial', '--capture', '--directory', os.path.join(self.output_dir, 'obj'),
                           '--exclude', os.path.join(self.chip_dir, '**/tests/*'),
                           '--exclude', os.path.join(self.chip_dir, 'zzz_generated/*'),
                           '--exclude', os.path.join(self.chip_dir, 'third_party/*'),
                           '--exclude', os.path.join(self.chip_dir, 'out/*'),
                           '--exclude', '/usr/include/*',
                           '--output-file', os.path.join(self.coverage_dir, 'lcov_base.info')], title="Initial coverage baseline")

    def PostBuildCommand(self):
        # TODO: CLANG coverage is not yet implemented, requires different tooling
        if self.app == HostApp.TESTS and self.use_coverage and not self.use_clang:
            self._Execute(['lcov', '--capture', '--directory', os.path.join(self.output_dir, 'obj'),
                           '--exclude', os.path.join(self.chip_dir, '**/tests/*'),
                           '--exclude', os.path.join(self.chip_dir, 'zzz_generated/*'),
                           '--exclude', os.path.join(self.chip_dir, 'third_party/*'),
                           '--exclude', os.path.join(self.chip_dir, 'out/*'),
                           '--exclude', '/usr/include/*',
                           '--output-file', os.path.join(self.coverage_dir, 'lcov_test.info')], title="Update coverage")
            self._Execute(['lcov', '--add-tracefile', os.path.join(self.coverage_dir, 'lcov_base.info'),
                           '--add-tracefile', os.path.join(self.coverage_dir, 'lcov_test.info'),
                           '--output-file', os.path.join(self.coverage_dir, 'lcov_final.info')
                           ], title="Final coverage info")
            self._Execute(['genhtml', os.path.join(self.coverage_dir, 'lcov_final.info'), '--output-directory',
                           os.path.join(self.coverage_dir, 'html')], title="HTML coverage")

        # coverage for clang works by having perfdata for every test run, which are in "*.profraw" files
        if self.app == HostApp.TESTS and self.use_coverage and self.use_clang:
            # Clang coverage config generates "coverage/{name}.profraw" for each test indivdually
            # Here we are merging ALL raw profiles into a single indexed file

            _indexed_instrumentation = shlex.quote(os.path.join(self.coverage_dir, "merged.profdata"))

            self._Execute([
                "bash",
                "-c",
                f'find {shlex.quote(self.coverage_dir)} -name "*.profraw"'
                + f' | xargs -n 10240 llvm-profdata merge -sparse -o {_indexed_instrumentation}'
            ],
                title="Generating merged coverage data")

            _lcov_data = os.path.join(self.coverage_dir, "merged.lcov")

            self._Execute([
                "bash",
                "-c",
                f'find {shlex.quote(self.coverage_dir)} -name "*.profraw"'
                + ' | xargs -n1 basename | sed "s/\\.profraw//" '
                + f' | xargs -I @ echo -object {shlex.quote(os.path.join(self.output_dir, "tests", "@"))}'
                + f' | xargs -n 10240 llvm-cov export -format=lcov --instr-profile {_indexed_instrumentation} '
                # only care about SDK code. third_party is not considered sdk
                + ' --ignore-filename-regex "/third_party/"'
                # about 75K lines with almost 0% coverage
                + ' --ignore-filename-regex "/zzz_generated/"'
                # generated interface files. about 8K lines with little coverage
                + ' --ignore-filename-regex "/out/.*/Linux/dbus/"'
                # 100% coverage for 1K lines, but not relevant (test code)
                + ' --ignore-filename-regex "/out/.*/clang_static_coverage_config/"'
                # Tests are likely 100% or close to, want to see only "functionality tested"
                + ' --ignore-filename-regex "/tests/"'
                # Ignore system includes
                + ' --ignore-filename-regex "/usr/include/"'
                + ' --ignore-filename-regex "/usr/lib/"'
                + f' | cat >{shlex.quote(_lcov_data)}'
            ],
                title="Generating lcov data")

            self._Execute([
                "genhtml",
                "--ignore-errors",
                "inconsistent",
                "--ignore-errors",
                "range",
                # "--hierarchical" <- this may be interesting
                "--output",
                os.path.join(self.output_dir, "html"),
                os.path.join(self.coverage_dir, "merged.lcov"),
            ],
                title="Generating HTML coverage report")

        if self.app == HostApp.JAVA_MATTER_CONTROLLER:
            self.createJavaExecutable("java-matter-controller")

        if self.app == HostApp.KOTLIN_MATTER_CONTROLLER:
            self.createJavaExecutable("kotlin-matter-controller")

    def build_outputs(self):
        for name in self.app.OutputNames():
            if not self.options.enable_link_map_file and name.endswith(".map"):
                continue
            path = os.path.join(self.output_dir, name)
            if os.path.isdir(path):
                for root, dirs, files in os.walk(path):
                    for file in files:
                        yield BuilderOutput(os.path.join(root, file), file)
            else:
                yield BuilderOutput(os.path.join(self.output_dir, name), name)
