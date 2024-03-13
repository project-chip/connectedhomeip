#! /usr/bin/python3
import os
import sys
import time
import argparse
import asyncio
import modules.util as _util
from bleak import BleakScanner
from bleak import BleakClient


class Bluetooth:

    def __init__(self, args):
        self.args = args
        self.stop_event = asyncio.Event()
        self.search_addr = None
        self.found_dev = None
        self.devices = {}

    async def scan(self):
        print("Scanning....")
        async with BleakScanner(self.callback) as scanner:
            await self.stop_event.wait()

    def callback(self, dev, data):
        # Avoid duplicates
        if dev.address in self.devices: return
        # New device
        d = Device(dev, data)
        # Ignore faraway devies
        if (self.args.rssi is not None) and d.rssi < self.args.rssi: return
        self.devices[dev.address] = d
        print("{}".format(d))
        if self.search_addr == d.address:
            self.stop_event.set()
            self.found_dev = d
            print("{}Found: {}".format(_util.MARGIN, d.address))

    @staticmethod
    async def find(addr):
        print("{}* FIND {}".format(_util.MARGIN, addr))
        scanner = BleakScanner()
        dev = await scanner.find_device_by_address(addr)
        if dev is None: return None
        return Device(dev, None)

    @staticmethod
    async def read(addr, uuid):
        print("{}* READ {}".format(_util.MARGIN, addr))
        # Search for the device
        scanner = BleakScanner()
        dev = await scanner.find_device_by_address(addr)
        if dev is None: _util.fail("Device not found: {}".format(addr))
        # Connect
        print("{}Connecting...".format(_util.MARGIN))
        async with BleakClient(dev.address) as cli:
            print("{}  [{}]".format(_util.MARGIN, uuid))
            ch = Bluetooth.findCharact(cli, uuid)
            if ch is None: _util.fail("Unknown characteristic: {}".format(uuid))
            res =  await cli.read_gatt_char(uuid)
            print("{}READ({}):  {}".format(2*_util.MARGIN, len(res), res.hex()))
            return res

    @staticmethod
    async def write(addr, uuid, value):
        print("{}* WRITE {}".format(_util.MARGIN, addr))
        data = isinstance(value, bytearray) and value or bytearray.fromhex(value)
        # Search for the device
        scanner = BleakScanner()
        dev = await scanner.find_device_by_address(addr)
        if dev is None: _util.fail("Device not found: {}".format(addr))
        # Connect
        print("{}Connecting...".format(_util.MARGIN))
        async with BleakClient(dev.address) as cli:
            print("{}  [{}]".format(_util.MARGIN, uuid))
            ch = Bluetooth.findCharact(cli, uuid)
            if ch is None: _util.fail("Unknown characteristic: {}".format(uuid))
            await cli.write_gatt_char(ch, data)
            print("{}WRITE({}): {}".format(2*_util.MARGIN, len(data), data.hex()))

    @staticmethod
    async def test(addr, uuid, value):
        print("{}* TEST {}".format(_util.MARGIN, addr))
        data = isinstance(value, bytearray) and value or bytearray.fromhex(value)
        # Search for the device
        scanner = BleakScanner()
        dev = await scanner.find_device_by_address(addr)
        if dev is None: _util.fail("Device not found: {}".format(addr))
        # Connect
        print("{}Connecting...".format(_util.MARGIN))
        async with BleakClient(dev.address) as cli:
            print("{}  [{}]".format(_util.MARGIN, uuid))
            ch = Bluetooth.findCharact(cli, uuid)
            if ch is None: _util.fail("Unknown characteristic: {}".format(uuid))
            await cli.write_gatt_char(ch, data)
            print("{}WRITE({}): {}".format(2*_util.MARGIN, len(data), data.hex()))
            await asyncio.sleep(0.1)
            res =  await cli.read_gatt_char(ch)
            print("{}READ({}):  {}".format(2*_util.MARGIN, len(res), res.hex()))
            return res

    @staticmethod
    def findCharact(cli, uuid):
        for srv in cli.services:
            for chc in srv.characteristics:
                if uuid == chc.uuid:
                    return chc
        return None


