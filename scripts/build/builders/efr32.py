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

import glob
import logging
import os
import shlex
import subprocess
from enum import Enum, auto

from .builder import BuilderOutput
from .gn import GnBuilder


class Efr32App(Enum):
    LIGHT = auto()
    LOCK = auto()
    SWITCH = auto()
    WINDOW_COVERING = auto()
    THERMOSTAT = auto()
    PUMP = auto()
    UNIT_TEST = auto()
    AIR_QUALITY_SENSOR = auto()

    def ExampleName(self):
        if self == Efr32App.LIGHT:
            return 'lighting-app'
        elif self == Efr32App.LOCK:
            return 'lock-app'
        elif self == Efr32App.SWITCH:
            return 'light-switch-app'
        elif self == Efr32App.WINDOW_COVERING:
            return 'window-app'
        elif self == Efr32App.THERMOSTAT:
            return 'thermostat'
        elif self == Efr32App.PUMP:
            return 'pump-app'
        elif self == Efr32App.AIR_QUALITY_SENSOR:
            return 'air-quality-sensor-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def AppNamePrefix(self):
        if self == Efr32App.LIGHT:
            return 'matter-silabs-lighting-example'
        elif self == Efr32App.LOCK:
            return 'matter-silabs-lock-example'
        elif self == Efr32App.SWITCH:
            return 'matter-silabs-light-switch-example'
        elif self == Efr32App.WINDOW_COVERING:
            return 'matter-silabs-window-example'
        elif self == Efr32App.THERMOSTAT:
            return 'matter-silabs-thermostat-example'
        elif self == Efr32App.PUMP:
            return 'matter-silabs-pump-example'
        elif self == Efr32App.UNIT_TEST:
            return 'matter-silabs-device_tests'
        elif self == Efr32App.AIR_QUALITY_SENSOR:
            return 'matter-silabs-air-quality-sensor-example'
        else:
            raise Exception('Unknown app type: %r' % self)

    def FlashBundleName(self):
        if self == Efr32App.LIGHT:
            return 'lighting_app.flashbundle.txt'
        elif self == Efr32App.LOCK:
            return 'lock_app.flashbundle.txt'
        elif self == Efr32App.SWITCH:
            return 'light_switch_app.flashbundle.txt'
        elif self == Efr32App.WINDOW_COVERING:
            return 'window_app.flashbundle.txt'
        elif self == Efr32App.THERMOSTAT:
            return 'thermostat_app.flashbundle.txt'
        elif self == Efr32App.PUMP:
            return 'pump_app.flashbundle.txt'
        elif self == Efr32App.UNIT_TEST:
            return os.path.join('tests', 'efr32_device_tests.flashbundle.txt')
        elif self == Efr32App.AIR_QUALITY_SENSOR:
            return 'air_quality_sensor_app.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        if self == Efr32App.UNIT_TEST:
            return os.path.join(root, 'src', 'test_driver', 'efr32')
        else:
            return os.path.join(root, 'examples', self.ExampleName(), 'silabs')


class Efr32Board(Enum):
    BRD2704B = 1
    BRD4316A = 2
    BRD4317A = 3
    BRD4318A = 4
    BRD4319A = 5
    BRD4186A = 6
    BRD4187A = 7
    BRD2601B = 8
    BRD4187C = 9
    BRD4186C = 10
    BRD4338A = 11
    BRD2703A = 12
    BRD2605A = 13
    BRD4343A = 14
    BRD4342A = 15

    def GnArgName(self):
        if self == Efr32Board.BRD2704B:
            return 'BRD2704B'
        elif self == Efr32Board.BRD4316A:
            return 'BRD4316A'
        elif self == Efr32Board.BRD4317A:
            return 'BRD4317A'
        elif self == Efr32Board.BRD4318A:
            return 'BRD4318A'
        elif self == Efr32Board.BRD4319A:
            return 'BRD4319A'
        elif self == Efr32Board.BRD4186A:
            return 'BRD4186A'
        elif self == Efr32Board.BRD4187A:
            return 'BRD4187A'
        elif self == Efr32Board.BRD2601B:
            return 'BRD2601B'
        elif self == Efr32Board.BRD4186C:
            return 'BRD4186C'
        elif self == Efr32Board.BRD4187C:
            return 'BRD4187C'
        elif self == Efr32Board.BRD4338A:
            return 'BRD4338A'
        elif self == Efr32Board.BRD2703A:
            return 'BRD2703A'
        elif self == Efr32Board.BRD2605A:
            return 'BRD2605A'
        elif self == Efr32Board.BRD4343A:
            return 'BRD4343A'
        elif self == Efr32Board.BRD4342A:
            return 'BRD4342A'
        else:
            raise Exception('Unknown board #: %r' % self)


