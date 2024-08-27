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
import signal
import sys
import os
from argparse import ArgumentParser


BRIDGE_COMMISSIONED = asyncio.Event()
# Log message which should appear in the fabric-admin output if
# the bridge is already commissioned.
BRIDGE_COMMISSIONED_MSG = b"Reading attribute: Cluster=0x0000_001D Endpoint=0x1 AttributeId=0x0000_0000"


async def forward_f(f_in, f_out, prefix: str):
    """Forward f_in to f_out with a prefix attached."""
    global BRIDGE_COMMISSIONED
    while True:
        line = await f_in.readline()
        if not line:
            break
        if not BRIDGE_COMMISSIONED.is_set() and BRIDGE_COMMISSIONED_MSG in line:
            BRIDGE_COMMISSIONED.set()
        f_out.buffer.write(prefix.encode())
        f_out.buffer.write(line)
        f_out.flush()


async def forward_stdin(f_out: asyncio.StreamWriter):
    """Forward stdin to f_out."""
    loop = asyncio.get_event_loop()
    reader = asyncio.StreamReader()
    protocol = asyncio.StreamReaderProtocol(reader)
    await loop.connect_read_pipe(lambda: protocol, sys.stdin)
    while True:
        line = await reader.readline()
        if not line:
            sys.exit(0)
        f_out.write(line)


async def run(tag, program, *args, stdin=None):
    p = await asyncio.create_subprocess_exec(program, *args,
                                             stdout=asyncio.subprocess.PIPE,
                                             stderr=asyncio.subprocess.PIPE,
                                             stdin=stdin)
    # Add the stdout and stderr processing to the event loop.
    asyncio.create_task(forward_f(p.stderr, sys.stderr, tag))
    asyncio.create_task(forward_f(p.stdout, sys.stdout, tag))
    return p


async def run_admin(program, storage_dir=None, rpc_admin_port=None,
                    rpc_bridge_port=None, paa_trust_store_path=None,
                    commissioner_name=None, commissioner_node_id=None,
                    commissioner_vendor_id=None):
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
    return await run("[FS-ADMIN]", program, "interactive", "start", *args,
                     stdin=asyncio.subprocess.PIPE)


async def run_bridge(program, storage_dir=None, rpc_admin_port=None,
                     rpc_bridge_port=None, discriminator=None, passcode=None,
                     secured_device_port=None):
    args = []
    if storage_dir is not None:
        args.extend(["--KVS",
                     os.path.join(storage_dir, "chip_fabric_bridge_kvs")])
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
    return await run("[FS-BRIDGE]", program, *args,
                     stdin=asyncio.subprocess.DEVNULL)


async def main(args):

    if args.commissioner_node_id == 1:
        raise ValueError("NodeID=1 is reserved for the local fabric-bridge")

    if args.storage_dir is not None:
        os.makedirs(args.storage_dir, exist_ok=True)

    def terminate(signum, frame):
        admin.terminate()
        bridge.terminate()
        sys.exit(1)

    signal.signal(signal.SIGINT, terminate)
    signal.signal(signal.SIGTERM, terminate)

    admin, bridge = await asyncio.gather(
        run_admin(
            args.app_admin,
            storage_dir=args.storage_dir,
            rpc_admin_port=args.app_admin_rpc_port,
            rpc_bridge_port=args.app_bridge_rpc_port,
            paa_trust_store_path=args.paa_trust_store_path,
            commissioner_name=args.commissioner_name,
            commissioner_node_id=args.commissioner_node_id,
            commissioner_vendor_id=args.commissioner_vendor_id,
        ),
        run_bridge(
            args.app_bridge,
            storage_dir=args.storage_dir,
            rpc_admin_port=args.app_admin_rpc_port,
            rpc_bridge_port=args.app_bridge_rpc_port,
            secured_device_port=args.secured_device_port,
            discriminator=args.discriminator,
            passcode=args.passcode,
        ))

    # Wait a bit for apps to start.
    await asyncio.sleep(1)

    try:
        # Check whether the bridge is already commissioned. If it is,
        # we will get the response, otherwise we will hit timeout.
        cmd = "descriptor read device-type-list 1 1 --timeout 1"
        admin.stdin.write((cmd + "\n").encode())
        await asyncio.wait_for(BRIDGE_COMMISSIONED.wait(), timeout=1)
    except asyncio.TimeoutError:
        # Commission the bridge to the admin.
        cmd = "fabricsync add-local-bridge 1"
        if args.passcode is not None:
            cmd += f" --setup-pin-code {args.passcode}"
        if args.secured_device_port is not None:
            cmd += f" --local-port {args.secured_device_port}"
        admin.stdin.write((cmd + "\n").encode())

    try:
        await asyncio.gather(
            admin.wait(), bridge.wait(),
            forward_stdin(admin.stdin))
    except SystemExit:
        admin.terminate()
        bridge.terminate()
    except Exception:
        admin.terminate()
        bridge.terminate()
        raise


if __name__ == "__main__":
    parser = ArgumentParser(description="Fabric-Sync Example Application")
    parser.add_argument("--app-admin", metavar="PATH",
                        default="out/linux-x64-fabric-admin-rpc/fabric-admin",
                        help="path to the fabric-admin executable; default=%(default)s")
    parser.add_argument("--app-bridge", metavar="PATH",
                        default="out/linux-x64-fabric-bridge-rpc/fabric-bridge-app",
                        help="path to the fabric-bridge executable; default=%(default)s")
    parser.add_argument("--app-admin-rpc-port", metavar="PORT", type=int,
                        help="fabric-admin RPC server port")
    parser.add_argument("--app-bridge-rpc-port", metavar="PORT", type=int,
                        help="fabric-bridge RPC server port")
    parser.add_argument("--storage-dir", metavar="PATH",
                        help="directory to place storage files in")
    parser.add_argument("--paa-trust-store-path", metavar="PATH",
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
    with contextlib.suppress(KeyboardInterrupt):
        asyncio.run(main(parser.parse_args()))
