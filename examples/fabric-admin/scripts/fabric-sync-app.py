#!/usr/bin/env python3

# Copyright (c) 2024 Project CHIP Authors
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

import asyncio
import contextlib
import os
import shutil
import signal
import sys
import typing
from argparse import ArgumentParser
from pathlib import Path
from tempfile import TemporaryDirectory


async def forward_f(prefix: bytes, f_in: asyncio.StreamReader,
                    f_out: typing.BinaryIO, cb=None):
    """Forward f_in to f_out with a prefix attached.

    This function can optionally feed received lines to a callback function.
    """
    while line := await f_in.readline():
        if cb is not None:
            cb(line)
        f_out.buffer.write(prefix)
        f_out.buffer.write(line)
        f_out.flush()


async def forward_pipe(pipe_path: str, f_out: asyncio.StreamWriter):
    """Forward named pipe to f_out.

    Unfortunately, Python does not support async file I/O on named pipes. This
    function performs busy waiting with a short asyncio-friendly sleep to read
    from the pipe.
    """
    fd = os.open(pipe_path, os.O_RDONLY | os.O_NONBLOCK)
    while True:
        try:
            data = os.read(fd, 1024)
            if data:
                f_out.write(data)
                await f_out.drain()
            if not data:
                await asyncio.sleep(0.1)
        except BlockingIOError:
            await asyncio.sleep(0.1)


async def forward_stdin(f_out: asyncio.StreamWriter):
    """Forward stdin to f_out."""
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader()
    protocol = asyncio.StreamReaderProtocol(reader)
    await loop.connect_read_pipe(lambda: protocol, sys.stdin)
    while line := await reader.readline():
        f_out.write(line)
        await f_out.drain()


class Subprocess:

    def __init__(self, tag: str, program: str, *args):
        self.event = asyncio.Event()
        self.tag = tag.encode()
        self.program = program
        self.args = args
        self.expected_output = None

    def _check_output(self, line: bytes):
        if self.expected_output is not None and self.expected_output in line:
            self.event.set()

    async def run(self):
        self.p = await asyncio.create_subprocess_exec(self.program, *self.args,
                                                      stdin=asyncio.subprocess.PIPE,
                                                      stdout=asyncio.subprocess.PIPE,
                                                      stderr=asyncio.subprocess.PIPE)
        # Add the stdout and stderr processing to the event loop.
        asyncio.create_task(forward_f(self.tag, self.p.stderr, sys.stderr))
        asyncio.create_task(forward_f(self.tag, self.p.stdout, sys.stdout,
                                      cb=self._check_output))

    async def send(self, message: str, expected_output: str = None, timeout: float = None):
        """Send a message to a process and optionally wait for a response."""

        if expected_output is not None:
            self.expected_output = expected_output.encode()
            self.event.clear()

        self.p.stdin.write((message + "\n").encode())
        await self.p.stdin.drain()

        if expected_output is not None:
            await asyncio.wait_for(self.event.wait(), timeout=timeout)
            self.expected_output = None

    async def wait(self):
        await self.p.wait()

    def terminate(self):
        self.p.terminate()


async def run_admin(program, storage_dir=None, rpc_admin_port=None, rpc_bridge_port=None,
                    paa_trust_store_path=None, commissioner_name=None,
                    commissioner_node_id=None, commissioner_vendor_id=None):
    args = []
    if storage_dir is not None:
        args.extend(["--storage-directory", storage_dir])
    if rpc_admin_port is not None:
        args.extend(["--local-server-port", str(rpc_admin_port)])
    if rpc_bridge_port is not None:
        args.extend(["--fabric-bridge-server-port", str(rpc_bridge_port)])
    if paa_trust_store_path is not None:
        args.extend(["--paa-trust-store-path", paa_trust_store_path])
    if commissioner_name is not None:
        args.extend(["--commissioner-name", commissioner_name])
    if commissioner_node_id is not None:
        args.extend(["--commissioner-nodeid", str(commissioner_node_id)])
    if commissioner_vendor_id is not None:
        args.extend(["--commissioner-vendor-id", str(commissioner_vendor_id)])
    p = Subprocess("[FS-ADMIN]", program, "interactive", "start", *args)
    await p.run()
    return p