class Efr32Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: Efr32App = Efr32App.LIGHT,
                 board: Efr32Board = Efr32Board.BRD4187C,
                 chip_build_libshell: bool = False,
                 chip_logging: bool = True,
                 chip_openthread_ftd: bool = True,
                 enable_heap_monitoring: bool = False,
                 enable_openthread_cli: bool = True,
                 show_qr_code: bool = False,
                 enable_rpcs: bool = False,
                 enable_ota_requestor: bool = False,
                 enable_icd: bool = False,
                 enable_low_power: bool = False,
                 enable_wifi: bool = False,
                 enable_rs9116: bool = False,
                 enable_wf200: bool = False,
                 enable_917_ncp: bool = False,
                 enable_wifi_ipv4: bool = False,
                 enable_additional_data_advertising: bool = False,
                 enable_ot_lib: bool = False,
                 enable_ot_coap_lib: bool = False,
                 no_version: bool = False,
                 enable_917_soc: bool = False,
                 use_rps_extension: bool = True
                 ):
        super(Efr32Builder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app
        self.extra_gn_options = ['silabs_board="%s"' % board.GnArgName()]
        self.dotfile = ''

        if enable_rpcs:
            self.extra_gn_options.append('is_debug=false import("//with_pw_rpc.gni")')

        if enable_ota_requestor:
            self.extra_gn_options.append('chip_enable_ota_requestor=true')

        if enable_icd:
            self.extra_gn_options.append('chip_enable_icd_server=true chip_openthread_ftd=false')

        if enable_low_power:
            self.extra_gn_options.append(
                'chip_build_libshell=false enable_openthread_cli=false show_qr_code=false disable_lcd=true')

        if chip_build_libshell:
            self.extra_gn_options.append('chip_build_libshell=true')

        if chip_logging is False:
            self.extra_gn_options.append('chip_logging=false')

        if chip_openthread_ftd is False:
            self.extra_gn_options.append('chip_openthread_ftd=false')

        if enable_heap_monitoring:
            self.extra_gn_options.append('enable_heap_monitoring=true')

        if enable_openthread_cli is False:
            self.extra_gn_options.append('enable_openthread_cli=false')

        if show_qr_code:
            self.extra_gn_options.append('show_qr_code=true')

        if enable_wifi:
            self.dotfile += self.root + '/build_for_wifi_gnfile.gn'
            if enable_917_soc:
                # Wifi SoC platform
                self.extra_gn_options.append('chip_device_platform=\"SiWx917\"')
            else:
                if enable_rs9116:
                    self.extra_gn_options.append('use_rs9116=true chip_device_platform =\"efr32\"')
                elif enable_wf200:
                    self.extra_gn_options.append('use_wf200=true chip_device_platform =\"efr32\"')
                elif enable_917_ncp:
                    self.extra_gn_options.append('use_SiWx917=true chip_device_platform =\"efr32\"')
                else:
                    raise Exception('Wifi usage: ...-wifi-[rs9116|wf200|siwx917]-...')

        if enable_wifi_ipv4:
            self.extra_gn_options.append('chip_enable_wifi_ipv4=true')

        if enable_additional_data_advertising:
            self.extra_gn_options.append('chip_enable_additional_data_advertising=true chip_enable_rotating_device_id=true')

        if enable_ot_lib:
            self.extra_gn_options.append(
                'use_silabs_thread_lib=true chip_openthread_target="../silabs:ot-efr32-cert" openthread_external_platform=""')

        if enable_ot_coap_lib:
            self.extra_gn_options.append(
                'use_silabs_thread_lib=true chip_openthread_target="../silabs:ot-efr32-cert" '
                'use_thread_coap_lib=true openthread_external_platform=""')

        if not no_version:
            shortCommitSha = subprocess.check_output(
                ['git', 'describe', '--always', '--dirty', '--exclude', '*']).decode('ascii').strip()
            branchName = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).decode('ascii').strip()
            self.extra_gn_options.append(
                'sl_matter_version_str="v1.3-%s-%s"' % (branchName, shortCommitSha))

        if use_rps_extension is False:
            self.extra_gn_options.append('use_rps_extension=false')

        if "GSDK_ROOT" in os.environ:
            # EFR32 SDK is very large. If the SDK path is already known (the
            # case for pre-installed images), use it directly.
            sdk_path = shlex.quote(os.environ['GSDK_ROOT'])
            self.extra_gn_options.append(f"efr32_sdk_root=\"{sdk_path}\"")

        if "GSDK_ROOT" in os.environ and not enable_wifi:
            self.extra_gn_options.append(f"openthread_root=\"{sdk_path}/util/third_party/openthread\"")

        if "WISECONNECT_SDK_ROOT" in os.environ:
            wiseconnect_sdk_path = shlex.quote(os.environ['WISECONNECT_SDK_ROOT'])
            self.extra_gn_options.append(f"wiseconnect_sdk_root=\"{wiseconnect_sdk_path}\"")

        if "WIFI_SDK_ROOT" in os.environ:
            wifi_sdk_path = shlex.quote(os.environ['WIFI_SDK_ROOT'])
            self.extra_gn_options.append(f"wifi_sdk_root=\"{wifi_sdk_path}\"")

    def GnBuildArgs(self):
        return self.extra_gn_options

    def _bundle(self):
        # Only unit-test needs to generate the flashbundle here.  All other examples will generate a flashbundle via the silabs_executable template.
        if self.app == Efr32App.UNIT_TEST:
            flash_bundle_path = os.path.join(self.output_dir, self.app.FlashBundleName())
            logging.info(f'Generating flashbundle {flash_bundle_path}')

            patterns = [
                os.path.join(self.output_dir, "tests", "*.flash.py"),
                os.path.join(self.output_dir, "tests", "*.s37"),
                os.path.join(self.output_dir, "tests", "silabs_firmware_utils.py"),
                os.path.join(self.output_dir, "tests", "firmware_utils.py"),
            ]

            # Generate the list of files by globbing each pattern.
            files = []
            for pattern in patterns:
                files.extend([os.path.basename(x) for x in glob.glob(pattern)])

            # Create the bundle file.
            with open(flash_bundle_path, 'w') as bundle_file:
                bundle_file.write("\n".join(files))

    def build_outputs(self):
        extensions = ["out", "hex"]
        if self.options.enable_link_map_file:
            extensions.append("out.map")

        if self.app == Efr32App.UNIT_TEST:
            # Efr32 unit-test generates the "tests" subdir with a set of files for each individual unit test source.
            for ext in extensions:
                pattern = os.path.join(self.output_dir, "tests", f"*.{ext}")
                for name in [os.path.basename(x) for x in glob.glob(pattern)]:
                    yield BuilderOutput(os.path.join(self.output_dir, "tests", name), name)
        else:
            # All other examples have just one set of files.
            for ext in extensions:
                name = f"{self.app.AppNamePrefix()}.{ext}"
                yield BuilderOutput(os.path.join(self.output_dir, name), name)

        if self.app == Efr32App.UNIT_TEST:
            # Include test runner python wheels
            for root, dirs, files in os.walk(os.path.join(self.output_dir, 'chip_pw_test_runner_wheels')):
                for file in files:
                    yield BuilderOutput(
                        os.path.join(root, file),
                        os.path.join("chip_pw_test_runner_wheels", file))

    def bundle_outputs(self):
        # If flashbundle creation is enabled, the outputs will include the s37 and flash.py files, plus the two firmware utils scripts that support flash.py.
        # For the unit-test example, there will be a s37 and flash.py file for each unit test source.
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for name in filter(None, [x.strip() for x in f.readlines()]):
                if self.app == Efr32App.UNIT_TEST:
                    sourcepath = os.path.join(self.output_dir, "tests", name)  # Unit tests are in the "tests" subdir.
                else:
                    sourcepath = os.path.join(self.output_dir, name)
                yield BuilderOutput(
                    sourcepath,
                    os.path.join("flashbundle", name))

    def generate(self):
        cmd = [
            'gn', 'gen', '--check', '--fail-on-unused-args',
            '--add-export-compile-commands=*',
            '--root=%s' % self.root
        ]
        if self.dotfile:
            cmd += ['--dotfile=%s' % self.dotfile]

        extra_args = self.GnBuildArgs()

        if self.options.pw_command_launcher:
            extra_args.append('pw_command_launcher="%s"' % self.options.pw_command_launcher)

        if self.options.enable_link_map_file:
            extra_args.append('chip_generate_link_map_file=true')

        if self.options.pregen_dir:
            extra_args.append('chip_code_pre_generated_directory="%s"' % self.options.pregen_dir)

        if extra_args:
            cmd += ['--args=%s' % ' '.join(extra_args)]

        cmd += [self.output_dir]

        title = 'Generating ' + self.identifier
        extra_env = self.GnBuildEnv()

        if extra_env:
            # convert the command into a bash command that includes
            # setting environment variables
            cmd = [
                'bash', '-c', '\n' + ' '.join(
                    ['%s="%s" \\\n' % (key, value) for key, value in extra_env.items()] +
                    [shlex.join(cmd)]
                )
            ]

        self._Execute(cmd, title=title)
