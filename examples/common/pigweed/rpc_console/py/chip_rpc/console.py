#
#    Copyright (c) 2021 Project CHIP Authors
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
"""Console for interacting with CHIP lighting app over RPC.

To start the console, provide a serial port as the --device argument

  python -m chip_rpc.console --device /dev/ttyUSB0

Alternatively to connect to a linux CHIP device provide the port.

  python -m chip_rpc.console -s localhost:33000

This starts an IPython console for communicating with the connected device. A
few variables are predefined in the interactive console. These include:

    rpcs   - used to invoke RPCs
    device - the serial device used for communication
    client - the HDLC rpc client
    protos - protocol buffer messages indexed by proto package

An example RPC command:
    rpcs.chip.rpc.Device.GetDeviceInfo()
    device.rpcs.chip.rpc.Device.GetDeviceInfo()
"""
import argparse
import sys
from pathlib import Path
from typing import Any, Collection

import pw_system.console
from pw_hdlc import rpc

# Protos
# isort: off
from actions_service import actions_service_pb2
from attributes_service import attributes_service_pb2
from boolean_state_service import boolean_state_service_pb2
from button_service import button_service_pb2
from descriptor_service import descriptor_service_pb2
from device_service import device_service_pb2
from echo_service import echo_pb2
from fabric_admin_service import fabric_admin_service_pb2
from fabric_bridge_service import fabric_bridge_service_pb2
from lighting_service import lighting_service_pb2
from locking_service import locking_service_pb2
from ot_cli_service import ot_cli_service_pb2
from thread_service import thread_service_pb2
from wifi_service import wifi_service_pb2


def _parse_args():
    """Parses and returns the command line arguments."""
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-d', '--device', help='the serial port to use')
    parser.add_argument('-b',
                        '--baudrate',
                        type=int,
                        default=115200,
                        help='the baud rate to use')
    parser.add_argument(
        '-o',
        '--output',
        type=argparse.FileType('wb'),
        default=sys.stdout.buffer,
        help=('The file to which to write device output (HDLC channel 1); '
              'provide - or omit for stdout.'))
    parser.add_argument(
        '-r',
        '--raw_serial',
        action="store_true",
        help=('Use raw serial instead of HDLC/RPC'))
    parser.add_argument("--token-databases",
                        metavar='elf_or_token_database',
                        nargs="+",
                        type=Path,
                        help="Path to tokenizer database csv file(s).")
    group.add_argument('-s',
                       '--socket-addr',
                       type=str,
                       help='use socket to connect to server, type default for\
            localhost:33000, or manually input the server address:port')
    return parser.parse_args()


def show_console(device: str, baudrate: int,
                 token_databases: Collection[Path],
                 socket_addr: str, output: Any, raw_serial: bool) -> int:

    # TODO: this shows a default console with little customization
    #       Ideally we should at least customize the default messages
    #
    #
    # For now example of how to run commands:
    #
    #   device.rpcs.chip.rpc.Device.GetDeviceInfo()
    #

    pw_system.console.console(
        device=device,
        baudrate=baudrate,
        socket_addr=socket_addr,
        output=output,
        hdlc_encoding=not raw_serial,
        token_databases=token_databases,
        logfile="",
        device_logfile="",
        channel_id=rpc.DEFAULT_CHANNEL_ID,

        # Defaults beyond the original console
        proto_globs=[],
        ticks_per_second=None,
        host_logfile="",
        json_logfile="",
        rpc_logging=False,
        # the pt-python based console seems to break on python 3.1 with
        # "set_wakeup_fd only works in main thread of the main interpreter"
        use_ipython=True,
        compiled_protos=[
                actions_service_pb2,
                attributes_service_pb2,
                boolean_state_service_pb2,
                button_service_pb2,
                descriptor_service_pb2,
                device_service_pb2,
                echo_pb2,
                fabric_admin_service_pb2,
                fabric_bridge_service_pb2,
                lighting_service_pb2,
                locking_service_pb2,
                ot_cli_service_pb2,
                thread_service_pb2,
                wifi_service_pb2]
    )


def main() -> int:
    return show_console(**vars(_parse_args()))


if __name__ == '__main__':
    sys.exit(main())
