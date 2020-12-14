#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2019-2020 Google LLC.
#    Copyright (c) 2015-2018 Nest Labs, Inc.
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

#
#    @file
#      BLE Central support for Chip Device Controller via OSX CoreBluetooth APIs.
#

from __future__ import absolute_import
from __future__ import print_function
from ctypes import *
from Foundation import *

import logging
import objc
import six.moves.queue
import time

from .ChipBleUtility import (
    BLE_SUBSCRIBE_OPERATION_SUBSCRIBE,
    BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE,
    BLE_ERROR_REMOTE_DEVICE_DISCONNECTED,
    BleTxEvent,
    BleDisconnectEvent,
    BleRxEvent,
    BleSubscribeEvent,
    BleTxEventStruct,
    BleDisconnectEventStruct,
    BleRxEventStruct,
    BleSubscribeEventStruct,
    BleDeviceIdentificationInfo,
    ParseServiceData,
)

from .ChipUtility import ChipUtility
from .ChipBleBase import ChipBleBase


try:

    objc.loadBundle(
        "CoreBluetooth",
        globals(),
        bundle_path=objc.pathForFramework(
            u"/System/Library/Frameworks/IOBluetooth.framework/Versions/A/Frameworks/CoreBluetooth.framework"
        ),
    )
except Exception as ex:
    objc.loadBundle(
        "CoreBluetooth",
        globals(),
        bundle_path=objc.pathForFramework(
            u"/System/Library/Frameworks/CoreBluetooth.framework"
        ),
    )

BLE_PERIPHERAL_STATE_DISCONNECTED = 0
CBCharacteristicWriteWithResponse = 0
CBCharacteristicWriteWithoutResponse = 1

CHIP_SERVICE = CBUUID.UUIDWithString_(u"0000FEAF-0000-1000-8000-00805F9B34FB")
CHIP_SERVICE_SHORT = CBUUID.UUIDWithString_(u"FEAF")
CHIP_TX = CBUUID.UUIDWithString_(u"18EE2EF5-263D-4559-959F-4F9C429F9D11")
CHIP_RX = CBUUID.UUIDWithString_(u"18EE2EF5-263D-4559-959F-4F9C429F9D12")
CHROMECAST_SETUP_SERVICE = CBUUID.UUIDWithString_(
    u"0000FEA0-0000-1000-8000-00805F9B34FB"
)
CHROMECAST_SETUP_SERVICE_SHORT = CBUUID.UUIDWithString_(u"FEA0")


def _VoidPtrToCBUUID(ptr, len):
    try:
        ptr = ChipUtility.VoidPtrToByteArray(ptr, len)
        ptr = ChipUtility.Hexlify(ptr)
        ptr = (
            ptr[:8]
            + "-"
            + ptr[8:12]
            + "-"
            + ptr[12:16]
            + "-"
            + ptr[16:20]
            + "-"
            + ptr[20:]
        )
        ptr = CBUUID.UUIDWithString_(ptr)
    except Exception as ex:
        print("ERROR: failed to convert void * to CBUUID")
        ptr = None

    return ptr

class BlePeripheral:
    def __init__(self, peripheral, advData):
        self.peripheral = peripheral
        self.advData = dict(advData)
    
    def __eq__(self, another):
        return self.peripheral == another.peripheral

    def getPeripheralDevIdInfo(self):
        # CHIP_SERVICE_SHORT
        if not self.advData:
            return None
        servDataDict = self.advData.get("kCBAdvDataServiceData", None)
        if not servDataDict:
            return None
        servDataDict = dict(servDataDict)
        for i in servDataDict.keys():
            if str(i).lower() == str(CHIP_SERVICE_SHORT).lower():
                return ParseServiceData(bytes(servDataDict[i]))
        return None

