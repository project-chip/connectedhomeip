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

from datetime import datetime

from .avh_instance import AvhInstance

APPLICATION_BINARY = "chip-tool"


class AvhChiptoolInstance(AvhInstance):
    def __init__(self, avh_client, name, application_binary_path):
        super().__init__(avh_client, name)

        self.application_binary_path = application_binary_path

    def upload_application_binary(self):
        super().upload_application_binary(
            self.application_binary_path, APPLICATION_BINARY
        )

    def configure_system(self):
        self.log_in_to_console()

        # set current date and time
        self.console_exec_command("sudo timedatectl set-ntp false", timeout_s=300)
        self.console_exec_command(
            f"sudo timedatectl set-time '{datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')}'",
            timeout_s=300,
        )
        self.console_exec_command("sudo timedatectl set-ntp true", timeout_s=300)

        # install network manager
        self.console_exec_command("sudo apt-get update", timeout_s=300)
        self.console_exec_command(
            "sudo apt-get -y install network-manager", timeout_s=300
        )

        # connect Wi-Fi to the Arm ssid
        self.console_exec_command("sudo nmcli r wifi on")
        self.console_exec_command("sudo nmcli d wifi connect Arm password password")

        # disable eth0
        self.console_exec_command("sudo nmcli dev set eth0 managed no")
        self.console_exec_command("sudo ip link set dev eth0 down")

    def pairing_ble_wifi(self, node_id, ssid, password, pin_code, discriminator):
        output = self.console_exec_command(
            f"./{APPLICATION_BINARY} pairing ble-wifi {node_id} {ssid} {password} {pin_code} {discriminator}",
            timeout_s=120.0,
        )

        return output

    def on(self, node_id):
        output = self.console_exec_command(
            f"./{APPLICATION_BINARY} onoff on {node_id} 1", timeout_s=30.0
        )

        return output

    def off(self, node_id):
        output = self.console_exec_command(
            f"./{APPLICATION_BINARY} onoff off {node_id} 1", timeout_s=30.0
        )

        return output
