# Copyright (c) 2023 Project CHIP Authors
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
from enum import Enum, auto

from .gn import GnBuilder


class RW61XApp(Enum):
    ALL_CLUSTERS = auto()
    THERMOSTAT = auto()
    LAUNDRY_WASHER = auto()

    def ExampleName(self):
        if self == RW61XApp.ALL_CLUSTERS:
            return 'all-clusters-app'
        elif self == RW61XApp.THERMOSTAT:
            return 'thermostat'
        elif self == RW61XApp.LAUNDRY_WASHER:
            return 'laundry-washer-app'
        else:
            raise Exception('Unknown app type: %r' % self)

    def NameSuffix(self):
        if self == RW61XApp.ALL_CLUSTERS:
            return '-'.join(['chip', 'rw61x', 'all-cluster-example'])
        if self == RW61XApp.THERMOSTAT:
            return '-'.join(['chip', 'rw61x', 'thermostat-example'])
        if self == RW61XApp.LAUNDRY_WASHER:
            return '-'.join(['chip', 'rw61x', 'laundry-washer-example'])
        else:
            raise Exception('Unknown app type: %r' % self)

    def BuildRoot(self, root):
        return os.path.join(root, 'examples', self.ExampleName(), 'nxp', 'rt/rw61x')


class RW61XBuilder(GnBuilder):

    def __init__(self,
                 root,
                 runner,
                 app: RW61XApp = RW61XApp.ALL_CLUSTERS,
                 release: bool = False,
                 disable_ble: bool = False,
                 enable_thread: bool = False,
                 enable_wifi: bool = False,
                 disable_ipv4: bool = False,
                 enable_shell: bool = False,
                 enable_ota: bool = False,
                 enable_factory_data: bool = False,
                 is_sdk_package: bool = True,
                 a2_board_revision: bool = False):
        super(RW61XBuilder, self).__init__(
            root=app.BuildRoot(root),
            runner=runner)
        self.app = app
        self.release = release
        self.disable_ipv4 = disable_ipv4
        self.disable_ble = disable_ble
        self.enable_thread = enable_thread
        self.enable_wifi = enable_wifi
        self.enable_ota = enable_ota
        self.enable_factory_data = enable_factory_data
        self.enable_shell = enable_shell
        self.is_sdk_package = is_sdk_package
        self.a2_board_revision = a2_board_revision

    def GnBuildArgs(self):
        args = []

        if self.release:
            args.append('is_debug=false')

        if self.enable_ota:
            args.append('chip_enable_ota_requestor=true no_mcuboot=false')

        if self.disable_ipv4:
            args.append('chip_inet_config_enable_ipv4=false')

        if self.disable_ble:
            args.append('chip_enable_ble=false')

        if self.enable_wifi:
            args.append('chip_enable_wifi=true')

        if self.enable_thread:
            args.append('chip_enable_openthread=true chip_inet_config_enable_ipv4=false')

        if self.enable_factory_data:
            args.append('chip_with_factory_data=1')

        if self.a2_board_revision:
            args.append('board_version=\"A2\"')

        if self.enable_shell:
            args.append('chip_enable_matter_cli=true')

        if self.is_sdk_package:
            args.append('is_sdk_package=true')

        return args

    def generate(self):
        super(RW61XBuilder, self).generate()

    def build_outputs(self):
        name = '%s' % self.app.NameSuffix()
        return {
            '%s.elf' % name: os.path.join(self.output_dir, name),
            '%s.map' % name: os.path.join(self.output_dir, '%s.map' % name)
        }
