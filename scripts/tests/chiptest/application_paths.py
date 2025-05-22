#
#    Copyright (c) 2025 Project CHIP Authors
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

import os
import typing
from dataclasses import dataclass


@dataclass
class ApplicationPaths:
    chip_tool: typing.List[str]
    all_clusters_app: typing.List[str]
    lock_app: typing.List[str]
    fabric_bridge_app: typing.List[str]
    ota_provider_app: typing.List[str]
    ota_requestor_app: typing.List[str]
    tv_app: typing.List[str]
    bridge_app: typing.List[str]
    lit_icd_app: typing.List[str]
    microwave_oven_app: typing.List[str]
    chip_repl_yaml_tester_cmd: typing.List[str]
    chip_tool_with_python_cmd: typing.List[str]
    rvc_app: typing.List[str]
    network_manager_app: typing.List[str]

    def items(self):
        return [self.chip_tool, self.all_clusters_app, self.lock_app,
                self.fabric_bridge_app, self.ota_provider_app, self.ota_requestor_app,
                self.tv_app, self.bridge_app, self.lit_icd_app,
                self.microwave_oven_app, self.chip_repl_yaml_tester_cmd,
                self.chip_tool_with_python_cmd, self.rvc_app, self.network_manager_app]

    def items_with_key(self):
        """
        Returns all path items and also the corresponding "Application Key" which
        is the typical application name.

        This is to provide scripts a consistent way to reference a path, even if
        the paths used for individual appplications contain different names
        (e.g. they could be wrapper scripts).
        """
        return [
            (self.chip_tool, "chip-tool"),
            (self.all_clusters_app, "chip-all-clusters-app"),
            (self.lock_app, "chip-lock-app"),
            (self.fabric_bridge_app, "fabric-bridge-app"),
            (self.ota_provider_app, "chip-ota-provider-app"),
            (self.ota_requestor_app, "chip-ota-requestor-app"),
            (self.tv_app, "chip-tv-app"),
            (self.bridge_app, "chip-bridge-app"),
            (self.lit_icd_app, "lit-icd-app"),
            (self.microwave_oven_app, "chip-microwave-oven-app"),
            (self.chip_repl_yaml_tester_cmd, "yamltest_with_chip_repl_tester.py"),
            (
                # This path varies, however it is a fixed python tool so it may be ok
                self.chip_tool_with_python_cmd,
                os.path.basename(self.chip_tool_with_python_cmd[-1]),
            ),
            (self.rvc_app, "chip-rvc-app"),
            (self.network_manager_app, "matter-network-manager-app"),
        ]
