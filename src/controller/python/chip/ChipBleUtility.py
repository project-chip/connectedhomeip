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
#      This file is utility for Chip BLE
#

from __future__ import absolute_import
from __future__ import print_function
from ctypes import *
from .ChipUtility import ChipUtility


# Duplicates of BLE definitions in ChipDeviceController-ScriptBinding.cpp
BLE_EVENT_TYPE_RX = 1
BLE_EVENT_TYPE_TX = 2
BLE_EVENT_TYPE_SUBSCRIBE = 3
BLE_EVENT_TYPE_DISCONNECT = 4

BLE_SUBSCRIBE_OPERATION_SUBSCRIBE = 1
BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE = 2

# From BleError.h:
BLE_ERROR_REMOTE_DEVICE_DISCONNECTED = 12

FAKE_CONN_OBJ_VALUE = 12121212

# Number of bytes in service data payload
SERVICE_DATA_LEN = 8


def VoidPtrToUUIDString(ptr, len):
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
        ptr = str(ptr)
    except Exception as ex:
        print("ERROR: failed to convert void * to UUID")
        ptr = None

    return ptr


def ParseBleEventType(val):
    if isinstance(val, int):
        return val
    if val.lower() == "rx":
        return BLE_EVENT_TYPE_RX
    if val.lower() == "tx":
        return BLE_EVENT_TYPE_TX
    raise Exception("Invalid Ble Event Type: " + str(val))


class BleTxEvent:
    def __init__(self, svcId=None, charId=None, status=False):
        self.EventType = BLE_EVENT_TYPE_TX
        self.ConnObj = None
        self.SvcId = svcId
        self.CharId = charId
        self.Status = status

    def Print(self, prefix=""):
        print(
            "%sBleEvent Type: %s"
            % (prefix, ("TX" if self.EventType == BLE_EVENT_TYPE_TX else "ERROR"))
        )
        print("%sStatus: %s" % (prefix, str(self.Status)))

        if self.SvcId:
            print("%sSvcId:" % (prefix))
            print(ChipUtility.Hexlify(self.SvcId))

        if self.CharId:
            print("%sCharId:" % (prefix))
            print(ChipUtility.Hexlify(self.CharId))

    def SetField(self, name, val):
        name = name.lower()
        if name == "eventtype" or name == "event-type" or name == "type":
            self.EventType = ParseBleEventType(val)
        elif name == "status":
            self.Status = val
        elif name == "svcid":
            self.SvcId = val
        elif name == "charid":
            self.CharId = val
        else:
            raise Exception("Invalid BleTxEvent field: " + str(name))


class BleDisconnectEvent:
    def __init__(self, error=0):
        self.EventType = BLE_EVENT_TYPE_DISCONNECT
        self.ConnObj = None
        self.Error = error

    def Print(self, prefix=""):
        print(
            "%sBleEvent Type: %s"
            % (prefix, ("DC" if self.EventType == BLE_EVENT_TYPE_DISCONNECT else "ERROR"))
        )
        print("%sError: %s" % (prefix, str(self.Error)))

    def SetField(self, name, val):
        name = name.lower()
        if name == "eventtype" or name == "event-type" or name == "type":
            self.EventType = ParseBleEventType(val)
        elif name == "error":
            self.Error = val
        else:
            raise Exception("Invalid BleDisconnectEvent field: " + str(name))


class BleRxEvent:
    def __init__(self, svcId=None, charId=None, buffer=None):
        self.EventType = BLE_EVENT_TYPE_RX
        self.ConnObj = None
        self.SvcId = svcId
        self.CharId = charId
        self.Buffer = buffer

    def Print(self, prefix=""):
        print(
            "%sBleEvent Type: %s"
            % (prefix, ("RX" if self.EventType == BLE_EVENT_TYPE_RX else "ERROR"))
        )
        if self.Buffer:
            print("%sBuffer:" % (prefix))
            print(ChipUtility.Hexlify(self.Buffer))

        if self.SvcId:
            print("%sSvcId:" % (prefix))
            print(ChipUtility.Hexlify(self.SvcId))

        if self.CharId:
            print("%sCharId:" % (prefix))
            print(ChipUtility.Hexlify(self.CharId))

    def SetField(self, name, val):
        name = name.lower()
        if name == "eventtype" or name == "event-type" or name == "type":
            self.EventType = ParseBleEventType(val)
        elif name == "buffer":
            self.Buffer = val
        elif name == "svcid":
            self.SvcId = val
        elif name == "charid":
            self.CharId = val
        else:
            raise Exception("Invalid BleRxEvent field: " + str(name))


