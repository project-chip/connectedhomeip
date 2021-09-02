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

import argparse
import logging
from pw_hdlc.rpc import HdlcRpcClient, default_channels, write_to_file
from pw_status import Status
import serial  # type: ignore
import subprocess
import sys
import time
from typing import Any

# RPC Protos
from nl_test_service import nl_test_pb2

PW_LOG = logging.getLogger(__name__)

PROTOS = [nl_test_pb2]


class colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'


PASS_STRING = colors.OKGREEN + u'\N{check mark}' + colors.ENDC
FAIL_STRING = colors.FAIL + 'FAILED' + colors.ENDC


def _parse_args():
    """Parses and returns the command line arguments."""
    parser = argparse.ArgumentParser(
        description="CHIP on device unit test runner.")
    parser.add_argument('-d', '--device', help='the serial port to use')
    parser.add_argument('-b',
                        '--baudrate',
                        type=int,
                        default=115200,
                        help='the baud rate to use')
    parser.add_argument('-f', '--flash_image',
                        help='a firmware image which will be flashed berfore runnning the test')
    parser.add_argument(
        '-o',
        '--output',
        type=argparse.FileType('wb'),
        default=sys.stdout.buffer,
        help=('The file to which to write device output (HDLC channel 1); '
              'provide - or omit for stdout.'))
    return parser.parse_args()


def flash_device(device: str, flash_image: str, **kwargs):
    """flashes the EFR32 device using commander"""
    err = subprocess.call(
        ['commander', 'flash', '--device', 'EFR32', flash_image])
    if err:
        raise Exception("flash failed")


def get_hdlc_rpc_client(device: str, baudrate: int, output: Any, **kwargs):
    """Get the HdlcRpcClient based on arguments."""
    serial_device = serial.Serial(device, baudrate, timeout=1)
    def read(): return serial_device.read(8192)
    write = serial_device.write
    return HdlcRpcClient(read, PROTOS, default_channels(write),
                         lambda data: write_to_file(data, output))


def runner(client) -> int:
    """ Run the tests"""
    status, result = client.client.channel(
        1).rpcs.chip.rpc.NlTest.Run(pw_rpc_timeout_s=120)

    if not status.ok():
        raise Exception("Error running test RPC: {}".format(status))

    total_failed = 0
    total_run = 0
    for streamed_data in result:
        if streamed_data.HasField("test_suite_start"):
            print("\n{}".format(
                colors.HEADER + streamed_data.test_suite_start.suite_name) + colors.ENDC)
        if streamed_data.HasField("test_case_run"):
            print("\t{}: {}".format(streamed_data.test_case_run.test_case_name,
                  FAIL_STRING if streamed_data.test_case_run.failed else PASS_STRING))
        if streamed_data.HasField("test_suite_tests_run_summary"):
            total_run += streamed_data.test_suite_tests_run_summary.total_count
            total_failed += streamed_data.test_suite_tests_run_summary.failed_count
            print("{}Total tests failed: {} of {}".format(
                  colors.OKGREEN if streamed_data.test_suite_tests_run_summary.failed_count == 0 else colors.FAIL,
                  streamed_data.test_suite_tests_run_summary.failed_count,
                  streamed_data.test_suite_tests_run_summary.total_count) + colors.ENDC)
        if streamed_data.HasField("test_suite_asserts_summary"):
            print("{}Total asserts failed:  {} of {}".format(
                  colors.OKGREEN if streamed_data.test_suite_asserts_summary.failed_count == 0 else colors.FAIL,
                  streamed_data.test_suite_asserts_summary.failed_count,
                  streamed_data.test_suite_asserts_summary.total_count) + colors.ENDC)
        for step in ["test_suite_setup", "test_suite_teardown", "test_case_initialize", "test_case_terminate"]:
            if streamed_data.HasField(step):
                print(colors.OKCYAN + "\t{}: {}".format(step,
                                                        FAIL_STRING if getattr(streamed_data, step).failed else PASS_STRING))
    print(colors.OKBLUE + colors.BOLD +
          "\n\nAll tests completed" + colors.ENDC)
    print("{}Total of all tests failed: {} of {}".format(
        colors.OKGREEN if total_failed == 0 else colors.FAIL,
        total_failed, total_run) + colors.ENDC)
    return total_failed


def main() -> int:
    args = _parse_args()
    if args.flash_image:
        flash_device(**vars(args))
        time.sleep(1)  # Give time for device to boot
    client = get_hdlc_rpc_client(**vars(args))
    return runner(client)


if __name__ == '__main__':
    sys.exit(main())
