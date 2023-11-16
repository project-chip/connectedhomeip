from typing import TYPE_CHECKING

from utils.artifact import create_standard_log_name, log
from utils.shell import Bash

if TYPE_CHECKING:
    from capture.platform.android import Android

from . import config

logger = log.get_logger(__file__)


class Capabilities:

    def __init__(self, platform: "Android"):
        self.logger = logger
        self.platform = platform
        self.c_has_tcpdump = False
        self.c_has_root = False
        self.c_is_64 = False
        self.c_hci_snoop_enabled = False
        self.artifact = create_standard_log_name("capabilities", "txt", parent=platform.artifact_dir)

    def __repr__(self):
        s = "Detected capabilities:\n"
        for item in [x for x in dir(self) if x.startswith("c_")]:
            s += f"{item}: {getattr(self, item)}\n"
        return s

    def check_snoop_log(self) -> bool:
        return config.hci_log_level in self.platform.run_adb_command("shell getprop persist.bluetooth.btsnooplogmode",
                                                                     capture_output=True).get_captured_output()

    def check_capabilities(self):
        self.logger.info("Checking if device has root")
        self.c_has_root = self.platform.run_adb_command(
            "shell which su", capture_output=True).finished_success()
        if self.c_has_root:
            self.logger.warning("adb root!")
            Bash("adb root", sync=True).start_command()
        self.logger.info("Checking if device has tcpdump")
        self.c_has_tcpdump = self.platform.run_adb_command(
            "shell which tcpdump", capture_output=True).finished_success()
        self.logger.info("Checking device CPU arch")
        self.c_is_64 = "8" in self.platform.run_adb_command("shell cat /proc/cpuinfo | grep rch",
                                                            capture_output=True).get_captured_output()
        self.c_hci_snoop_enabled = self.check_snoop_log()
        if not self.c_hci_snoop_enabled:
            self.logger.info("HCI not enabled, attempting to enable!")
            self.platform.run_adb_command(
                f"shell setprop persist.bluetooth.btsnooplogmode {config.hci_log_level}")
            self.platform.run_adb_command("shell svc bluetooth disable")
            self.platform.run_adb_command("shell svc bluetooth enable")
            self.c_hci_snoop_enabled = self.check_snoop_log()
            if not self.c_hci_snoop_enabled:
                self.logger.error("Failed to enabled HCI snoop log")
        self.logger.info(self)
        with open(self.artifact, "w") as artifact:
            artifact.write(str(self))