class CoreBluetoothManager(ChipBleBase):
    def __init__(self, devCtrl, logger=None):
        if logger:
            self.logger = logger
        else:
            self.logger = logging.getLogger("ChipBLEMgr")
            logging.basicConfig(
                level=logging.INFO,
                format="%(asctime)s %(name)-12s %(levelname)-8s %(message)s",
            )
        self.manager = None
        self.peripheral = None
        self.service = None
        self.scan_quiet = False
        self.characteristics = {}
        self.peripheral_list = []
        self.peripheral_adv_list = []
        self.bg_peripheral_name = None
        self.chip_queue = six.moves.queue.Queue()

        self.manager = CBCentralManager.alloc()
        self.manager.initWithDelegate_queue_options_(self, None, None)

        self.ready_condition = False
        self.loop_condition = (
            False  # indicates whether the cmd requirement has been met in the runloop.
        )
        self.connect_state = False  # reflects whether or not there is a connection.
        self.send_condition = False
        self.subscribe_condition = False

        self.runLoopUntil(("ready", time.time(), 10.0))

        self.orig_input_hook = None
        self.hookFuncPtr = None

        self.setInputHook(self.readlineCB)
        self.devCtrl = devCtrl
        self.devCtrl.SetBlockingCB(self.devMgrCB)

        def HandleBleEventCB():
            return self.GetBleEvent()

        def HandleBleWriteCharCB(connObj, svcId, charId, buffer, length):
            return self.WriteBleCharacteristic(connObj, svcId, charId, buffer, length)

        def HandleBleSubscribeCB(connObj, svcId, charId, subscribe):
            return self.SubscribeBleCharacteristic(connObj, svcId, charId, subscribe)

        def HandleBleCloseCB(connObj):
            return self.CloseBle(connObj)

        self.devCtrl.SetBleEventCB(HandleBleEventCB)
        self.devCtrl.SetBleWriteCharCB(HandleBleWriteCharCB)
        self.devCtrl.SetBleSubscribeCharCB(HandleBleSubscribeCB)
        self.devCtrl.SetBleCloseCB(HandleBleCloseCB)

        # test if any connections currently exist (left around from a previous run) and disconnect if need be.
        peripherals = self.manager.retrieveConnectedPeripheralsWithServices_(
            [CHIP_SERVICE_SHORT, CHIP_SERVICE]
        )

        if peripherals and len(peripherals):
            for periph in peripherals:
                self.logger.info("disconnecting old connection.")
                self.loop_condition = False
                self.manager.cancelPeripheralConnection_(periph)
                self.runLoopUntil(("disconnect", time.time(), 5.0))

            self.connect_state = False
            self.loop_condition = False

    def __del__(self):
        self.disconnect()
        self.setInputHook(self.orig_input_hook)
        self.devCtrl.SetBlockingCB(None)
        self.devCtrl.SetBleEventCB(None)

    def devMgrCB(self):
        """A callback used by ChipDeviceCtrl.py to drive the OSX runloop while the
        main thread waits for the Chip thread to complete its operation."""
        runLoop = NSRunLoop.currentRunLoop()
        runLoop.limitDateForMode_(NSDefaultRunLoopMode)

    def readlineCB(self):
        """A callback used by readline to drive the OSX runloop while the main thread
        waits for commandline input from the user."""
        runLoop = NSRunLoop.currentRunLoop()
        runLoop.limitDateForMode_(NSDefaultRunLoopMode)

        if self.orig_input_hook:
            self.orig_input_hook()

    def setInputHook(self, hookFunc):
        """Set the PyOS_InputHook to call the specific function."""
        hookFunctionType = CFUNCTYPE(None)
        self.hookFuncPtr = hookFunctionType(hookFunc)
        pyos_inputhook_ptr = c_void_p.in_dll(pythonapi, "PyOS_InputHook")
        # save the original so that on del we can revert it back to the way it was.
        self.orig_input_hook = cast(pyos_inputhook_ptr.value, PYFUNCTYPE(c_int))
        # set the new hook. readLine will call this periodically as it polls for input.
        pyos_inputhook_ptr.value = cast(self.hookFuncPtr, c_void_p).value

    def shouldLoop(self, should_tuple):
        """ Used by runLoopUntil to determine whether it should exit the runloop."""
        result = False

        time_expired = time.time() >= should_tuple[1] + should_tuple[2]
        if time_expired:
            return False

        if should_tuple[0] == "ready":
            return not self.ready_condition
        elif should_tuple[0] == "scan":
            for peripheral in self.peripheral_adv_list:
                if should_tuple[3] and str(peripheral.peripheral._.name) == should_tuple[3]:
                    return False
                devIdInfo = peripheral.getPeripheralDevIdInfo()
                if devIdInfo and should_tuple[3] and str(devIdInfo.discriminator) == should_tuple[3]:
                    return False
        elif should_tuple[0] == "connect":
            return (not self.loop_condition)
        elif should_tuple[0] == "disconnect":
            return (not self.loop_condition)
        elif should_tuple[0] == "send":
            return (not self.send_condition)
        elif should_tuple[0] == "subscribe":
            return (not self.subscribe_condition)
        elif should_tuple[0] == "unsubscribe":
            return self.subscribe_condition

        return False

    def runLoopUntil(self, should_tuple):
        """Helper function to drive OSX runloop until an expected event is received or
        the timeout expires."""
        runLoop = NSRunLoop.currentRunLoop()
        nextfire = 1

        while nextfire and self.shouldLoop(should_tuple):
            nextfire = runLoop.limitDateForMode_(NSDefaultRunLoopMode)

    def centralManagerDidUpdateState_(self, manager):
        """ IO Bluetooth initialization is successful."""

        state = manager.state()
        string = "BLE is ready!" if state > 4 else "BLE is not ready!"
        self.logger.info(string)
        self.manager = manager
        self.ready_condition = True if state > 4 else False

    def centralManager_didDiscoverPeripheral_advertisementData_RSSI_(
        self, manager, peripheral, data, rssi
    ):
        """ Called for each peripheral discovered during scan."""
        if self.bg_peripheral_name is None:
            if peripheral not in self.peripheral_list:
                if not self.scan_quiet:
                    self.logger.info("adding to scan list:")
                    self.logger.info("")
                    self.logger.info(
                        "{0:<16}= {1:<80}".format("Name", str(peripheral._.name))
                    )
                    self.logger.info(
                        "{0:<16}= {1:<80}".format(
                            "ID", str(peripheral._.identifier.UUIDString())
                        )
                    )
                    self.logger.info("{0:<16}= {1:<80}".format("RSSI", rssi))
                    devIdInfo = BlePeripheral(peripheral, data).getPeripheralDevIdInfo()
                    if devIdInfo:
                        self.logger.info("{0:<16}= {1}".format("Pairing State", devIdInfo.pairingState))
                        self.logger.info("{0:<16}= {1}".format("Discriminator", devIdInfo.discriminator))
                        self.logger.info("{0:<16}= {1}".format("Vendor Id", devIdInfo.vendorId))
                        self.logger.info("{0:<16}= {1}".format("Product Id", devIdInfo.productId))
                    self.logger.info("ADV data: " + repr(data))
                    self.logger.info("")

                self.peripheral_list.append(peripheral)
                self.peripheral_adv_list.append(BlePeripheral(peripheral, data))
        else:
            if (peripheral._.name == self.bg_peripheral_name) or (str(devIdInfo.discriminator) == self.bg_peripheral_name):
                if len(self.peripheral_list) == 0:
                    self.logger.info("found background peripheral")
                self.peripheral_list = [peripheral]
                self.peripheral_adv_list = [BlePeripheral(peripheral, data)]

    def centralManager_didConnectPeripheral_(self, manager, peripheral):
        """Called by CoreBluetooth via runloop when a connection succeeds."""
        self.logger.debug(repr(peripheral))
        # make this class the delegate for peripheral events.
        self.peripheral.setDelegate_(self)
        # invoke service discovery on the periph.
        self.logger.info("Discovering services")
        self.peripheral.discoverServices_([CHIP_SERVICE_SHORT, CHIP_SERVICE])

    def centralManager_didFailToConnectPeripheral_error_(
        self, manager, peripheral, error
    ):
        """Called by CoreBluetooth via runloop when a connection fails."""
        self.logger.info("Failed to connect error = " + repr(error))
        self.loop_condition = True
        self.connect_state = False

    def centralManager_didDisconnectPeripheral_error_(self, manager, peripheral, error):
        """Called by CoreBluetooth via runloop when a disconnect completes. error = None on success."""
        self.loop_condition = True
        self.connect_state = False
        if self.devCtrl:
            self.logger.info("BLE disconnected, error = " + repr(error))
            dcEvent = BleDisconnectEvent(BLE_ERROR_REMOTE_DEVICE_DISCONNECTED)
            self.chip_queue.put(dcEvent)
            self.devCtrl.DriveBleIO()

    def peripheral_didDiscoverServices_(self, peripheral, services):
        """Called by CoreBluetooth via runloop when peripheral services are discovered."""
        if len(self.peripheral.services()) == 0:
            self.logger.error("Chip service not found")
            self.connect_state = False
        else:
            # in debugging, we found connect being called twice. This
            # would trigger discovering the services twice, and
            # consequently, discovering characteristics twice.  We use the
            # self.service as a flag to indicate whether the
            # characteristics need to be invalidated immediately.
            if self.service == self.peripheral.services()[0]:
                self.logger.debug("didDiscoverServices already happened")
            else:
                self.service = self.peripheral.services()[0]
                self.characteristics[self.service.UUID()] = []
            # NOTE: currently limiting discovery to only the pair of Chip characteristics.
            self.peripheral.discoverCharacteristics_forService_(
                [CHIP_RX, CHIP_TX], self.service
            )

    def peripheral_didDiscoverCharacteristicsForService_error_(
        self, peripheral, service, error
    ):
        """Called by CoreBluetooth via runloop when a characteristic for a service is discovered."""
        self.logger.debug(
            "didDiscoverCharacteristicsForService:error "
            + str(repr(peripheral))
            + " "
            + str(repr(service))
        )
        self.logger.debug(repr(service))
        self.logger.debug(repr(error))

        if not error:
            self.characteristics[service.UUID()] = [
                char for char in self.service.characteristics()
            ]

            self.connect_state = True

        else:
            self.logger.error("ERROR: failed to discover characteristics for service.")
            self.connect_state = False

        self.loop_condition = True

    def peripheral_didWriteValueForCharacteristic_error_(
        self, peripheral, characteristic, error
    ):
        """Called by CoreBluetooth via runloop when a write to characteristic
        operation completes. error = None on success."""
        self.logger.debug("didWriteValue error = " + repr(error))
        self.send_condition = True
        charId = bytearray(characteristic.UUID().data().bytes().tobytes())
        svcId = bytearray(CHIP_SERVICE.data().bytes().tobytes())

        if self.devCtrl:
            txEvent = BleTxEvent(
                charId=charId, svcId=svcId, status=True if not error else False
            )
            self.chip_queue.put(txEvent)
            self.devCtrl.DriveBleIO()

    def peripheral_didUpdateNotificationStateForCharacteristic_error_(
        self, peripheral, characteristic, error
    ):
        """Called by CoreBluetooth via runloop when a subscribe for notification operation completes.
        Error = None on success."""
        self.logger.debug("Receiving notifications")
        charId = bytearray(characteristic.UUID().data().bytes().tobytes())
        svcId = bytearray(CHIP_SERVICE.data().bytes().tobytes())
        # look at error and send True/False on Success/Failure
        success = True if not error else False
        if characteristic.isNotifying():
            operation = BLE_SUBSCRIBE_OPERATION_SUBSCRIBE
            self.subscribe_condition = True
        else:
            operation = BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE
            self.subscribe_condition = False

        self.logger.debug("Operation = " + repr(operation))
        self.logger.debug("success = " + repr(success))

        if self.devCtrl:
            subscribeEvent = BleSubscribeEvent(
                charId=charId, svcId=svcId, status=success, operation=operation
            )
            self.chip_queue.put(subscribeEvent)
            self.devCtrl.DriveBleIO()

    def peripheral_didUpdateValueForCharacteristic_error_(
        self, peripheral, characteristic, error
    ):
        """Called by CoreBluetooth via runloop when a new characteristic value is received for a
        characteristic to which this device has subscribed."""
        # len = characteristic.value().length()
        bytes = bytearray(characteristic.value().bytes().tobytes())
        charId = bytearray(characteristic.UUID().data().bytes().tobytes())
        svcId = bytearray(CHIP_SERVICE.data().bytes().tobytes())

        # Kick Chip thread to retrieve the saved packet.
        if self.devCtrl:
            # Save buffer, length, service UUID and characteristic UUID
            rxEvent = BleRxEvent(charId=charId, svcId=svcId, buffer=bytes)
            self.chip_queue.put(rxEvent)
            self.devCtrl.DriveBleIO()

        self.logger.debug("received")
        self.logger.debug(
            "received ("
            + str(len)
            + ") bytes: "
            + repr(characteristic.value().bytes().tobytes())
        )

    def GetBleEvent(self):
        """ Called by ChipDeviceMgr.py on behalf of Chip to retrieve a queued message."""
        if not self.chip_queue.empty():
            ev = self.chip_queue.get()

            if isinstance(ev, BleRxEvent):
                eventStruct = BleRxEventStruct.fromBleRxEvent(ev)
                return cast(pointer(eventStruct), c_void_p).value
            elif isinstance(ev, BleTxEvent):
                eventStruct = BleTxEventStruct.fromBleTxEvent(ev)
                return cast(pointer(eventStruct), c_void_p).value
            elif isinstance(ev, BleSubscribeEvent):
                eventStruct = BleSubscribeEventStruct.fromBleSubscribeEvent(ev)
                return cast(pointer(eventStruct), c_void_p).value
            elif isinstance(ev, BleDisconnectEvent):
                eventStruct = BleDisconnectEventStruct.fromBleDisconnectEvent(ev)
                return cast(pointer(eventStruct), c_void_p).value

        return None

    def scan(self, line):
        """ API to initiatae BLE scanning for -t user_timeout seconds."""

        args = self.ParseInputLine(line, "scan")

        if not args:
            return
        self.scan_quiet = args[1]
        self.bg_peripheral_name = None
        del self.peripheral_list[:]
        del self.peripheral_adv_list[:]
        self.peripheral_list = []
        self.peripheral_adv_list = []
        # Filter on the service UUID Array or None to accept all scan results.
        self.manager.scanForPeripheralsWithServices_options_(
            [
                CHIP_SERVICE_SHORT,
                CHIP_SERVICE,
                CHROMECAST_SETUP_SERVICE_SHORT,
                CHROMECAST_SETUP_SERVICE,
            ],
            None,
        )
        # self.manager.scanForPeripheralsWithServices_options_(None, None)

        self.runLoopUntil(("scan", time.time(), args[0], args[2]))

        self.manager.stopScan()
        self.logger.info("scanning stopped")

    def bgScanStart(self, name):
        """ API to initiate background BLE scanning."""
        self.logger.info("scanning started")
        self.bg_peripheral_name = name
        del self.peripheral_list[:]
        self.peripheral_list = []
        # Filter on the service UUID Array or None to accept all scan results.
        self.manager.scanForPeripheralsWithServices_options_(
            [
                CHIP_SERVICE_SHORT,
                CHIP_SERVICE,
                CHROMECAST_SETUP_SERVICE_SHORT,
                CHROMECAST_SETUP_SERVICE,
            ],
            None,
        )

    def bgScanStop(self):
        """ API to stop background BLE scanning."""
        self.manager.stopScan()
        self.bg_peripheral_name = None
        self.logger.info("scanning stopped")

    def connect(self, identifier):
        """ API to initiate BLE connection to peripheral device whose identifier == identifier."""
        self.logger.info("trying to connect to " + identifier)

        if self.connect_state:
            self.logger.error("ERROR: Connection to a BLE device already exists!")
        else:
            for peripheral in self.peripheral_adv_list:
                p = peripheral.peripheral
                devIdInfo = peripheral.getPeripheralDevIdInfo()
                if not devIdInfo:
                    # Not a chip device
                    continue
                p_id = str(p.identifier().UUIDString())
                p_name = str(p.name())

                self.logger.debug(p_id + " vs " + str(identifier))
                self.logger.debug(p_name + " vs " + str(identifier))

                if p_id == str(identifier) or p_name == str(identifier) or str(devIdInfo.discriminator) == str(identifier):
                    self.loop_condition = False
                    self.peripheral = p
                    self.manager.connectPeripheral_options_(p, None)

                    self.runLoopUntil(("connect", time.time(), 15.0))
                    # Cleanup when the connect fails due to timeout,
                    # otherwise CoreBluetooth will continue to try to connect after this
                    # API exits.
                    if not self.connect_state:
                        self.manager.cancelPeripheralConnection_(p)
                        self.peripheral = None

                    break

        ret = True if self.loop_condition and self.connect_state else False

        resString = "connect " + ("success" if ret else "fail")
        self.logger.info(resString)

        return ret

    def disconnect(self):
        """ API to initiate BLE disconnect procedure."""
        self.logger.info("disconnecting")

        if (
            self.peripheral
            and self.peripheral.state() != BLE_PERIPHERAL_STATE_DISCONNECTED
        ):
            self.loop_condition = False
            self.manager.cancelPeripheralConnection_(self.peripheral)

            self.runLoopUntil(("disconnect", time.time(), 10.0))

        resString = "disconnect " + (
            "success" if self.loop_condition and not self.connect_state else "fail"
        )
        self.logger.info(resString)

        self.characteristics = {}
        # del self.peripheral_list[:]
        # self.peripheral_list = []
        self.peripheral = None
        self.service = None

    def scan_connect(self, line):
        """ API to perform both scan and connect operations in one call."""

        args = self.ParseInputLine(line, "scan-connect")

        if not args:
            return

        self.scan_quiet = args[1]
        self.scan(line)

        if len(self.peripheral_list):
            return self.connect(args[2])
        else:
            self.logger.info(
                "Failed to scan device named: " + args[2] + ". Connection skipped."
            )
            return False

    def isConnected(self):
        if (
            self.peripheral
            and self.peripheral.state() != BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
        ):
            return True

        return False

    def WriteBleCharacteristic(self, connObj, svcId, charId, buffer, length):
        """ Called by ChipDeviceMgr.py to satisfy a request by Chip to transmit a packet over BLE."""
        result = False

        bytes = ChipUtility.VoidPtrToByteArray(buffer, length)
        bytes = NSData.dataWithBytes_length_(
            bytes, len(bytes)
        )  # convert bytearray to NSData

        svcId = _VoidPtrToCBUUID(svcId, 16)
        charId = _VoidPtrToCBUUID(charId, 16)

        if (
            self.peripheral
            and self.peripheral.state() != BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
        ):
            for char in self.characteristics[svcId]:
                if char.UUID() == charId:
                    self.peripheral.writeValue_forCharacteristic_type_(
                        bytes, char, CBCharacteristicWriteWithResponse
                    )
                    result = True
                    break
        else:
            self.logger.warning("WARNING: peripheral is no longer connected.")

        return result

    def SubscribeBleCharacteristic(self, connObj, svcId, charId, subscribe):
        """ Called by Chip to (un-)subscribe to a characteristic of a service."""
        result = False

        svcId = _VoidPtrToCBUUID(svcId, 16)
        charId = _VoidPtrToCBUUID(charId, 16)

        if (
            self.peripheral
            and self.peripheral.state() != BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
        ):
            for char in self.characteristics[svcId]:
                if char.UUID() == charId:
                    self.peripheral.setNotifyValue_forCharacteristic_(
                        True if subscribe else False, char
                    )
                    result = True
                    break
        else:
            self.logger.warning("WARNING: peripheral is no longer connected.")

        return result

    def ble_debug_log(self, line):
        args = self.ParseInputLine(line)
        if int(args[0]) == 1:
            self.logger.setLevel(logging.DEBUG)
            self.logger.debug("current logging level is debug")
        else:
            self.logger.setLevel(logging.INFO)
            self.logger.info("current logging level is info")
        return True

    def CloseBle(self, connObj):
        """ Called by Chip to close the BLE connection."""
        if self.peripheral:
            self.manager.cancelPeripheralConnection_(self.peripheral)
            self.characteristics = {}
            # del self.peripheral_list[:]
            # self.peripheral_list = []
            self.peripheral = None
            self.service = None
            self.connect_state = False

        return True

    def updateCharacteristic(self, bytes, svcId, charId):
        # TODO: implement this for Peripheral support.
        return False
