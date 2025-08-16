#!/usr/bin/env python3

#
#    Copyright (c) 2024 Project CHIP Authors
#    All rights reserved.
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
#

# This is used to restart the remote device in cirque test

import logging
import os
import sys
import threading
import time

try:
    import paramiko
except ImportError:
    pass

CHIP_REPO = os.path.join(os.path.abspath(
    os.path.dirname(__file__)), "..", "..", "..", "..", "..")

logger = logging.getLogger("CirqueRestartRemoteDevice")
logger.setLevel(logging.INFO)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
sh.setStream(sys.stdout)
logger.addHandler(sh)


class restartRemoteDevice(threading.Thread):
    def __init__(self, remote_ip: str, ssh_port: int, user: str, password: str, remote_server_app: str,
                 extra_args: str):
        super(restartRemoteDevice, self).__init__()
        self.remote_ip = remote_ip
        self.ssh_port = ssh_port
        self.user = user
        self.password = password
        self.remote_server_app = remote_server_app
        self.extra_args = extra_args

    def run(self):
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        try:
            client.connect(self.remote_ip, self.ssh_port, self.user, self.password)
            client.exec_command(
                ("kill \"$(ps aux | grep -E \'out/debug/{}\' | grep -v grep | grep -v gdb | "
                 "awk \'{{print $2}}\')\"").format(self.remote_server_app))
            time.sleep(1)
            stdin, stdout, stderr = client.exec_command(
                ("ps aux | grep -E \'out/debug/standalone/{}\' | grep -v grep | grep -v gdb | "
                 "awk \'{{print $2}}\'").format(self.remote_server_app))
            if not stdout.read().decode().strip():
                logger.info(f"Succeed to kill remote process {self.remote_server_app}")
            else:
                logger.error(f"Failed to kill remote process {self.remote_server_app}")

            restart_remote_device_command = (
                "CHIPCirqueDaemon.py -- run gdb -batch -return-child-result -q -ex \"set pagination off\" "
                "-ex run -ex \"thread apply all bt\" --args {} {}").format(
                    os.path.join(CHIP_REPO, "out/debug", self.remote_server_app), self.extra_args)
            client.exec_command(restart_remote_device_command)

        finally:
            client.close()
