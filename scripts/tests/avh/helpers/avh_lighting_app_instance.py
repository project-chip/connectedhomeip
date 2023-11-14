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
import time

import websocket

from .avh_instance import AvhInstance

APPLICATION_BINARY = "chip-lighting-app"


class AvhLightingAppInstance(AvhInstance):
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

        # remove the Wi-Fi configuration and disable network manager on the Wi-Fi interface
        self.console_exec_command("sudo nmcli connection delete Arm")
        self.console_exec_command("sudo nmcli dev set wlan0 managed no")

        # set wlan0 ipv6 to have generated address based on EUI64
        self.console_exec_command("sudo sysctl net.ipv6.conf.wlan0.addr_gen_mode=0")

        # patch and restart wpa_supplication DBus
        self.console_exec_command(
            'sudo sed -i "s/wpa_supplicant -u -s -O/wpa_supplicant -u -s -i wlan0 -O/i" /etc/systemd/system/dbus-fi.w1.wpa_supplicant1.service'
        )
        self.console_exec_command("sudo systemctl restart wpa_supplicant.service")
        self.console_exec_command("sudo systemctl daemon-reload")

        # disable eth0
        self.console_exec_command("sudo nmcli dev set eth0 managed no")
        self.console_exec_command("sudo ip link set dev eth0 down")

    def start_application(self):
        self.console.send(f"./{APPLICATION_BINARY} --wifi\n")

    def stop_application(self):
        self.console.send("\03")  # CTRL-C
        super().wait_for_console_prompt()

    def get_application_output(self, timeout_s=5.0):
        self.console.settimeout(1.0)

        start_time = time.monotonic()
        output = b""

        while True:
            if (time.monotonic() - start_time) > timeout_s:
                break

            try:
                output += self.console.recv()
            except websocket.WebSocketTimeoutException:
                pass

        return output
