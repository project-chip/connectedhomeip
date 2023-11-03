#
#    Copyright (c) 2023 Project CHIP Authors
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

import os
import platform as host_platform
import sys

import psutil

from utils import log
from utils.log import border_print
from utils.shell import Bash

logger = log.get_logger(__file__)

def is_mac():
    p = host_platform.platform().lower()
    return "darwin" in p or "mac" in p


def get_ll_interface():
    # TODO: Makes too many assumptions
    if is_mac():
        return "en0"
    net_interface_path = "/sys/class/net/"
    available_net_interfaces = os.listdir(net_interface_path) \
        if os.path.exists(net_interface_path) \
        else []
    for interface in available_net_interfaces:
        if "wl" in interface:
            return interface


def get_available_interfaces():
    net_interface_path = "/sys/class/net/"
    available_net_interfaces = os.listdir(net_interface_path) \
        if os.path.exists(net_interface_path) \
        else []
    available_net_interfaces.append("any")
    return available_net_interfaces


def command_is_available(cmd_name) -> bool:
    cmd = Bash(f"which {cmd_name}", sync=True, capture_output=True)
    cmd.start_command()
    return cmd.finished_success()


def verify_host_dependencies(deps: [str]) -> None:
    missing_deps = []
    for dep in deps:
        logger.info(f"Verifying host dependency {dep}")
        if not command_is_available(dep):
            missing_deps.append(dep)
    if len(missing_deps) > 0:
        for missing_dep in missing_deps:
            border_print(f"Missing dependency, please install {missing_dep}!", important=True)
        kill_idt_children()
        sys.exit(1)


def kill_idt_children() -> None:
    for child_proc in psutil.Process(os.getpid()).children(recursive=True):
        Bash("").stop_single_proc(child_proc)
