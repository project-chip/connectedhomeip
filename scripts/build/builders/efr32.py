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
import subprocess
from enum import Enum, auto

from .gn import GnBuilder


class Efr32App(Enum):
    LIGHT = auto()
    LOCK = auto()
    SWITCH = auto()
    WINDOW_COVERING = auto()
    THERMOSTAT = auto()
    PUMP = auto()
    UNIT_TEST = auto()

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
            return 'efr32_device_tests.flashbundle.txt'
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        if self == Efr32App.UNIT_TEST:
            return os.path.join(root, 'src', 'test_driver', 'efr32')
        else:
            return os.path.join(root, 'examples', self.ExampleName(), 'silabs')


class Efr32Board(Enum):
    BRD4161A = 1
    BRD4163A = 2
    BRD4164A = 3
    BRD4166A = 4
    BRD4170A = 5
    BRD4186A = 6
    BRD4187A = 7
    BRD4304A = 8
    BRD4187C = 9
    BRD4186C = 10
    BRD4338A = 11

    def GnArgName(self):
        if self == Efr32Board.BRD4161A:
            return 'BRD4161A'
        elif self == Efr32Board.BRD4163A:
            return 'BRD4163A'
        elif self == Efr32Board.BRD4164A:
            return 'BRD4164A'
        elif self == Efr32Board.BRD4166A:
            return 'BRD4166A'
        elif self == Efr32Board.BRD4170A:
            return 'BRD4170A'
        elif self == Efr32Board.BRD4186A:
            return 'BRD4186A'
        elif self == Efr32Board.BRD4187A:
            return 'BRD4187A'
        elif self == Efr32Board.BRD4304A:
            return 'BRD4304A'
        elif self == Efr32Board.BRD4186C:
            return 'BRD4186C'
        elif self == Efr32Board.BRD4187C:
            return 'BRD4187C'
        elif self == Efr32Board.BRD4338A:
            return 'BRD4338A'
        else:
            raise Exception('Unknown board #: %r' % self)


class Efr32Builder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: Efr32App = Efr32App.LIGHT,
                 board: Efr32Board = Efr32Board.BRD4161A,
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
                 enable_rs911x: bool = False,
                 enable_wf200: bool = False,
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
            if board == Efr32Board.BRD4161A:
                self.extra_gn_options.append('is_debug=false chip_logging=false')
            else:
                self.extra_gn_options.append('disable_lcd=true use_external_flash=false')

            if enable_rs911x:
                self.extra_gn_options.append('use_rs911x=true')
            elif enable_wf200:
                self.extra_gn_options.append('use_wf200=true')
            elif enable_917_soc:
                self.extra_gn_options.append('chip_device_platform=\"SiWx917\"')
            else:
                raise Exception('Wifi usage: ...-wifi-[rs911x|wf200]-...')

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
                'sl_matter_version_str="v1.2-%s-%s"' % (branchName, shortCommitSha))
        if enable_917_soc:
            if use_rps_extension is False:
                self.extra_gn_options.append('use_rps_extension=false')

        if "GSDK_ROOT" in os.environ:
            # EFR32 SDK is very large. If the SDK path is already known (the
            # case for pre-installed images), use it directly.
            sdk_path = shlex.quote(os.environ['GSDK_ROOT'])
            self.extra_gn_options.append(f"efr32_sdk_root=\"{sdk_path}\"")

        if "GSDK_ROOT" in os.environ and not enable_wifi:
            self.extra_gn_options.append(f"openthread_root=\"{sdk_path}/util/third_party/openthread\"")

        if "WISECONNECT_SDK_ROOT" in os.environ and enable_rs911x:
            wiseconnect_sdk_path = shlex.quote(os.environ['WISECONNECT_SDK_ROOT'])
            self.extra_gn_options.append(f"wiseconnect_sdk_root=\"{wiseconnect_sdk_path}\"")

        if "WIFI_SDK_ROOT" in os.environ and enable_917_soc:
            wifi_sdk_path = shlex.quote(os.environ['WIFI_SDK_ROOT'])
            self.extra_gn_options.append(f"wifi_sdk_root=\"{wifi_sdk_path}\"")

    def GnBuildArgs(self):
        return self.extra_gn_options

    def build_outputs(self):
        items = {}
        for extension in ["out", "out.map", "hex"]:
            name = '%s.%s' % (self.app.AppNamePrefix(), extension)
            items[name] = os.path.join(self.output_dir, name)

        if self.app == Efr32App.UNIT_TEST:
            # Include test runner python wheels
            for root, dirs, files in os.walk(os.path.join(self.output_dir, 'chip_nl_test_runner_wheels')):
                for file in files:
                    items["chip_nl_test_runner_wheels/" +
                          file] = os.path.join(root, file)

        # Figure out flash bundle files and build accordingly
        with open(os.path.join(self.output_dir, self.app.FlashBundleName())) as f:
            for line in f.readlines():
                name = line.strip()
                items['flashbundle/%s' %
                      name] = os.path.join(self.output_dir, name)

        return items

    def generate(self):
        cmd = [
            'gn', 'gen', '--check', '--fail-on-unused-args',
            '--export-compile-commands',
            '--root=%s' % self.root
        ]
        if self.dotfile:
            cmd += ['--dotfile=%s' % self.dotfile]

        extra_args = self.GnBuildArgs()

        if self.options.pw_command_launcher:
            extra_args.append('pw_command_launcher="%s"' % self.options.pw_command_launcher)

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