async def run_bridge(program, storage_dir=None, rpc_admin_port=None,
                     rpc_bridge_port=None, discriminator=None, passcode=None,
                     secured_device_port=None):
    args = []
    if storage_dir is not None:
        args.extend(["--KVS", storage_dir.joinpath("chip_fabric_bridge_kvs")])
    if rpc_admin_port is not None:
        args.extend(["--fabric-admin-server-port", str(rpc_admin_port)])
    if rpc_bridge_port is not None:
        args.extend(["--local-server-port", str(rpc_bridge_port)])
    if discriminator is not None:
        args.extend(["--discriminator", str(discriminator)])
    if passcode is not None:
        args.extend(["--passcode", str(passcode)])
    if secured_device_port is not None:
        args.extend(["--secured-device-port", str(secured_device_port)])
    p = Subprocess("[FS-BRIDGE]", program, *args)
    await p.run()
    return p


async def main(args):

    # Node ID of the bridge on the fabric.
    bridge_node_id = 1

    if args.commissioner_node_id == bridge_node_id:
        raise ValueError(f"NodeID={bridge_node_id} is reserved for the local fabric-bridge")

    storage_dir = args.storage_dir
    if storage_dir is not None:
        storage_dir.mkdir(parents=True, exist_ok=True)
    else:
        storage = TemporaryDirectory(prefix="fabric-sync-app")
        storage_dir = Path(storage.name)

    if args.stdin_pipe and not args.stdin_pipe.exists():
        os.mkfifo(args.stdin_pipe)

    admin, bridge = await asyncio.gather(
        run_admin(
            args.app_admin,
            storage_dir=storage_dir,
            rpc_admin_port=args.app_admin_rpc_port,
            rpc_bridge_port=args.app_bridge_rpc_port,
            paa_trust_store_path=args.paa_trust_store_path,
            commissioner_name=args.commissioner_name,
            commissioner_node_id=args.commissioner_node_id,
            commissioner_vendor_id=args.commissioner_vendor_id,
        ),
        run_bridge(
            args.app_bridge,
            storage_dir=storage_dir,
            rpc_admin_port=args.app_admin_rpc_port,
            rpc_bridge_port=args.app_bridge_rpc_port,
            secured_device_port=args.secured_device_port,
            discriminator=args.discriminator,
            passcode=args.passcode,
        ))

    loop = asyncio.get_event_loop()

    def terminate():
        with contextlib.suppress(ProcessLookupError):
            admin.terminate()
        with contextlib.suppress(ProcessLookupError):
            bridge.terminate()
        if args.stdin_pipe:
            args.stdin_pipe.unlink(missing_ok=True)
        loop.remove_signal_handler(signal.SIGINT)
        loop.remove_signal_handler(signal.SIGTERM)

    loop.add_signal_handler(signal.SIGINT, terminate)
    loop.add_signal_handler(signal.SIGTERM, terminate)

    # Wait a bit for apps to start.
    await asyncio.sleep(1)

    try:
        # Check whether the bridge is already commissioned. If it is,
        # we will get the response, otherwise we will hit timeout.
        await admin.send(
            f"descriptor read device-type-list {bridge_node_id} 1 --timeout 1",
            # Log message which should appear in the fabric-admin output if
            # the bridge is already commissioned.
            expected_output="Reading attribute: Cluster=0x0000_001D Endpoint=0x1 AttributeId=0x0000_0000",
            timeout=1.5)
    except asyncio.TimeoutError:
        # Commission the bridge to the admin.
        cmd = f"fabricsync add-local-bridge {bridge_node_id}"
        if args.passcode is not None:
            cmd += f" --setup-pin-code {args.passcode}"
        if args.secured_device_port is not None:
            cmd += f" --local-port {args.secured_device_port}"
        await admin.send(
            cmd,
            # Wait for the log message indicating that the bridge has been
            # added to the fabric.
            f"Commissioning complete for node ID {bridge_node_id:#018x}: success",
            timeout=30)

    # Open commissioning window with original setup code for the bridge.
    cw_endpoint_id = 0
    cw_option = 0  # 0: Original setup code, 1: New setup code
    cw_timeout = 600
    cw_iteration = 1000
    cw_discriminator = 0
    await admin.send(f"pairing open-commissioning-window {bridge_node_id} {cw_endpoint_id}"
                     f" {cw_option} {cw_timeout} {cw_iteration} {cw_discriminator}")

    def get_input_forwarder():
        if args.stdin_pipe:
            return forward_pipe(args.stdin_pipe, admin.p.stdin)
        return forward_stdin(admin.p.stdin)

    try:
        # Wait for any of the tasks to complete.
        _, pending = await asyncio.wait([
            asyncio.create_task(admin.wait()),
            asyncio.create_task(bridge.wait()),
            asyncio.create_task(get_input_forwarder()),
        ], return_when=asyncio.FIRST_COMPLETED)
        # Cancel the remaining tasks.
        for task in pending:
            task.cancel()
    except Exception as e:
        print(e, file=sys.stderr)

    terminate()
    # Make sure that we will not return until both processes are terminated.
    await admin.wait()
    await bridge.wait()


