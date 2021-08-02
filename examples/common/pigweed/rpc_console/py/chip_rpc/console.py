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
    client - the pw_rpc.Client
    protos - protocol buffer messages indexed by proto package

An example RPC command:
  rpcs.chip.rpc.DeviceCommon.GetDeviceInfo()
"""

import argparse
from collections import namedtuple
import logging
import sys
from typing import Any, BinaryIO
import socket
from inspect import cleandoc
import serial  # type: ignore
import re
import pw_cli.log
from pw_console.console_app import embed
from pw_console.__main__ import create_temp_log_file
from pw_hdlc.rpc import HdlcRpcClient, default_channels

# Protos
from button_service import button_service_pb2
from device_service import device_service_pb2
from lighting_service import lighting_service_pb2
from locking_service import locking_service_pb2
from wifi_service import wifi_service_pb2

_LOG = logging.getLogger(__name__)
_DEVICE_LOG = logging.getLogger('rpc_device')

PW_RPC_MAX_PACKET_SIZE = 256
SOCKET_SERVER = 'localhost'
SOCKET_PORT = 33000

PROTOS = [button_service_pb2,
          lighting_service_pb2,
          locking_service_pb2,
          wifi_service_pb2,
          device_service_pb2]


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
    group.add_argument('-s',
                       '--socket-addr',
                       type=str,
                       help='use socket to connect to server, type default for\
            localhost:33000, or manually input the server address:port')
    return parser.parse_args()


def _start_ipython_terminal(client: HdlcRpcClient) -> None:
    """Starts an interactive IPython terminal with preset variables."""
    local_variables = dict(
        client=client,
        channel_client=client.client.channel(1),
        rpcs=client.client.channel(1).rpcs,
        protos=client.protos.packages,
        # Include the active pane logger for creating logs in the repl.
        LOG=_DEVICE_LOG,
    )

    welcome_message = cleandoc("""
        Welcome to the CHIP RPC Console!

        Press F1 for help.
        Example commands:

          rpcs.chip.rpc.DeviceCommon.GetDeviceInfo()

          LOG.warning('Message appears console log window.')
    """)

    embed(global_vars=local_variables,
          local_vars=None,
          loggers=[_DEVICE_LOG],
          repl_startup_message=welcome_message,
          help_text=__doc__,
          app_title="CHIP Console")


class SocketClientImpl:
    def __init__(self, config: str):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socket_server = ''
        socket_port = 0

        if config == 'default':
            socket_server = SOCKET_SERVER
            socket_port = SOCKET_PORT
        else:
            socket_server, socket_port_str = config.split(':')
            socket_port = int(socket_port_str)
        self.socket.connect((socket_server, socket_port))

    def write(self, data: bytes):
        self.socket.sendall(data)

    def read(self, num_bytes: int = PW_RPC_MAX_PACKET_SIZE):
        return self.socket.recv(num_bytes)


def write_to_output(data: bytes,
                    unused_output: BinaryIO = sys.stdout.buffer,):
    log_line = data
    RegexStruct = namedtuple('RegexStruct', 'platform type regex match_num')
    LEVEL_MAPPING = {"I": logging.INFO, "W": logging.WARNING,
                     "E": logging.ERROR, "F": logging.FATAL, "V": logging.DEBUG, "D": logging.DEBUG}
    ESP_CHIP_REGEX = r"(?P<level>[IWEFV]) \((?P<time>\d+)\) (?P<mod>chip\[[a-zA-Z]+\]):\s(?P<msg>.*)"
    ESP_APP_REGEX = r"(?P<level>[IWEFVD]) \((?P<time>\d+)\) (?P<mod>[a-z\-_A-Z]+):\s(?P<msg>.*)"
    LogRegexes = [RegexStruct("ESP", "CHIP", re.compile(ESP_CHIP_REGEX), 4),
                  RegexStruct("ESP", "APP", re.compile(ESP_APP_REGEX), 4)
                  ]
    for line in log_line.decode(errors="surrogateescape").splitlines():
        fields = {'level': logging.INFO, "time": "",
                  "mod": "", "type": "", "msg": line}
        for log_regex in LogRegexes:
            match = log_regex.regex.search(line)
            if match and len(match.groups()) == log_regex.match_num:
                fields['type'] = log_regex.type
                fields.update(match.groupdict())
                if "level" in match.groupdict():
                    fields["level"] = LEVEL_MAPPING[fields["level"]]
                break
        _DEVICE_LOG.log(fields["level"], fields["msg"], extra={'extra_metadata_fields': {
                        "time": fields["time"], "type": fields["type"], "mod": fields["mod"]}})


def console(device: str, baudrate: int,
            socket_addr: str, output: Any) -> int:
    """Starts an interactive RPC console for HDLC."""
    # argparse.FileType doesn't correctly handle '-' for binary files.
    if output is sys.stdout:
        output = sys.stdout.buffer

    logfile = create_temp_log_file()
    pw_cli.log.install(logging.INFO, True, False, logfile)

    if socket_addr is None:
        serial_device = serial.Serial(device, baudrate, timeout=1)
        def read(): return serial_device.read(8192)
        write = serial_device.write
    else:
        try:
            socket_device = SocketClientImpl(socket_addr)
            read = socket_device.read
            write = socket_device.write
        except ValueError:
            _LOG.exception('Failed to initialize socket at %s', socket_addr)
            return 1

    _start_ipython_terminal(
        HdlcRpcClient(read, PROTOS, default_channels(write),
                      lambda data: write_to_output(data, output)))
    return 0


def main() -> int:
    return console(**vars(_parse_args()))


if __name__ == '__main__':
    sys.exit(main())
