#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import logging
import os
import re
import shlex
import subprocess
import sys

# Absolute path to Tizen Studio CLI tool.
tizen_sdk_root = os.environ["TIZEN_SDK_ROOT"]

# Setup basic logging capabilities.
logging.basicConfig(level=logging.DEBUG)

parser = argparse.ArgumentParser(
    description="Run Tizen on QEMU.")
parser.add_argument(
    '-i', '--interactive', action='store_true',
    help="run QEMU in interactive mode (no output redirection, no runner)")
parser.add_argument(
    '--smp', metavar='NUM', type=int, default=2,
    help=("the number of CPUs available in QEMU; default: %(default)s"))
parser.add_argument(
    '--memory', metavar='SIZE', type=int, default=512,
    help=("the size of RAM assigned to QEMU; default: %(default)s"))
parser.add_argument(
    '--virtio-net', action='store_true',
    help="enable external network access via virtio-net")
parser.add_argument(
    '--kernel', metavar='PATH',
    default=os.path.join(tizen_sdk_root, "iot-qemu-virt-zImage"),
    help=("path to the kernel image; "
          "default: $TIZEN_SDK_ROOT/iot-qemu-virt-zImage"))
parser.add_argument(
    '--image-root', metavar='IMAGE',
    default=os.path.join(tizen_sdk_root, "iot-rootfs.img"),
    help=("path to the root image; "
          "default: $TIZEN_SDK_ROOT/iot-rootfs.img"))
parser.add_argument(
    '--image-data', metavar='IMAGE',
    default=os.path.join(tizen_sdk_root, "iot-sysdata.img"),
    help=("path to the system data image; "
          "default: $TIZEN_SDK_ROOT/iot-sysdata.img"))
parser.add_argument(
    '--image-iso', metavar='IMAGE',
    help=("path to the ISO image with the runner script; the ISO image "
          "should have 'CHIP' label and a file named 'runner.sh' at the "
          "root directory"))
parser.add_argument(
    '--output', metavar='FILE', default="/dev/null",
    help="store the QEMU output in a FILE")

args = parser.parse_args()

qemu_args = [
    'qemu-system-arm',
    '-monitor', 'null',
    '-serial', 'stdio',
    '-display', 'none',
    '-M', 'virt',
    '-smp', str(args.smp),
    '-m', str(args.memory),
]

if args.virtio_net:
    # Add network support.
    qemu_args += [
        '-device', 'virtio-net-device,netdev=virtio-net',
        '-netdev', 'user,id=virtio-net',
    ]

if args.image_iso:
    # Add a block device for the runner ISO image.
    qemu_args += [
        '-device', 'virtio-blk-device,drive=virtio-blk3',
        '-drive', 'file=%s,id=virtio-blk3,if=none,format=raw' % args.image_iso,
    ]

# Add Tizen image block devices.
qemu_args += [
    '-device', 'virtio-blk-device,drive=virtio-blk2',
    '-drive', 'file=%s,id=virtio-blk2,if=none,format=raw,readonly=on' % args.image_data,
    # XXX: Device for the root image has to be added as the last one so we can
    #      use /dev/vda as the root device in the kernel command line arguments.
    '-device', 'virtio-blk-device,drive=virtio-blk1',
    '-drive', 'file=%s,id=virtio-blk1,if=none,format=raw,readonly=on' % args.image_root,
]

kernel_args = "console=ttyAMA0 earlyprintk earlycon root=/dev/vda"
if args.interactive:
    # Run root shell instead of the runner script.
    kernel_args += " rootshell"

qemu_args += [
    '-kernel', args.kernel,
    '-append', kernel_args,
]

if args.interactive:
    # Run QEMU in interactive mode.
    sys.exit(subprocess.call(qemu_args))

status = 0
# Run QEMU.
with open(args.output, "wb") as output:
    logging.info("run: %s", " ".join(map(shlex.quote, qemu_args)))
    with subprocess.Popen(qemu_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT) as proc:
        for line in iter(proc.stdout.readline, b''):

            # Forward the output to the stdout and the log file.
            sys.stdout.write(line.decode(sys.stdout.encoding))
            sys.stdout.flush()
            output.write(line)

            # Check if the runner script has finished and save the status.
            stop = re.match(rb'^### RUNNER STOP: (?P<status>\d+)', line)
            if stop:
                status = int(stop.group('status'))

# Return with the status of the runner script.
sys.exit(status)
