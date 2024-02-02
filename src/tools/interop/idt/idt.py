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

import argparse
import asyncio
import os
import shutil
import sys
from pathlib import Path

import features.probe.runner as probe_runner
from features.advertise import FakeMatterAdBle, FakeMatterAdDnssd
from features.capture import PacketCaptureRunner, controller
from features.capture.thread.runner import ThreadCaptureRunner
from features.discovery import MatterBleScanner, MatterDnssdListener
from utils.artifact import create_file_timestamp, safe_mkdir
from utils.host import current_platform
from utils.log import border_print, get_logger
from utils.error import write_error_report
from features.advertise import config as advertise_config
from utils.data import MATTER_APPLICATION_DEVICE_TYPES
from features.setup import list_available_targets, do_setup

import config

logger = get_logger(__file__)


class InteropDebuggingTool:

    def __init__(self) -> None:

        if config.ENABLE_COLOR:
            print(config.SPLASH)

        border_print(f"Version {config.IDT_VERSION}")

        current_platform.verify_host_dependencies()

        self.artifact_dir = None
        create_artifact_dir = True
        if len(sys.argv) == 1:
            create_artifact_dir = False
        elif sys.argv[1] != "capture" and sys.argv[1] != "discover" and sys.argv[1] != "probe":
            create_artifact_dir = False
        elif len(sys.argv) >= 3 and (sys.argv[2] == "-h" or sys.argv[2] == "--help"):
            create_artifact_dir = False

        if not os.environ["IDT_OUTPUT_DIR"]:
            print("Missing required env vars! Use /scripts!!!")
            sys.exit(1)

        self.artifact_dir_parent = os.path.join(
            Path(__file__).resolve().parent,
            os.environ["IDT_OUTPUT_DIR"])
        artifact_timestamp = create_file_timestamp()
        self.artifact_dir = os.path.join(
            self.artifact_dir_parent,
            f"idt_{artifact_timestamp}")
        if create_artifact_dir:
            safe_mkdir(self.artifact_dir)
            border_print(f"Using artifact dir {self.artifact_dir}")

        self.advertise_device_type_choices = [int(i, 16) for i in MATTER_APPLICATION_DEVICE_TYPES.keys()]

        self.capture_platforms_choices = controller.list_available_platforms()
        self.capture_platforms_default = "Android" if "Android" in self.capture_platforms_choices else None
        self.capture_platform_required = self.capture_platforms_default is None
        self.capture_ecosystems_choices = controller.list_available_ecosystems()
        self.capture_ecosystems_default = "ALL"
        self.capture_ecosystems_choices.append(self.capture_ecosystems_default)
        self.capture_pcap_interfaces_choices = current_platform.get_interfaces_available_for_pcap()
        # TODO: No None below
        self.capture_pcap_interfaces_default = "any" if "any" in self.capture_pcap_interfaces_choices else None
        self.capture_pcap_interface_required = self.capture_pcap_interfaces_default is None
        self.capture_pcap_wlan_channel_default, self.capture_pcap_wlan_width_default = current_platform. \
            current_wifi_channel_width(display=True)  # TODO: Remove display
        self.capture_pcap_wifi_bands_choices = ["2", "5"]
        self.capture_pcap_wifi_bands_default = "5" if current_platform.using_5g_band() else "2"
        self.capture_pcap_artifact_dir = os.path.join(self.artifact_dir, "pcap")
        self.capture_thread_modes_choices = ["none", "sniff", "on_network"]
        self.capture_thread_mode_default = self.capture_thread_modes_choices[0]
        self.capture_thread_artifact_dir = os.path.join(self.artifact_dir, "thread")

        self.discovery_ble_artifact_dir = os.path.join(self.artifact_dir, "ble")
        self.discovery_dnssd_artifact_dir = os.path.join(self.artifact_dir, "dnssd")

        self.probe_artifact_dir = os.path.join(self.artifact_dir, "features/probe")

        self.process_args()

    def process_args(self) -> None:
        parser = argparse.ArgumentParser(
            prog="idt",
            description="Interop Debugging Tool for Matter")
        subparsers = parser.add_subparsers(title="subcommands")

        advertise_parser = subparsers.add_parser("advertise",
                                                 help="Create a fake advertisement for a Matter device")
        advertise_parser.add_argument(
            "--vid",
            "-v",
            help=f"Vendor ID to use in the advertisement (int, default: {advertise_config.DEFAULT_VID})",
            default=advertise_config.DEFAULT_VID
        )
        advertise_parser.add_argument(
            "--pid",
            "-p",
            help=f"Product ID to use in the advertisement (int, default: {advertise_config.DEFAULT_PID})",
            default=advertise_config.DEFAULT_PID
        )
        advertise_parser.add_argument(
            "--discriminator",
            "-i",
            help=f"Discriminator to use in the advertisement (int, default: {advertise_config.DEFAULT_DISCRIMINATOR})",
            default=advertise_config.DEFAULT_DISCRIMINATOR
        )
        advertise_parser.add_argument(
            "--device_name",
            "-n",
            help=f"Device name to be used in the advertisement (str, default: {advertise_config.DEFAULT_DEVICE_NAME})",
            default=advertise_config.DEFAULT_DEVICE_NAME
        )
        advertise_parser.add_argument(
            "--device_type",
            "-d",
            help=f"Device type to be used in the advertisement (int, default: {advertise_config.DEFAULT_DEVICE_TYPE})",
            choices=self.advertise_device_type_choices,
            default=advertise_config.DEFAULT_DEVICE_TYPE
        )
        advertise_parser.add_argument(
            "--port",
            "-o",
            help=f"The port that this device is reachable on (int, default: {advertise_config.DEFAULT_PORT})",
            default=advertise_config.DEFAULT_PORT
        )
        advertise_parser.add_argument(
            "--commissioning_open",
            "-c",
            choices=["t", "f"],
            help=f"Whether commissioning window is open or not (default: {advertise_config.DEFAULT_COMMISSIONING_OPEN})",
            default=advertise_config.DEFAULT_COMMISSIONING_OPEN
        )
        advertise_parser.add_argument(
            "--mac_address",
            "-m",
            help=f"MAC Address to use for the instance name in this advertisement "
                 f"(str, default: {advertise_config.DEFAULT_MAC_ADDR})",
            default=advertise_config.DEFAULT_MAC_ADDR
        )
        advertise_parser.add_argument(
            "--allow_gua",
            "-g",
            help=f"Whether DNS-SD advertisements should include GUAs (if they're available on the host)"
                 f"(default: {advertise_config.DEFAULT_ALLOW_V6_GUA})",
            choices=["t", "f"],
            default=advertise_config.DEFAULT_ALLOW_V6_GUA
        )
        advertise_parser.add_argument(
            "--type",
            "-t",
            help="Specify the type of advertisement to create",
            required=True,
            choices=[
                "ble",
                "b",
                "dnssd",
                "d"])
        advertise_parser.set_defaults(func=self.command_advertise)

        capture_parser = subparsers.add_parser(
            "capture",
            help="Capture all information of interest while running a manual test")
        capture_platform_help = "Run capture for a particular platform"
        if self.capture_platforms_default:
            capture_platform_help += f" (default {self.capture_platforms_default})"
        capture_parser.add_argument("--platform",
                                    "-p",
                                    help=capture_platform_help,
                                    required=self.capture_platform_required,
                                    choices=self.capture_platforms_choices,
                                    default=self.capture_platforms_default)
        capture_parser.add_argument(
            "--ecosystem",
            "-e",
            help=f"Run capture for a particular ecosystem or ALL ecosystems (default {self.capture_ecosystems_default})",
            required=False,
            choices=self.capture_ecosystems_choices,
            default=self.capture_ecosystems_default)
        capture_parser.add_argument("--pcap",
                                    "-c",
                                    help="Run packet capture (default t)",
                                    required=False,
                                    choices=["t", "f"],
                                    default="t")
        capture_interface_help = "Specify packet capture interface"
        if self.capture_pcap_interfaces_default:
            capture_interface_help += f" (default {self.capture_pcap_interfaces_default})"
        capture_parser.add_argument(
            "--interface",
            "-i",
            help=capture_interface_help,
            required=self.capture_pcap_interface_required,
            choices=self.capture_pcap_interfaces_choices,
            default=self.capture_pcap_interfaces_default)
        capture_parser.add_argument("--monitor",
                                    "-m",
                                    help="Run packet capture using a monitor mode interface (default f)",
                                    required=False,
                                    choices=["t", "f"],
                                    default="f")
        capture_channel_help = \
            f"Use this Wi-Fi channel if in monitor mode (default {self.capture_pcap_wlan_channel_default})"
        capture_parser.add_argument("--channel",
                                    "-n",
                                    help=capture_channel_help,
                                    required=False,
                                    default=self.capture_pcap_wlan_channel_default)
        capture_parser.add_argument("--band",
                                    "-b",
                                    help=f"Use 2 for 2.4GHz, 5 for 5GHz (default {self.capture_pcap_wifi_bands_default})",
                                    required=False,
                                    choices=self.capture_pcap_wifi_bands_choices,
                                    default=self.capture_pcap_wifi_bands_default)
        capture_parser.add_argument("--width",
                                    "-w",
                                    help=f"Optionally set the channel width of a monitor mode pcap "
                                         f"(default {self.capture_pcap_wlan_width_default})",
                                    required=False,
                                    default=self.capture_pcap_wlan_width_default)
        capture_parser.add_argument(
            "--thread",
            "-t",
            help=f"Execute thread sniffer or join OTBR to network (Default {self.capture_thread_mode_default})",
            choices=self.capture_thread_modes_choices,
            default=self.capture_thread_mode_default
        )
        capture_parser.set_defaults(func=self.command_capture)

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
                "dnssd",
                "d"])
        # TODO: Add filter for discriminator
        # TODO: Verify that case is handled properly here
        discover_parser.add_argument(
            "--vid",
            "-v",
            help="Only display advertisements with this Vendor ID. Hex values (without 0x prefix) are expected. Eg "
                 "FFF1",
            required=False)
        discover_parser.add_argument(
            "--pid",
            "-p",
            help="If vid argument is set, filter advertisements by this Product ID as well. Hex values (without 0x "
                 "prefix) are expected. Eg 8000",
            required=False)
        discover_parser.add_argument(
            "--v4",
            "-4",
            help="Whether to browse on IPv4 or not (Default t)",
            required=False,
            choices=["t", "f"],
            default="t")
        discover_parser.add_argument(
            "--v6",
            "-6",
            help="Whether to browse on IPv6 or not (Default t)",
            required=False,
            choices=["t", "f"],
            default="t")
        probe_parser = subparsers.add_parser("probe",
                                             help="Probe the environment for Matter and general networking info")
        probe_parser.set_defaults(func=self.command_probe)

        setup_parser = subparsers.add_parser("setup",
                                             help="Build / flash devices to use with the tool, e.g. otncp")
        setup_parser.add_argument("--target",
                                  "-t",
                                  help="The target to setup",
                                  choices=list_available_targets(),
                                  required=True)
        setup_parser.set_defaults(func=self.command_setup)

        args, unknown = parser.parse_known_args()
        if not hasattr(args, "func"):
            parser.print_help()
        else:
            args.func(args)

    def command_advertise(self, args: argparse.Namespace) -> None:
        commissioning_open = args.commissioning_open == "t"
        allow_gua = args.allow_gua == "t"
        if args.type[0] == "b":
            asyncio.run(FakeMatterAdBle().advertise())
        else:
            asyncio.run(FakeMatterAdDnssd(args.vid,
                                          args.pid,
                                          args.discriminator,
                                          args.device_name,
                                          args.device_type,
                                          args.port,
                                          commissioning_open,
                                          args.mac_address,
                                          allow_gua).advertise())

    def command_capture(self, args: argparse.Namespace) -> None:
        pcap = args.pcap == "t"
        monitor_mode = args.monitor == "t"
        if monitor_mode:
            logger.warning("You have selected monitor mode pcap. If you proceed, wireless connections from this "
                           "machine will not be available for the duration of the capture. On certain Linux systems, "
                           "you may need to restart your machine to return to normal Wi-Fi function.")
            logger.critical("Enter y/Y to proceed with monitor mode, any other input to exit!")
            answer = input()
            if answer.lower().strip() != "y":
                logger.info("y/Y not provided, exiting!")
                sys.exit(1)
            # TODO: Validate arguments
        pcap_runner = None if not pcap else PacketCaptureRunner(
            self.capture_pcap_artifact_dir, args.interface, monitor_mode, args.channel, args.band, args.width)
        if pcap:
            border_print("Starting pcap")
            safe_mkdir(self.capture_pcap_artifact_dir)
            pcap_runner.start_pcap()
        thread = args.thread != "none"
        thread_runner = None if not thread else ThreadCaptureRunner(
            self.capture_thread_artifact_dir,
            args.thread
        )
        if thread:
            border_print(f"Starting thread capture in mode {args.thread}")
            safe_mkdir(self.capture_thread_artifact_dir)
            thread_runner.start()
        asyncio.run(controller.init_ecosystems(args.platform,
                                               args.ecosystem,
                                               self.artifact_dir))
        asyncio.run(controller.start())
        asyncio.run(controller.run_analyzers())
        if pcap:
            border_print("Stopping pcap")
            pcap_runner.stop_pcap()
        if thread:
            border_print("Stopping thread")
            thread_runner.stop()
        asyncio.run(controller.stop())
        asyncio.run(controller.probe())
        border_print("Checking error report")
        write_error_report(self.artifact_dir)
        border_print("Compressing artifacts...")
        self.zip_artifacts()

    def command_discover(self, args: argparse.Namespace) -> None:
        vendor_id = None if not args.vid else "0x" + args.vid.lstrip("0")
        product_id = None if not args.pid else "0x" + args.pid.lstrip("0")
        if args.type[0] == "b":
            safe_mkdir(self.discovery_ble_artifact_dir)
            scanner = MatterBleScanner(self.discovery_ble_artifact_dir, vendor_id, product_id)
            asyncio.run(scanner.browse_interactive())
            self.zip_artifacts()
        else:
            safe_mkdir(self.discovery_dnssd_artifact_dir)
            MatterDnssdListener(self.discovery_dnssd_artifact_dir,
                                vendor_id,
                                product_id,
                                v4=args.v4 == "t",
                                v6=args.v6 == "t").browse_interactive()
            self.zip_artifacts()

    def command_probe(self, args: argparse.Namespace) -> None:
        border_print("Starting generic Matter prober for local environment!")
        safe_mkdir(self.discovery_dnssd_artifact_dir)
        safe_mkdir(self.probe_artifact_dir)
        probe_runner.run_probes(self.probe_artifact_dir, self.discovery_dnssd_artifact_dir)
        self.zip_artifacts()

    def command_setup(self, args: argparse.Namespace) -> None:
        border_print("Executing setup!")
        do_setup(args.target)

    def zip_artifacts(self) -> None:
        # TODO: 1 Standardized machine readable report / platform & ecosystem
        zip_basename = os.path.basename(self.artifact_dir)
        archive_file = shutil.make_archive(zip_basename,
                                           "zip",
                                           root_dir=self.artifact_dir)
        output_zip = shutil.move(archive_file, self.artifact_dir_parent)
        border_print(f"Output zip: {output_zip}")
