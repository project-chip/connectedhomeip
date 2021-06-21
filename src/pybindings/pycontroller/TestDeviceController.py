from pybindings.PyChip.chip import Controller, PersistentStorageDelegate, Platform, RendezvousParameters
from pybindings.PyChip.chip import SetupPayload, Transport, DeviceLayer
from pybindings.PyChip.chip import QRCodeSetupPayloadParser
from pybindings.PyChip import ChipExceptions

import time
import threading
import ctypes
import atexit
import logging
from sys import platform

is_linux_platform = platform == "linux" or platform == "linux2"
logger = logging.getLogger("ChipBLEMgr")
logger.setLevel(logging.DEBUG)


ChipExceptions.CHIPErrorToException(Platform.MemoryInit())
platform_manager = DeviceLayer.PlatformMgr()

# This has to go before InitChipStack or else there will be GLib critical errors.
if is_linux_platform:
    DeviceLayer.Internal.BLEMgrImpl().ConfigureBle(0, True)  # Central is set to True

atexit.register(platform_manager.Shutdown)
chip_stack_init_result = platform_manager.InitChipStack()
print("Chip Stack Init Result: {}".format(chip_stack_init_result))
ChipExceptions.CHIPErrorToException(chip_stack_init_result)

platform_manager.RunEventLoop()
setup_payload = SetupPayload()
qr_code_parser = QRCodeSetupPayloadParser("VP:vendorpayload%MT:W0GU2OTB00KA0648G00")
payload = qr_code_parser.populatePayload(setup_payload)
print("Version: {}".format(setup_payload.version))
print("Vendor ID: {}".format(setup_payload.vendorID))
print("Product ID: {}".format(setup_payload.productID))
print("Commissioning Flow: {}".format(setup_payload.commissioningFlow))
print("Rendezvous Information Flag: {}".format(setup_payload.rendezvousInformation.Raw()))
print("Discriminator: {}".format(setup_payload.discriminator))
print("Setup Pin Code: {}".format(setup_payload.setUpPINCode))
print(setup_payload.getAllOptionalVendorData())

discriminator = 0xF00
setup_code = 20202021
node_id = 1
local_node_id = 112233 

class PyPersistentStorageDelegate(PersistentStorageDelegate):
    data = {}
    def SyncGetKeyValue(self, key,  buffer, size):
        print('SyncGetKeyValue key: {}'.format(key))
        print('Stored Value: {}'.format(self.data.get(key, 4016)))
        return self.data.get(key, 4016)

    def SyncSetKeyValue(self, key, value, size):
        print('SyncSetKeyValue Key: {}'.format(key))
        print('SyncSetKeyValue Value: {}'.format(value))
        self.data[key] = value
        return 0

    def SyncDeleteKeyValue(self, key):
        print('SyncDeleteKeyValue: {}'.format(key))
        del self.data[key]
        return 0

class PyDeviceAddressUpdateDelegate(Controller.DeviceAddressUpdateDelegate):
    def OnAddressUpdateComplete(nodeId, error):
        print('Node ID: {}'.format(nodeId))
        print('Error: {}'.format(error))

class PyDevicePairingDelegate(Controller.DevicePairingDelegate):
    def OnPairingComplete(self, error):
        print("OnPairingComplete Error: {}".format(error))

    def OnStatusUpdate(self, status):
        print("OnStatusUpdate Status: {}".format(status))

    def OnPairingDeleted(self, error):
        print("OnPairingDeleted Error: {}".format(error))



crendential_issuer_delegate = Controller.ExampleOperationalCredentialsIssuer()

device_pairing_delegate = PyDevicePairingDelegate()
device_address_delegate = PyDeviceAddressUpdateDelegate()
storage_delegate = PyPersistentStorageDelegate()
interaction_delegate = Controller.DeviceControllerInteractionModelDelegate()
credential_init_error = crendential_issuer_delegate.Initialize(storage_delegate)
print("Credential Init Error : {}".format(credential_init_error))
ChipExceptions.CHIPErrorToException(credential_init_error)

commissioner_init_params = Controller.CommissionerInitParams()
commissioner_init_params.storageDelegate = storage_delegate
commissioner_init_params.mDeviceAddressUpdateDelegate = device_address_delegate
commissioner_init_params.pairingDelegate = device_pairing_delegate
commissioner_init_params.operationalCredentialsDelegate = crendential_issuer_delegate
commissioner_init_params.imDelegate = interaction_delegate

device_commissioner = Controller.DeviceCommissioner()
init_error = device_commissioner.Init(node_id, commissioner_init_params)
print("Init Error : {}".format(init_error))
ChipExceptions.CHIPErrorToException(init_error)


ChipExceptions.CHIPErrorToException(device_commissioner.ServiceEvents())

peer_addr_ble = Transport.PeerAddress.BLE()
rendezvous_parms = RendezvousParameters()
rendezvous_parms.SetDiscriminator(discriminator)
rendezvous_parms.SetPeerAddress(peer_addr_ble)
rendezvous_parms.SetSetupPINCode(setup_code)



pair_error = device_commissioner.PairDevice(local_node_id, rendezvous_parms)
print("Pair Error : {}".format(pair_error))
print()
ChipExceptions.CHIPErrorToException(pair_error)