if __name__ == "__main__":
    parser = ArgumentParser(description="Fabric-Sync Example Application")
    parser.add_argument("--app-admin", metavar="PATH", type=Path,
                        default=shutil.which("fabric-admin"),
                        help="path to the fabric-admin executable; default=%(default)s")
    parser.add_argument("--app-bridge", metavar="PATH", type=Path,
                        default=shutil.which("fabric-bridge-app"),
                        help="path to the fabric-bridge executable; default=%(default)s")
    parser.add_argument("--app-admin-rpc-port", metavar="PORT", type=int,
                        help="fabric-admin RPC server port")
    parser.add_argument("--app-bridge-rpc-port", metavar="PORT", type=int,
                        help="fabric-bridge RPC server port")
    parser.add_argument("--stdin-pipe", metavar="PATH", type=Path,
                        help="read input from a named pipe instead of stdin")
    parser.add_argument("--storage-dir", metavar="PATH", type=Path,
                        help=("directory to place storage files in; by default "
                              "volatile storage is used"))
    parser.add_argument("--paa-trust-store-path", metavar="PATH", type=Path,
                        help="path to directory holding PAA certificates")
    parser.add_argument("--commissioner-name", metavar="NAME",
                        help="commissioner name to use for the admin")
    parser.add_argument("--commissioner-node-id", metavar="NUM", type=int,
                        help="commissioner node ID to use for the admin")
    parser.add_argument("--commissioner-vendor-id", metavar="NUM", type=int,
                        help="commissioner vendor ID to use for the admin")
    parser.add_argument("--secured-device-port", metavar="NUM", type=int,
                        help="secure messages listen port to use for the bridge")
    parser.add_argument("--discriminator", metavar="NUM", type=int,
                        help="discriminator to use for the bridge")
    parser.add_argument("--passcode", metavar="NUM", type=int,
                        help="passcode to use for the bridge")
    args = parser.parse_args()
    if args.app_admin is None or not args.app_admin.exists():
        parser.error("fabric-admin executable not found in PATH. Use '--app-admin' argument to provide it.")
    if args.app_bridge is None or not args.app_bridge.exists():
        parser.error("fabric-bridge-app executable not found in PATH. Use '--app-bridge' argument to provide it.")
    if args.stdin_pipe and args.stdin_pipe.exists() and not args.stdin_pipe.is_fifo():
        parser.error("given stdin pipe exists and is not a named pipe")
    with contextlib.suppress(KeyboardInterrupt):
        asyncio.run(main(args))
