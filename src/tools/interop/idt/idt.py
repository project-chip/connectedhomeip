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

import probe.runner as probe_runner
from capture import PacketCaptureRunner, controller
from discovery import MatterBleScanner, MatterDnssdListener
from utils.artifact import create_file_timestamp, safe_mkdir
from utils.host_platform import get_available_interfaces, verify_host_dependencies
from utils.log import border_print

import config

splash = '''\x1b[0m
\x1b[32;1m┌────────┐\x1b[33;20m▪\x1b[32;1m \x1b[34;1m┌──────┐ \x1b[33;20m• \x1b[35;1m┌──────────┐ \x1b[33;20m●
\x1b[32;1m│░░░░░░░░│  \x1b[34;1m│░░░░░░└┐ \x1b[33;20mﾟ\x1b[35;1m│░░░░░░░░░░│
\x1b[32;1m└──┐░░┌──┘\x1b[33;20m۰\x1b[32;1m \x1b[34;1m│░░┌┐░░░│  \x1b[35;1m└───┐░░┌───┘
\x1b[32;1m   │░░│     \x1b[34;1m│░░│└┐░░│\x1b[33;20m▫ \x1b[35;1m \x1b[33;20m۰\x1b[35;1m  │░░│  \x1b[33;20m｡
\x1b[32;1m \x1b[33;20m•\x1b[32;1m │░░│  \x1b[33;20m●  \x1b[34;1m│░░│┌┘░░│  \x1b[35;1m    │░░│
\x1b[32;1m┌──┘░░└──┐  \x1b[34;1m│░░└┘░░░│  \x1b[35;1m    │░░│ \x1b[33;20m•
\x1b[32;1m│░░░░░░░░│  \x1b[34;1m│░░░░░░┌┘\x1b[33;20m۰ \x1b[35;1m \x1b[33;20m▪\x1b[35;1m  │░░│
\x1b[32;1m└────────┘\x1b[33;20m•\x1b[32;1m \x1b[34;1m└──────┘\x1b[33;20m｡  \x1b[35;1m    └──┘ \x1b[33;20m▫
\x1b[32;1m✰ Interop\x1b[34;1m  ✰ Debugging\x1b[35;1m   ✰ Tool
\x1b[0m'''


class InteropDebuggingTool:

    def __init__(self) -> None:
        if config.enable_color:
            print(splash)
        self.artifact_dir = None
        create_artifact_dir = True
        if len(sys.argv) == 1:
            create_artifact_dir = False
        elif sys.argv[1] != "capture" and sys.argv[1] != "discover":
            create_artifact_dir = False
        elif len(sys.argv) >= 3 and (sys.argv[2] == "-h" or sys.argv[2] == "--help"):
            create_artifact_dir = False

        verify_host_dependencies(["adb", "tcpdump"])

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

        self.available_platforms = controller.list_available_platforms()
        self.available_platforms_default = 'Android' if 'Android' in self.available_platforms else None
        self.platform_required = self.available_platforms_default is None

        self.available_ecosystems = controller.list_available_ecosystems()
        self.available_ecosystems_default = 'ALL'
        self.available_ecosystems.append(self.available_ecosystems_default)

        self.available_net_interfaces = get_available_interfaces()
        self.available_net_interfaces_default = "any" if "any" in self.available_net_interfaces else None
        self.pcap_artifact_dir = os.path.join(self.artifact_dir, "pcap")
        self.net_interface_required = self.available_net_interfaces_default is None

        self.ble_artifact_dir = os.path.join(self.artifact_dir, "ble")
        self.dnssd_artifact_dir = os.path.join(self.artifact_dir, "dnssd")
        self.prober_dir = os.path.join(self.artifact_dir, "probes")

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
                "dnssd",
                "d"])

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

        interface_help = "Specify packet capture interface"
        if self.available_net_interfaces_default:
            interface_help += f" (default {self.available_net_interfaces_default})"
        capture_parser.add_argument(
            "--interface",
            "-i",
            help=interface_help,
            required=self.net_interface_required,
            choices=self.available_net_interfaces,
            default=self.available_net_interfaces_default)

        capture_parser.set_defaults(func=self.command_capture)

        prober_parser = subparsers.add_parser("probe",
                                              help="Probe the environment for Matter and general networking info")
        prober_parser.set_defaults(func=self.command_probe)

        args, unknown = parser.parse_known_args()
        if not hasattr(args, 'func'):
            parser.print_help()
        else:
            args.func(args)

    def command_discover(self, args: argparse.Namespace) -> None:
        if args.type[0] == "b":
            safe_mkdir(self.ble_artifact_dir)
            scanner = MatterBleScanner(self.ble_artifact_dir)
            asyncio.run(scanner.browse_interactive())
            self.zip_artifacts()
        else:
            safe_mkdir(self.dnssd_artifact_dir)
            MatterDnssdListener(self.dnssd_artifact_dir).browse_interactive()
            self.zip_artifacts()

    def zip_artifacts(self) -> None:
        zip_basename = os.path.basename(self.artifact_dir)
        archive_file = shutil.make_archive(zip_basename,
                                           'zip',
                                           root_dir=self.artifact_dir)
        output_zip = shutil.move(archive_file, self.artifact_dir_parent)
        border_print(f'Output zip: {output_zip}')

    def command_capture(self, args: argparse.Namespace) -> None:
        pcap = args.pcap == 't'
        pcap_runner = None if not pcap else PacketCaptureRunner(
            self.pcap_artifact_dir, args.interface)
        if pcap:
            border_print("Starting pcap")
            safe_mkdir(self.pcap_artifact_dir)
            pcap_runner.start_pcap()
        asyncio.run(controller.init_ecosystems(args.platform,
                                               args.ecosystem,
                                               self.artifact_dir))
        asyncio.run(controller.start())
        asyncio.run(controller.run_analyzers())
        if pcap:
            border_print("Stopping pcap")
            pcap_runner.stop_pcap()
        asyncio.run(controller.stop())
        asyncio.run(controller.probe())
        border_print("Checking error report")
        controller.write_error_report(self.artifact_dir)
        border_print("Compressing artifacts...")
        self.zip_artifacts()

    def command_probe(self, args: argparse.Namespace) -> None:
        border_print("Starting generic Matter prober for local environment!")
        safe_mkdir(self.dnssd_artifact_dir)
        safe_mkdir(self.prober_dir)
        probe_runner.run_probes(self.prober_dir, self.dnssd_artifact_dir)
        self.zip_artifacts()
