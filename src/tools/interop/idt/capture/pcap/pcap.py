import os
import time

from capture.file_utils import create_standard_log_name
from capture.shell_utils import BashRunner


class PacketCaptureRunner:

    def __init__(self, artifact_dir: str, interface: str) -> None:

        self.artifact_dir = artifact_dir
        self.output_path = str(
            os.path.join(
                self.artifact_dir,
                create_standard_log_name(
                    "pcap",
                    "cap")))
        self.start_delay_seconds = 3
        self.interface = interface
        self.pcap_proc: BashRunner | None = None
        self.pcap_command = f"tcpdump -i {self.interface} -n -w {self.output_path}"

    def start_pcap(self) -> None:
        if self.pcap_proc is None or not self.pcap_proc.command_is_running():
            self.pcap_proc = BashRunner(self.pcap_command)
            print("Pausing to check if pcap started...")
            time.sleep(self.start_delay_seconds)
            if not self.pcap_proc.command_is_running():
                print(
                    f"Pcap did not start, you might need root; please authorize if prompted.")
                BashRunner("sudo echo \"\"", sync=True)
                print("Retrying pcap with sudo...")
                self.pcap_command = f"sudo {self.pcap_command}"
                self.pcap_proc = BashRunner(self.pcap_command)
                time.sleep(self.start_delay_seconds)
            if not self.pcap_proc.command_is_running():
                print("WARNING Failed to start pcap!")
            else:
                print(f"Pcap started, output in {self.output_path}")

    def stop_pcap(self) -> None:
        # TODO: Automated analysis
        self.pcap_proc.stop_command(soft=True)
        print("Pcap stopped")
