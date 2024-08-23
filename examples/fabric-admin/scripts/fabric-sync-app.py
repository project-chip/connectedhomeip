#!/usr/bin/env python3

import contextlib
import sys
import os
import asyncio
from argparse import ArgumentParser


async def forwarder(f_in, f_out, prefix: str):
    """Forward f_in to f_out with a prefix attached."""
    while True:
        line = await f_in.readline()
        if not line:
            break
        f_out.buffer.write(prefix.encode())
        f_out.buffer.write(line)


async def run(tag, program, *args, stdin=None):
    p = await asyncio.create_subprocess_exec(program, *args,
                                             stdout=asyncio.subprocess.PIPE,
                                             stderr=asyncio.subprocess.PIPE,
                                             stdin=stdin)
    # Add the stdout and stderr processing to the event loop.
    asyncio.create_task(forwarder(p.stderr, sys.stderr, tag))
    asyncio.create_task(forwarder(p.stdout, sys.stdout, tag))
    return p


async def run_admin(program, storage_dir=None, paa_trust_store_path=None,
                    commissioner_name=None, commissioner_node_id=None,
                    commissioner_vendor_id=None):
    args = []
    if storage_dir is not None:
        args.extend(["--storage-directory", storage_dir])
    if paa_trust_store_path is not None:
        args.extend(["--paa-trust-store-path", paa_trust_store_path])
    if commissioner_name is not None:
        args.extend(["--commissioner-name", commissioner_name])
    if commissioner_node_id is not None:
        args.extend(["--commissioner-nodeid", commissioner_node_id])
    if commissioner_vendor_id is not None:
        args.extend(["--commissioner-vendor-id", commissioner_vendor_id])
    p = await run("[ADMIN]", program, "interactive", "start", *args)
    await p.wait()


async def run_bridge(program, storage_dir=None, discriminator=None,
                     passcode=None, secured_device_port=None):
    args = []
    if storage_dir is not None:
        args.extend(["--KVS",
                     os.path.join(storage_dir, "chip_fabric_bridge_kvs")])
    if discriminator is not None:
        args.extend(["--discriminator", discriminator])
    if passcode is not None:
        args.extend(["--passcode", passcode])
    if secured_device_port is not None:
        args.extend(["--secured-device-port", secured_device_port])
    p = await run("[BRIDGE]", program, *args, stdin=asyncio.subprocess.DEVNULL)
    await p.wait()


async def main(args):
    await asyncio.gather(
        run_admin(
            args.app_admin,
            storage_dir=args.storage_dir,
            paa_trust_store_path=args.paa_trust_store_path,
            commissioner_name=args.commissioner_name,
            commissioner_node_id=args.commissioner_nodeid,
            commissioner_vendor_id=args.commissioner_vendor_id,
        ),
        run_bridge(
            args.app_bridge,
            storage_dir=args.storage_dir,
            secured_device_port=args.secured_device_port,
            discriminator=args.discriminator,
            passcode=args.passcode,
        ))


if __name__ == "__main__":
    parser = ArgumentParser(description="Fabric-Sync Example Application")
    parser.add_argument("--app-admin", metavar="PATH",
                        default="out/linux-x64-fabric-admin-rpc/fabric-admin",
                        help="path to the fabric-admin executable; default=%(default)s")
    parser.add_argument("--app-bridge", metavar="PATH",
                        default="out/linux-x64-fabric-bridge-rpc/fabric-bridge-app",
                        help="path to the fabric-bridge executable; default=%(default)s")
    parser.add_argument("--storage-dir", metavar="PATH",
                        help="directory to place storage files in")
    parser.add_argument("--paa-trust-store-path", metavar="PATH",
                        help="path to directory holding PAA certificates")
    parser.add_argument("--commissioner-name", metavar="NAME",
                        help="commissioner name to use for the admin")
    parser.add_argument("--commissioner-nodeid", metavar="NUM",
                        help="commissioner node ID to use for the admin")
    parser.add_argument("--commissioner-vendor-id", metavar="NUM",
                        help="commissioner vendor ID to use for the admin")
    parser.add_argument("--secured-device-port", metavar="NUM",
                        help="secure messages listen port to use for the bridge")
    parser.add_argument("--discriminator", metavar="NUM",
                        help="discriminator to use for the bridge")
    parser.add_argument("--passcode", metavar="NUM",
                        help="passcode to use for the bridge")
    with contextlib.suppress(KeyboardInterrupt):
        asyncio.run(main(parser.parse_args()))
