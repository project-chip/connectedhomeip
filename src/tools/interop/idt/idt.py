import argparse
import logging
import os
import shutil
import sys
from pathlib import Path

from capture import CaptureEcosystems, CapturePlatforms, PacketCaptureRunner
from capture.file_utils import border_print, create_file_timestamp, safe_mkdir
from discovery.matter_ble import MatterBleScanner
from discovery.matter_mdns import MatterMdnsListener

# TODO argument for log level
logging.basicConfig(
    format='%(asctime)s.%(msecs)03d %(levelname)s {%(module)s} [%(funcName)s]\n%(message)s \n',
    level=logging.INFO)


class InteropDebuggingTool:

    def __init__(self) -> None:

        self.artifact_dir = None
        create_artifact_dir = True
        if len(sys.argv) == 1:
            create_artifact_dir = False
        elif sys.argv[1] != "capture" and sys.argv[1] != "discover":
            create_artifact_dir = False
        elif len(sys.argv) >= 3 and (sys.argv[2] == "-h" or sys.argv[2] == "--help"):
            create_artifact_dir = False

        if not os.environ['IDT_OUTPUT_DIR']:
            print('Missing required env vars! Use /scripts!!!')
            sys.exit(1)

        self.artifact_dir_parent = os.path.join(
            Path(__file__).resolve().parent,
            os.environ['IDT_OUTPUT_DIR'])
        artifact_timestamp = create_file_timestamp()
        self.artifact_dir = os.path.join(
            self.artifact_dir_parent,
            f'idt_{artifact_timestamp}')
        if create_artifact_dir:
            safe_mkdir(self.artifact_dir)
            border_print(f"Using artifact dir {self.artifact_dir}")

        self.available_platforms = CapturePlatforms.list_available_platforms()
        self.available_platforms_default = 'Android' if 'Android' in self.available_platforms else None
        self.platform_required = self.available_platforms_default is None

        self.available_ecosystems = CaptureEcosystems.list_available_ecosystems()
        self.available_ecosystems_default = 'ALL'
        self.available_ecosystems.append(self.available_ecosystems_default)

        # TODO: Support loading net interfaces on more platforms
        net_interface_path = "/sys/class/net/"
        if os.path.exists(net_interface_path):
            self.available_net_interfaces = os.listdir(net_interface_path)
        else:
            self.available_net_interfaces = []
        self.available_net_interfaces_default = "wlan0mon" if "wlan0mon" in self.available_net_interfaces else None
        if self.available_net_interfaces_default is None:
            saw_one_wl_prefix = False
            only_one_wl_prefix = True
            last_seen_wl_interface = ""
            for interface in self.available_net_interfaces:
                if interface.startswith("wl") and not saw_one_wl_prefix:
                    last_seen_wl_interface = interface
                    saw_one_wl_prefix = True
                elif interface.startswith("wl"):
                    only_one_wl_prefix = False
            if saw_one_wl_prefix and only_one_wl_prefix:
                self.available_net_interfaces_default = last_seen_wl_interface
        self.net_interface_required = self.available_net_interfaces_default is None
        self.pcap_artifact_dir = os.path.join(self.artifact_dir, "pcap")

        self.ble_artifact_dir = os.path.join(self.artifact_dir, "ble")
        self.mdns_artifact_dir = os.path.join(self.artifact_dir, "mdns")

        self.process_args()

    def process_args(self) -> None:
        parser = argparse.ArgumentParser(
            prog="idt",
            description="Interop Debugging Tool for Matter")

        subparsers = parser.add_subparsers(title="subcommands")

        discover_parser = subparsers.add_parser(
            "discover", help="Discover all Matter devices")
        discover_parser.set_defaults(func=self.command_discover)
        discover_parser.add_argument(
            "--type",
            "-t",
            help="Specify the type of discovery to execute",
            required=True,
            choices=[
                "ble",
                "b",
                "mdns",
                "m"])

        capture_parser = subparsers.add_parser(
            "capture",
            help="Capture all information of interest while running a manual test")

        platform_help = "Run capture for a particular platform"
        if self.available_platforms_default:
            platform_help += f" (default {self.available_platforms_default})"
        capture_parser.add_argument("--platform",
                                    "-p",
                                    help=platform_help,
                                    required=self.platform_required,
                                    choices=self.available_platforms,
                                    default=self.available_platforms_default)

        # TODO: Ability for each ecosystem to specify its own custom arguments
        capture_parser.add_argument(
            "--ecosystem",
            "-e",
            help="Run capture for a particular ecosystem or ALL ecosystems (default ALL)",
            required=False,
            choices=self.available_ecosystems,
            default=self.available_ecosystems_default)

        capture_parser.add_argument("--pcap",
                                    "-c",
                                    help="Run packet capture (default t)",
                                    required=False,
                                    choices=['t', 'f'],
                                    default='t')

        interface_help = "Run packet capture against a specified interface"
        if self.available_net_interfaces_default:
            interface_help += f" (default {self.available_net_interfaces_default})"
        capture_parser.add_argument(
            "--interface",
            "-i",
            help=interface_help,
            required=self.net_interface_required,
            choices=self.available_net_interfaces,
            default=self.available_net_interfaces_default)

        # TODO: Implement
        capture_parser.add_argument(
            "--additional",
            "-a",
            help="Run ble and mdns scanners in the background while capturing (default t)",
            required=False,
            choices=[
                't',
                'f'],
            default='t')

        capture_parser.set_defaults(func=self.command_capture)

        args, unknown = parser.parse_known_args()
        if not hasattr(args, 'func'):
            parser.print_help()
        else:
            args.func(args)

    def command_discover(self, args: argparse.Namespace) -> None:
        if args.type[0] == "b":
            safe_mkdir(self.ble_artifact_dir)
            MatterBleScanner(self.ble_artifact_dir).browse_interactive()
        else:
            safe_mkdir(self.mdns_artifact_dir)
            MatterMdnsListener(self.mdns_artifact_dir).browse_interactive()

    def zip_artifacts(self) -> None:
        zip_basename = os.path.basename(self.artifact_dir)
        archive_file = shutil.make_archive(zip_basename,
                                           'zip',
                                           root_dir=self.artifact_dir)
        output_zip = shutil.move(archive_file, self.artifact_dir_parent)
        print(f'Output zip: {output_zip}')

    def command_capture(self, args: argparse.Namespace) -> None:

        # TODO: Add UnsupportedCapturePlatformException exception handling here
        self.available_ecosystems.remove('ALL')
        ecosystems = {}
        if args.ecosystem == 'ALL':
            for ecosystem in self.available_ecosystems:
                ecosystems[ecosystem] = CaptureEcosystems.get_ecosystem_impl(
                    ecosystem, args.platform, self.artifact_dir)
        else:
            ecosystems[args.ecosystem] = CaptureEcosystems.get_ecosystem_impl(
                args.ecosystem, args.platform, self.artifact_dir)

        pcap = args.pcap == 't'
        pcap_runner = None if not pcap else PacketCaptureRunner(
            self.pcap_artifact_dir, args.interface)
        if pcap:
            border_print("Starting pcap")
            safe_mkdir(self.pcap_artifact_dir)
            pcap_runner.start_pcap()

        for ecosystem in ecosystems:
            border_print(f"Starting capture for {ecosystem}")
            ecosystems[ecosystem].start_capture()

        border_print(f"Press enter to stop streaming", important=True)
        input("")

        if pcap:
            border_print("Stopping pcap")
            pcap_runner.stop_pcap()

        for ecosystem in ecosystems:
            border_print(f"Stopping capture for {ecosystem}")
            ecosystems[ecosystem].stop_capture()

        for ecosystem in ecosystems:
            border_print(f"Analyze + collate capture for {ecosystem}")
            ecosystems[ecosystem].analyze_capture()

        border_print("Zipping artifacts, this may take a second!")
        self.zip_artifacts()
