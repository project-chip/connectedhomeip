from modules.parameters import Types, Formats, ID
from bleak import BleakClient
from bleak import BleakScanner
import modules.channel as _base
import modules.util as _util
import asyncio
import time


class BluetoothChannel(_base.Channel):
    PROVISION_TX_UUID = '18ee2ef5-263d-4559-959f-4f9c429f9d11'
    PROVISION_RX_UUID = '18ee2ef5-263d-4559-959f-4f9c429f9d12'

    def __init__(self, paths, args, address) -> None:
        super().__init__(_base.Channel.BLE)
        # ch_arg = args.get(ID.kChannel)
        # if ch_arg.value is None: raise ValueError("Missing channel argument")
        self.address = address
        self.loop = asyncio.get_event_loop()
        self.client = None

    async def connect(self):
        scanner = BleakScanner()
        dev = await scanner.find_device_by_address(self.address)
        if dev is None: raise AssertionError("Device not found: {}".format(self.address))
        self.client = BleakClient(dev.address)
        await self.client.connect()
        self.tx_charact = self.findCharact(BluetoothChannel.PROVISION_TX_UUID)
        self.rx_charact = self.findCharact(BluetoothChannel.PROVISION_RX_UUID)

    async def wait(self):
        await asyncio.sleep(0.1)

    def findCharact(self, uuid):
        for srv in self.client.services:
            for chc in srv.characteristics:
                if uuid == chc.uuid:
                    return chc
        raise ValueError("Unknown characteristic: {}".format(uuid))

    def open(self):
        print("* Open BLUETOOTH channel to {}\n".format(self.address))
        self.loop.run_until_complete(self.connect())

    def write(self, data):
        self.loop.run_until_complete(self.client.write_gatt_char(self.tx_charact, data))
        # Wait! The firmware needs time to update the output characteristic
        time.sleep(0.1)

    def read(self):
        return self.loop.run_until_complete(self.client.read_gatt_char(self.rx_charact))

    def close(self):
        self.loop.run_until_complete(self.client.disconnect())
        print("* Bluetooth channel closed.\n")