class BleSubscribeEvent:
    def __init__(
        self,
        svcId=None,
        charId=None,
        status=True,
        operation=BLE_SUBSCRIBE_OPERATION_SUBSCRIBE,
    ):
        self.EventType = BLE_EVENT_TYPE_SUBSCRIBE
        self.ConnObj = None
        self.SvcId = svcId
        self.CharId = charId
        self.Status = status
        self.Operation = operation

    def Print(self, prefix=""):
        print(
            "%sBleEvent Type: %s"
            % (
                prefix,
                ("SUBSCRIBE" if self.EventType ==
                 BLE_EVENT_TYPE_SUBSCRIBE else "ERROR"),
            )
        )
        print("%sStatus: %s" % (prefix, str(self.Status)))
        print(
            "%sOperation: %s"
            % (
                prefix,
                (
                    "UNSUBSCRIBE"
                    if self.Operation == BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE
                    else "SUBSCRIBE"
                ),
            )
        )

        if self.SvcId:
            print("%sSvcId:" % (prefix))
            print(ChipUtility.Hexlify(self.SvcId))

        if self.CharId:
            print("%sCharId:" % (prefix))
            print(ChipUtility.Hexlify(self.CharId))

    def SetField(self, name, val):
        name = name.lower()
        if name == "eventtype" or name == "event-type" or name == "type":
            self.EventType = ParseBleEventType(val)
        elif name == "status":
            self.Status = val
        elif name == "svcid":
            self.SvcId = val
        elif name == "charid":
            self.CharId = val
        elif name == "operation":
            self.Operation = val
        else:
            raise Exception("Invalid BleSubscribeEvent field: " + str(name))


class BleTxEventStruct(Structure):
    _fields_ = [
        ("EventType", c_int32),  # The type of event.
        # a Handle back to the connection object or None.
        ("ConnObj", c_void_p),
        ("SvcId", c_void_p),  # the byte array of the service UUID.
        ("CharId", c_void_p),  # the byte array of the characteristic UUID.
        ("Status", c_bool),  # The status of the previous Tx request
    ]

    def toBleTxEvent(self):
        return BleTxEvent(
            svcId=ChipUtility.VoidPtrToByteArray(self.SvcId, 16),
            charId=ChipUtility.VoidPtrToByteArray(self.CharId, 16),
            status=self.Status,
        )

    @classmethod
    def fromBleTxEvent(cls, bleTxEvent):
        bleTxEventStruct = cls()
        bleTxEventStruct.EventType = bleTxEvent.EventType
        bleTxEventStruct.ConnObj = c_void_p(FAKE_CONN_OBJ_VALUE)
        bleTxEventStruct.SvcId = ChipUtility.ByteArrayToVoidPtr(
            bleTxEvent.SvcId)
        bleTxEventStruct.CharId = ChipUtility.ByteArrayToVoidPtr(
            bleTxEvent.CharId)
        bleTxEventStruct.Status = bleTxEvent.Status
        return bleTxEventStruct


class BleDisconnectEventStruct(Structure):
    _fields_ = [
        ("EventType", c_int32),  # The type of event.
        # a Handle back to the connection object or None.
        ("ConnObj", c_void_p),
        ("Error", c_int32),  # The disconnect error code.
    ]

    def toBleDisconnectEvent(self):
        return BleDisconnectEvent(error=self.Error)

    @classmethod
    def fromBleDisconnectEvent(cls, bleDisconnectEvent):
        bleDisconnectEventStruct = cls()
        bleDisconnectEventStruct.EventType = bleDisconnectEvent.EventType
        bleDisconnectEventStruct.ConnObj = c_void_p(FAKE_CONN_OBJ_VALUE)
        bleDisconnectEventStruct.Error = bleDisconnectEvent.Error
        return bleDisconnectEventStruct


