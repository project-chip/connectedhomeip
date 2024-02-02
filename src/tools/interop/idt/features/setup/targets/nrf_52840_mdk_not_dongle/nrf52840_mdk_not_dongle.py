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

import inspect
import os
import shutil

from features.capture.thread import sniff
from features.setup.targets.base import Setup
from utils import log
from utils.artifact import safe_mkdir
from utils.error import log_error, write_error_report
from utils.host import current_platform
from utils.shell import Bash

_LOGGER = log.get_logger(__file__)


class Nrf52840MdkNotDongle(Setup):

    def __init__(self, logger=_LOGGER) -> None:
        self.build_dir = os.path.join(os.path.dirname(inspect.getfile(sniff)), "BUILD", "sniff")
        self.build_log = os.path.join(self.build_dir, "BUILD_LOG.txt")
        self.error_context = "setup_capture_thread_sniff_Nrf52840MdkNotDongle"
        self.logger = logger

    def _create_command(self, command: str, join_path_for_script=False, cwd=None) -> Bash:
        if not cwd:
            cwd = self.build_dir
        if join_path_for_script:
            command = os.path.join(os.path.dirname(__file__), command)
        return Bash(f"{command} 2>&1 | tee -a {self.build_log}", sync=True, cwd=cwd)

    def _log_error(self, msg: str) -> None:
        log_error(self.error_context, msg, stack_trace=False)

    def _write_error_report(self) -> None:
        write_error_report(self.build_dir)

    def _install_flash_utils(self) -> bool:
        self.logger.info("Fetching NCP image!")
        fetch_command = self._create_command("git clone https://github.com/makerdiary/nrf52840-mdk.git")
        fetch_command.start_command()
        if not fetch_command.finished_success():
            self._log_error("otncp image fetch failed!")
            shutil.rmtree(self.build_dir)
            return False
        self.logger.info("Installing pyocd")
        install_pyocd_command = self._create_command("linux_install_pyocd.sh", join_path_for_script=True)
        install_pyocd_command.start_command()
        if not install_pyocd_command.finished_success():  # TODO: Not all errors propagate from commands here
            self._log_error("Failed to install pyocd")
            shutil.rmtree(self.build_dir)
            return False
        return True

    def _install_control_utils(self) -> bool:
        if current_platform.command_is_available("wpantund"):
            self.logger.info("wpantund already installed; skipping!")
        else:
            self.logger.info("Installing wpantund")
            wpantund_command = self._create_command("linux_install_wpantund.sh", join_path_for_script=True)
            wpantund_command.start_command()
            if not wpantund_command.finished_success():
                self._log_error("wpantund install failed!")
                shutil.rmtree(self.build_dir)
                return False
        self.logger.info("Installing pyspinel")
        install_pyspinel_command = self._create_command("pip install pyserial ipaddress && git clone "
                                                        "https://github.com/openthread/pyspinel && cd pyspinel && sudo "
                                                        "python3 setup.py install")
        install_pyspinel_command.start_command()
        if not install_pyspinel_command.finished_success():
            self._log_error("PySpinel install failed")
            shutil.rmtree(self.build_dir)
            return False
        return True

    def _build(self) -> bool:
        safe_mkdir(self.build_dir)
        if not self._install_flash_utils():
            return False
        if not self._install_control_utils():
            return False
        return True

    def _flash(self) -> None:
        self.logger.info("Connected devices:")
        Bash("python3 -m pyocd list", sync=True).start_command()
        self.logger.info("Trying to flash")
        flash_dir = os.path.join(self.build_dir, "nrf52840-mdk/firmware/openthread/")
        flash_command = Bash("python3 -m pyocd load --target nrf52 ot-ncp-ftd.hex", sync=True, cwd=flash_dir)
        flash_command.start_command()
        if not flash_command.finished_success():
            self._log_error("Failed to flash!")
            return

    def setup(self) -> None:
        # TODO: Alert user we might install deps and give the option to opt out
        if current_platform.is_mac():
            raise NotImplementedError
        if not os.path.exists(self.build_dir):
            self.logger.warning(f"Existing build dir not found, building in {self.build_dir}!")
            if self._build():
                self._flash()
        else:
            self.logger.info(f"Build already found in {self.build_dir}; skipping!")
            self._flash()
