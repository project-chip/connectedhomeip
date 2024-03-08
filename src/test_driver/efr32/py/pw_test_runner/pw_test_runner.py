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

import argparse
import logging
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

import serial  # type: ignore
from pw_hdlc import rpc
from pw_unit_test.rpc import run_tests

PW_LOG = logging.getLogger(__name__)

PROTO = Path(os.environ["PW_ROOT"],
             "pw_unit_test/pw_unit_test_proto/unit_test.proto")


class colors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"


PASS_STRING = colors.OKGREEN + "\N{check mark}" + colors.ENDC
FAIL_STRING = colors.FAIL + "FAILED" + colors.ENDC


def _parse_args():
    """Parses and returns the command line arguments."""
    parser = argparse.ArgumentParser(
        description="CHIP on device unit test runner.")
    parser.add_argument("-d", "--device", help="the serial port to use")
    parser.add_argument(
        "-b", "--baudrate", type=int, default=115200, help="the baud rate to use"
    )
    parser.add_argument(
        "-f",
        "--flash_image",
        help="a firmware image which will be flashed berfore runnning the test",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=argparse.FileType("wb"),
        default=sys.stdout.buffer,
        help=(
            "The file to which to write device output (HDLC channel 1); "
            "provide - or omit for stdout."
        ),
    )
    return parser.parse_args()


def flash_device(device: str, flash_image: str, **kwargs):
    """flashes the EFR32 device using commander"""
    err = subprocess.call(
        ["commander", "flash", "--device", "EFR32", flash_image])
    if err:
        raise Exception("flash failed")


def get_hdlc_rpc_client(device: str, baudrate: int, output: Any, **kwargs):
    """Get the HdlcRpcClient based on arguments."""
    serial_device = serial.Serial(device, baudrate, timeout=1)
    reader = rpc.SerialReader(serial_device, 8192)
    write = serial_device.write
    return rpc.HdlcRpcClient(
        reader,
        PROTO,
        rpc.default_channels(write),
        lambda data: rpc.write_to_file(data, output),
    )


def runner(client: rpc.HdlcRpcClient) -> int:
    """Run the tests"""

    test_records = run_tests(client.rpcs())

    return len(test_records.failing_tests)


def main() -> int:
    args = _parse_args()
    if args.flash_image:
        flash_device(**vars(args))
        time.sleep(1)  # Give time for device to boot

    with get_hdlc_rpc_client(**vars(args)) as client:
        return runner(client)


if __name__ == "__main__":
    sys.exit(main())