class BleRxEventStruct(Structure):
    _fields_ = [
        ("EventType", c_int32),  # The type of event.
        # a Handle back to the connection object or None.
        ("ConnObj", c_void_p),
        ("SvcId", c_void_p),  # the byte array of the service UUID.
        ("CharId", c_void_p),  # the byte array of the characteristic UUID.
        ("Buffer", c_void_p),  # the byte array of the Rx packet.
        ("Length", c_uint16),  # the length of the byte array (buffer).
    ]

    def toBleRxEvent(self):
        return BleRxEvent(
            svcId=ChipUtility.VoidPtrToByteArray(self.SvcId, 16),
            charId=ChipUtility.VoidPtrToByteArray(self.CharId, 16),
            buffer=ChipUtility.VoidPtrToByteArray(self.Buffer, self.Length),
        )

    @classmethod
    def fromBleRxEvent(cls, bleRxEvent):
        bleRxEventStruct = cls()
        bleRxEventStruct.EventType = bleRxEvent.EventType
        bleRxEventStruct.ConnObj = c_void_p(FAKE_CONN_OBJ_VALUE)
        bleRxEventStruct.SvcId = ChipUtility.ByteArrayToVoidPtr(
            bleRxEvent.SvcId)
        bleRxEventStruct.CharId = ChipUtility.ByteArrayToVoidPtr(
            bleRxEvent.CharId)
        bleRxEventStruct.Buffer = ChipUtility.ByteArrayToVoidPtr(
            bleRxEvent.Buffer)
        bleRxEventStruct.Length = (
            len(bleRxEvent.Buffer) if (bleRxEvent.Buffer != None) else 0
        )
        return bleRxEventStruct


class BleSubscribeEventStruct(Structure):
    _fields_ = [
        ("EventType", c_int32),  # The type of event.
        # a Handle back to the connection object or None.
        ("ConnObj", c_void_p),
        ("SvcId", c_void_p),  # the byte array of the service UUID.
        ("CharId", c_void_p),  # the byte array of the characteristic UUID.
        ("Operation", c_int32),  # The subscribe operation.
        ("Status", c_bool),  # The status of the previous Tx request
    ]

    def toBleSubscribeEvent(self):
        return BleSubscribeEvent(
            svcId=ChipUtility.VoidPtrToByteArray(self.SvcId, 16),
            charId=ChipUtility.VoidPtrToByteArray(self.CharId, 16),
            status=self.Status,
            operation=self.Operation,
        )

    @classmethod
    def fromBleSubscribeEvent(cls, bleSubscribeEvent):
        bleSubscribeEventStruct = cls()
        bleSubscribeEventStruct.EventType = bleSubscribeEvent.EventType
        bleSubscribeEventStruct.ConnObj = c_void_p(FAKE_CONN_OBJ_VALUE)
        bleSubscribeEventStruct.SvcId = ChipUtility.ByteArrayToVoidPtr(
            bleSubscribeEvent.SvcId
        )
        bleSubscribeEventStruct.CharId = ChipUtility.ByteArrayToVoidPtr(
            bleSubscribeEvent.CharId
        )
        bleSubscribeEventStruct.Operation = bleSubscribeEvent.Operation
        bleSubscribeEventStruct.Status = bleSubscribeEvent.Status
        return bleSubscribeEventStruct


class BleDeviceIdentificationInfo:
    def __init__(self, pairingState, discriminator, vendorId, productId):
        self.pairingState = pairingState
        self.discriminator = discriminator
        self.vendorId = vendorId
        self.productId = productId


def ParseServiceData(data):
    if len(data) != SERVICE_DATA_LEN:
        return None
    return BleDeviceIdentificationInfo(
        int(data[0]),
        int.from_bytes(data[1:3], byteorder='little'),
        int.from_bytes(data[3:5], byteorder='little'),
        int.from_bytes(data[5:7], byteorder='little'),
    )