class Device:

    def __init__(self, info, data = None):
        self.address = info.address
        self.name = info.name
        self.rssi = None
        self.local = None
        self.services = {}
        if data is not None:
            self.rssi = data.rssi
            self.local = data.local_name
            for uuid in data.service_uuids:
                self.services[uuid] = None

    def __str__(self):
        name = (self.name is not None) and "\"{}\"".format(self.name) or '?'
        rssi = (self.rssi is not None) and "{}".format(self.rssi) or '?'
        return "{}  {}  {}".format(self.address, rssi, name)

    async def collect(self):
        async with BleakClient(self.address) as cli:
            for srv in cli.services:
                s = Service(srv)
                self.services[s.uuid] = s

    def print(self):
        print("* {}".format(self))
        for u, s in self.services.items():
            print("{}+ {}".format(_util.MARGIN, s))
            for uu, c in s.characts.items():
                print("{}- {}".format(2 * _util.MARGIN, c))
                for uuu, d in c.descripts.items():
                    print("{}. {}".format(3 * _util.MARGIN, d))

class Service:
    def __init__(self, info):
        self.uuid = info.uuid
        self.name = info.description
        self.handle = info.handle
        self.characts = {}
        for char in info.characteristics:
            ch = Characteristic(char)
            self.characts[ch.uuid] = ch

    def __str__(self):
        handle = (self.handle is not None) and "{}".format(self.handle) or '?'
        return "{}  {:02}  \"{}\"".format(self.uuid, handle, self.name)

class Characteristic:
    def __init__(self, info):
        self.uuid = info.uuid
        self.handle = info.handle
        self.name = info.description
        self.descripts = {}
        for desc in info.descriptors:
            d = Descriptor(desc)
            self.descripts[d.uuid] = d

    def __str__(self):
        handle = (self.handle is not None) and "{}".format(self.handle) or '?'
        return "{}  {:02}  \"{}\"".format(self.uuid, handle, self.name)

class Descriptor:
    def __init__(self, info):
        self.uuid = info.uuid
        self.handle = info.handle
        self.name = info.description

    def __str__(self):
        handle = (self.handle is not None) and "{}".format(self.handle) or '?'
        return "{}  {:02}  \"{}\"".format(self.uuid, handle, self.name)


async def run(args):

    b = Bluetooth(args)
    if 'scan' == args.action:
        # Scan
        await b.scan()
    if 'desc' == args.action:
        # Describe
        d = await b.find(args.address)
        if d is None: _util.fail("Not found: {}".format(args.address))
        await d.collect()
        d.print()
    elif 'read' == args.action:
        # Read
        if args.specific is None: _util.fail("Missing UUID or handle")
        res = await Bluetooth.read(args.address, args.specific)
    elif 'write' == args.action:
        # Write
        if args.specific is None: _util.fail("Missing UUID or handle")
        if args.value is None: _util.fail("Missing value")
        data = bytearray.fromhex(args.value)
        res = await Bluetooth.write(args.address, args.specific, data)
    elif 'test' == args.action:
        # Test
        if args.specific is None: _util.fail("Missing UUID or handle")
        if args.value is None: _util.fail("Missing value")
        data = bytearray.fromhex(args.value)
        res = await Bluetooth.test(args.address, args.specific, data)

async def main(argv):
    base_dir = os.path.normpath(os.path.dirname(__file__))

    # Parse arguments
    parser = argparse.ArgumentParser(description='Provisioner Support')
    parser.add_argument('action', nargs='?', default=None)
    parser.add_argument('address', nargs='?', default=None)
    parser.add_argument('specific', nargs='?', default=None)
    parser.add_argument('value', nargs='?', default=None)
    parser.add_argument('-r', '--rssi', type=int, help='Minimum RSSI.', default=-50)
    args = parser.parse_args()
    await run(args)

asyncio.run(main(sys.argv[1:]))
