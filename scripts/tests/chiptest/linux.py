#
#    Copyright (c) 2026 Project CHIP Authors
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

"""
Handles linux-specific functionality for running test cases
"""

import contextlib
import logging
import os
import shlex
import shutil
import subprocess
from typing import BinaryIO

from chiptest.concurrency.worker import WorkerProcess
from chiptest.runner import Executor, LogPipe, SubprocessInfo
from chiptest.test_definition import TEST_THREAD_DATASET
from python_path import PythonPath

log = logging.getLogger(__name__)

root_dir = os.path.dirname(
    os.path.dirname(
        os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

with PythonPath(os.path.join(root_dir, 'src/python_testing/matter_testing_infrastructure'), relative_to=__file__):
    from matter.testing.commissioning_types import CommissioningMethod
    from matter.testing.linux import (BluetoothMock, DBusTestSystemBus, IsolatedNetworkNamespace, ThreadBorderRouter,
                                      WpaSupplicantMock, ensure_namespace_availability, ensure_private_state)

__all__ = [
    "ensure_namespace_availability",
    "ensure_private_state",
    "BluetoothMock",
    "DBusTestSystemBus",
    "IsolatedNetworkNamespace",
    "LinuxNamespacedExecutor",
    "ThreadBorderRouter",
    "WpaSupplicantMock"
]


class LinuxNamespacedExecutor(Executor):
    def __init__(self, ns: IsolatedNetworkNamespace):
        super().__init__()
        self.ns = ns

    def run(self, subproc: SubprocessInfo, stdin: BinaryIO | None = None, stdout: BinaryIO | LogPipe | None = None,
            stderr: BinaryIO | LogPipe | None = None):
        try:
            subprocess_ns = self.ns.netns_for_subprocess_kind(subproc.kind)
            wrapped = subproc.wrap_with(*subprocess_ns.netns_cmd_wrapper)
        except ValueError as e:
            log.warning("%s", e)
            wrapped = subproc
        return super().run(wrapped, stdin=stdin, stdout=stdout, stderr=stderr)


class LinuxWorkerProcess(WorkerProcess):
    """Linux implementation of the worker process."""

    def _platform_init(self, exit_stack: contextlib.ExitStack) -> LinuxNamespacedExecutor:
        log.debug("Initializing Linux test executor")

        # Create a virtual /tmp.
        tmp_dir_default = self._config.tmp_dir_default
        tmp_dir = self._config.tmp_dir_worker_base / str(self._config.id)

        if self._config.tmp_dir_clear:
            shutil.rmtree(tmp_dir)

        tmp_dir.mkdir(parents=True, exist_ok=True)

        # Check if the directory is empty.
        with contextlib.suppress(StopIteration):
            next(tmp_dir.iterdir())
            log.warning("Worker virtual %s directory (%s) is not empty. The worker will reuse the state from the last run",
                        tmp_dir_default, tmp_dir)

        log.info("Remounting %s as %s for the worker", tmp_dir, tmp_dir_default)
        if subprocess.run(["mount", "-o", "bind", str(tmp_dir), str(tmp_dir_default)]).returncode != 0:
            raise RuntimeError(f"Failed to mount a virtual {tmp_dir_default}")

        commissioning_method = self._config.commissioning_method

        self.net_ns: IsolatedNetworkNamespace = exit_stack.enter_context(IsolatedNetworkNamespace(
            index=self._config.id,
            # Do not bring up the app interface link automatically when doing BLE-WiFi commissioning.
            app_link_up=not commissioning_method.wifi_required,
            add_ula=not commissioning_method.thread_required,
            # Change the app link name so the interface will be recognized as WiFi or Ethernet depending on the commissioning
            # method used.
            app_link_name=commissioning_method.app_link_name, tool_link_name=commissioning_method.tool_link_name))
        self.mgmt_ns_wrapper = shlex.join(self.net_ns.mgmt_ns.netns_cmd_wrapper)

        match commissioning_method:
            case CommissioningMethod.BLE_WIFI:
                exit_stack.enter_context(DBusTestSystemBus())
                exit_stack.enter_context(BluetoothMock())
                exit_stack.enter_context(WpaSupplicantMock([commissioning_method.app_link_name],
                                                           "MatterAP", "MatterAPPassword", self.net_ns))
            case CommissioningMethod.BLE_THREAD:
                exit_stack.enter_context(DBusTestSystemBus())
                exit_stack.enter_context(BluetoothMock())
                exit_stack.enter_context(ThreadBorderRouter(TEST_THREAD_DATASET, self.net_ns))
            case CommissioningMethod.THREAD_MESHCOP:
                exit_stack.enter_context(tbr := ThreadBorderRouter(TEST_THREAD_DATASET, self.net_ns))
                self.thread_ba_host = tbr.get_border_agent_host()
                self.thread_ba_port = tbr.get_border_agent_port()
            case CommissioningMethod.WIFIPAF_WIFI:
                exit_stack.enter_context(DBusTestSystemBus())
                exit_stack.enter_context(WpaSupplicantMock(
                    [commissioning_method.app_link_name, commissioning_method.tool_link_name],
                    "MatterAP", "MatterAPPassword", self.net_ns))

        return exit_stack.enter_context(LinuxNamespacedExecutor(self.net_ns))
