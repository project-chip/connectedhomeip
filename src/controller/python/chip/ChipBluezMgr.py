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
#      BLE Central support for Chip Device Manager via BlueZ APIs.
#

from __future__ import absolute_import
from __future__ import print_function
import dbus
import dbus.service
import dbus.mainloop.glib
import gc
import logging
import pprint
import sys
import threading
import time
import traceback
import uuid
import six.moves.queue


from ctypes import *
import six
from six.moves import range

try:
    from gi.repository import GObject
except Exception as ex:
    logging.exception("Unable to find GObject from gi.repository")
    from pgi.repository import GObject

from .ChipUtility import ChipUtility

from .ChipBleUtility import (
    BLE_SUBSCRIBE_OPERATION_SUBSCRIBE,
    BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE,
    BLE_ERROR_REMOTE_DEVICE_DISCONNECTED,
    VoidPtrToUUIDString,
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

from .ChipBleBase import ChipBleBase

chip_service = uuid.UUID("0000FEAF-0000-1000-8000-00805F9B34FB")
chip_tx = uuid.UUID("18EE2EF5-263D-4559-959F-4F9C429F9D11")
chip_rx = uuid.UUID("18EE2EF5-263D-4559-959F-4F9C429F9D12")
chip_service_short = uuid.UUID("0000FEAF-0000-0000-0000-000000000000")
chromecast_setup_service = uuid.UUID("0000FEA0-0000-1000-8000-00805F9B34FB")
chromecast_setup_service_short = uuid.UUID("0000FEA0-0000-0000-0000-000000000000")

BLUEZ_NAME = "org.bluez"
ADAPTER_INTERFACE = BLUEZ_NAME + ".Adapter1"
DEVICE_INTERFACE = BLUEZ_NAME + ".Device1"
SERVICE_INTERFACE = BLUEZ_NAME + ".GattService1"
CHARACTERISTIC_INTERFACE = BLUEZ_NAME + ".GattCharacteristic1"
DBUS_PROPERTIES = "org.freedesktop.DBus.Properties"

BLE_SCAN_CONNECT_GUARD_SEC = 2.0
BLE_STATUS_TRANSITION_TIMEOUT_SEC = 5.0
BLE_CONNECT_TIMEOUT_SEC = 15.0
BLE_SERVICE_DISCOVERY_TIMEOUT_SEC = 5.0
BLE_CHAR_DISCOVERY_TIMEOUT_SEC = 5.0
BLE_SUBSCRIBE_TIMEOUT_SEC = 5.0
BLE_WRITE_CHARACTERISTIC_TIMEOUT_SEC = 10.0
BLE_IDLE_DELTA = 0.1


def get_bluez_objects(bluez, bus, interface, prefix_path):
    results = []
    if bluez is None or bus is None or interface is None or prefix_path is None:
        return results
    for item in six.iteritems(bluez.GetManagedObjects()):
        delegates = item[1].get(interface)
        if not delegates:
            continue
        slice = {}
        if item[0].startswith(prefix_path):
            slice["object"] = bus.get_object(BLUEZ_NAME, item[0])
            slice["path"] = item[0]
            results.append(slice)
    return results


class BluezDbusAdapter:
    def __init__(self, bluez_obj, bluez, bus, logger=None):
        self.logger = logger if logger else logging.getLogger("ChipBLEMgr")
        self.object = bluez_obj
        self.adapter = dbus.Interface(bluez_obj, ADAPTER_INTERFACE)
        self.adapter_properties = dbus.Interface(bluez_obj, DBUS_PROPERTIES)
        self.adapter_event = threading.Event()
        self.bluez = bluez
        self.bus = bus
        self.path = self.adapter.object_path
        self.signalReceiver = None

    def __del__(self):
        self.destroy()

    def destroy(self):
        self.logger.debug("destroy adapter")
        self.adapter_unregister_signal()
        self.adapter = None
        self.adapter_properties = None
        self.adapter_event.clear()
        self.bluez = None
        self.bus = None
        self.object = None
        self.path = None
        self.signalReceiver = None

    def adapter_register_signal(self):
        if self.signalReceiver is None:
            self.logger.debug("add adapter signal")
            self.signalReceiver = self.bus.add_signal_receiver(
                self.adapter_on_prop_changed_cb,
                bus_name=BLUEZ_NAME,
                dbus_interface=DBUS_PROPERTIES,
                signal_name="PropertiesChanged",
                path=self.path,
            )

    def adapter_unregister_signal(self):
        if self.signalReceiver is not None:
            self.logger.debug(" remove adapter signal")
            self.bus.remove_signal_receiver(
                self.signalReceiver,
                signal_name="PropertiesChanged",
                dbus_interface="org.freedesktop.DBus.Properties",
            )

    def adapter_on_prop_changed_cb(
        self, interface, changed_properties, invalidated_properties
    ):
        if len(changed_properties) == 0:
            self.logger.debug("changed_properties is empty")
            return

        if len(invalidated_properties) > 0:
            self.logger.debug(
                "invalidated_properties is not empty %s" % str(invalidated_properties)
            )
            return

        if interface == ADAPTER_INTERFACE:
            if "Discovering" in changed_properties:
                self.adapter_event.set()

    def adapter_bg_scan(self, enable):
        self.adapter_event.clear()
        action_flag = False
        try:
            if enable:
                if not self.Discovering:
                    action_flag = True
                    self.logger.info("scanning started")
                    self.adapter.StartDiscovery()
                else:
                    self.logger.info("it has started scanning")
            else:
                if self.Discovering:
                    action_flag = True
                    self.adapter.StopDiscovery()
                    self.logger.info("scanning stopped")
                else:
                    print("it has stopped scanning")
            if action_flag:
                if not self.adapter_event.wait(BLE_STATUS_TRANSITION_TIMEOUT_SEC):
                    if enable:
                        self.logger.debug("scan start error")
                    else:
                        self.logger.debug("scan stop error")
            self.adapter_event.clear()
        except dbus.exceptions.DBusException as ex:
            self.adapter_event.clear()
            self.logger.debug(str(ex))
        except Exception as ex:
            self.logger.debug(traceback.format_exc())

    @property
    def Address(self):
        try:
            result = self.adapter_properties.Get(ADAPTER_INTERFACE, "Address")
            return result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def UUIDs(self):
        try:
            return self.adapter_properties.Get(ADAPTER_INTERFACE, "UUIDs")
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    def SetDiscoveryFilter(self, dict):
        try:
            self.adapter.SetDiscoveryFilter(dict)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
        except Exception as ex:
            self.logger.debug(traceback.format_exc())

    @property
    def Discovering(self):
        try:
            result = self.adapter_properties.Get(ADAPTER_INTERFACE, "Discovering")
            return bool(result)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False

    def DiscoverableTimeout(self, timeoutSec):
        try:
            result = self.adapter_properties.Set(
                ADAPTER_INTERFACE, "DiscoverableTimeout", timeoutSec
            )
            return bool(result)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False

    def Powered(self, enable):
        try:
            result = self.adapter_properties.Set(ADAPTER_INTERFACE, "Powered", enable)
            return bool(result)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False

    def find_devices(self, uuids):
        devices = [
            BluezDbusDevice(p["object"], self.bluez, self.bus, self.logger)
            for p in get_bluez_objects(
                self.bluez, self.bus, DEVICE_INTERFACE, self.path
            )
        ]
        found = []
        for device in devices:
            for i in device.uuids:
                if i in uuids:
                    found.append(device)
                    break
            # Some devices do not advertise their uuid lists, thus we should also check service data.
            if device.ServiceData:
                for i in device.ServiceData:
                    if uuid.UUID(str(i)) in uuids:
                        found.append(device)
                        break
        return found

    def clear_adapter(self):
        devices = [
            BluezDbusDevice(p["object"], self.bluez, self.bus, self.logger)
            for p in get_bluez_objects(
                self.bluez, self.bus, DEVICE_INTERFACE, self.path
            )
        ]
        for device in devices:
            try:
                if device.Connected:
                    device.device_bg_connect(False)
                self.adapter.RemoveDevice(device.device.object_path)
            except Exception as ex:
                pass


class BluezDbusDevice:
    def __init__(self, bluez_obj, bluez, bus, logger=None):
        self.logger = logger if logger else logging.getLogger("ChipBLEMgr")
        self.object = bluez_obj
        self.device = dbus.Interface(bluez_obj, DEVICE_INTERFACE)
        self.device_properties = dbus.Interface(bluez_obj, DBUS_PROPERTIES)
        self.path = self.device.object_path
        self.device_event = threading.Event()
        if self.Name:
            try:
                self.device_id = uuid.uuid3(uuid.NAMESPACE_DNS, self.Name)
            except UnicodeDecodeError:
                self.device_id = uuid.uuid3(
                    uuid.NAMESPACE_DNS, self.Name.encode("utf-8")
                )
        else:
            self.device_id = uuid.uuid4()
        self.bluez = bluez
        self.bus = bus
        self.signalReceiver = None
        self.path = self.device.object_path

    def __del__(self):
        self.destroy()

    def destroy(self):
        self.logger.debug("destroy device")
        self.device_unregister_signal()
        self.device = None
        self.device_properties = None
        self.device_event = None
        self.device_id = None
        self.bluez = None
        self.bus = None
        self.object = None
        self.signalReceiver = None

    def device_register_signal(self):
        if self.signalReceiver is None:
            self.logger.debug("add device signal")
            self.signalReceiver = self.bus.add_signal_receiver(
                self.device_on_prop_changed_cb,
                bus_name=BLUEZ_NAME,
                dbus_interface=DBUS_PROPERTIES,
                signal_name="PropertiesChanged",
                path=self.path,
            )

    def device_unregister_signal(self):
        if self.signalReceiver is not None:
            self.logger.debug("remove device signal")
            self.bus.remove_signal_receiver(
                self.signalReceiver,
                signal_name="PropertiesChanged",
                dbus_interface=DBUS_PROPERTIES,
            )

    def device_on_prop_changed_cb(
        self, interface, changed_properties, invalidated_properties
    ):
        if len(changed_properties) == 0:
            self.logger.debug("changed_properties is empty")
            return

        if len(invalidated_properties) > 0:
            self.logger.debug(
                "invalidated_properties is not empty %s" % str(invalidated_properties)
            )
            return

        if interface == DEVICE_INTERFACE:
            if "Connected" in changed_properties:
                self.device_event.set()

    def device_bg_connect(self, enable):
        time.sleep(BLE_SCAN_CONNECT_GUARD_SEC)
        action_flag = False
        self.device_event.clear()
        try:
            if enable:
                if not self.Connected:
                    action_flag = True
                    self.device.Connect()
                    self.logger.info("BLE connecting")
                else:
                    self.logger.info("BLE has connected")
            else:
                if self.Connected:
                    action_flag = True
                    self.device.Disconnect()
                    self.logger.info("BLE disconnected")
                else:
                    self.logger.info("BLE has disconnected")
            if action_flag:
                if not self.device_event.wait(BLE_STATUS_TRANSITION_TIMEOUT_SEC):
                    if enable:
                        self.logger.info("BLE connect error")
                    else:
                        self.logger.info("BLE disconnect error")
            self.device_event.clear()
        except dbus.exceptions.DBusException as ex:
            self.device_event.clear()
            self.logger.info(str(ex))
        except Exception as ex:
            self.logger.debug(traceback.format_exc())

    def service_discover(self, gatt_dic):
        self.logger.info("Discovering services")
        try:
            expired = time.time() + BLE_SERVICE_DISCOVERY_TIMEOUT_SEC
            while time.time() < expired:
                if self.ServicesResolved:
                    services = [
                        BluezDbusGattService(
                            p["object"], self.bluez, self.bus, self.logger
                        )
                        for p in get_bluez_objects(
                            self.bluez, self.bus, SERVICE_INTERFACE, self.path
                        )
                    ]
                    for service in services:
                        if service.uuid in gatt_dic["services"]:
                            self.logger.info("Service discovering success")
                            return service
                time.sleep(BLE_IDLE_DELTA)
            self.logger.error("Service discovering fail")
            return None
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def uuids(self):
        try:
            uuids = self.device_properties.Get(DEVICE_INTERFACE, "UUIDs")
            uuid_result = []
            for i in uuids:
                if len(str(i)) == 4:
                    uuid_normal = "0000%s-0000-0000-0000-000000000000" % i
                else:
                    uuid_normal = i
                uuid_result.append(uuid.UUID(str(uuid_normal)))
            return uuid_result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def Address(self):
        try:
            return self.device_properties.Get(DEVICE_INTERFACE, "Address")
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def Name(self):
        try:
            name = self.device_properties.Get(DEVICE_INTERFACE, "Name")
            return name
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def Connected(self):
        try:
            result = self.device_properties.Get(DEVICE_INTERFACE, "Connected")
            return bool(result)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False

    @property
    def TxPower(self):
        try:
            return self.device_properties.Get(DEVICE_INTERFACE, "TxPower")
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def RSSI(self):
        try:
            result = self.device_properties.Get(DEVICE_INTERFACE, "RSSI")
            return result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def Adapter(self):
        try:
            return self.device_properties.Get(DEVICE_INTERFACE, "Adapter")
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def ServiceData(self):
        try:
            return self.device_properties.Get(DEVICE_INTERFACE, "ServiceData")
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def ServicesResolved(self):
        try:
            result = self.device_properties.Get(DEVICE_INTERFACE, "ServicesResolved")
            return bool(result)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False


class BluezDbusGattService:
    def __init__(self, bluez_obj, bluez, bus, logger=None):
        self.logger = logger if logger else logging.getLogger("ChipBLEMgr")
        self.object = bluez_obj
        self.service = dbus.Interface(bluez_obj, SERVICE_INTERFACE)
        self.service_properties = dbus.Interface(bluez_obj, DBUS_PROPERTIES)
        self.bluez = bluez
        self.bus = bus
        self.path = self.service.object_path

    def __del__(self):
        self.destroy()

    def destroy(self):
        self.logger.debug("destroy GattService")
        self.service = None
        self.service_properties = None
        self.bluez = None
        self.bus = None
        self.object = None
        self.path = None

    @property
    def uuid(self):
        try:
            result = uuid.UUID(
                str(self.service_properties.Get(SERVICE_INTERFACE, "UUID"))
            )
            return result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    @property
    def Primary(self):
        try:
            result = bool(self.service_properties.Get(SERVICE_INTERFACE, "Primary"))
            return result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False

    @property
    def Device(self):
        try:
            result = self.service_properties.Get(SERVICE_INTERFACE, "Device")
            return result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    def find_characteristic(self, uuid):
        try:
            expired = time.time() + BLE_CHAR_DISCOVERY_TIMEOUT_SEC
            while time.time() < expired:
                characteristics = [
                    BluezDbusGattCharacteristic(
                        p["object"], self.bluez, self.bus, self.logger
                    )
                    for p in get_bluez_objects(
                        self.bluez, self.bus, CHARACTERISTIC_INTERFACE, self.path
                    )
                ]
                for characteristic in characteristics:
                    if characteristic.uuid == uuid:
                        return characteristic
                time.sleep(BLE_IDLE_DELTA)
            self.logger.error("Char discovering fail")
            return None
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None


class BluezDbusGattCharacteristic:
    def __init__(self, bluez_obj, bluez, bus, logger=None):
        self.logger = logger if logger else logging.getLogger("ChipBLEMgr")
        self.object = bluez_obj
        self.characteristic = dbus.Interface(bluez_obj, CHARACTERISTIC_INTERFACE)
        self.characteristic_properties = dbus.Interface(bluez_obj, DBUS_PROPERTIES)
        self.received = None
        self.path = self.characteristic.object_path
        self.bluez = bluez
        self.bus = bus
        self.signalReceiver = None

    def __del__(self):
        self.destroy()

    def destroy(self):
        self.logger.debug("destroy GattCharacteristic")
        self.gattCharacteristic_unregister_signal()
        self.characteristic = None
        self.object = None
        self.characteristic_properties = None
        self.received = None
        self.bluez = None
        self.bus = None
        self.path = None
        self.signalReceiver = None

    def gattCharacteristic_register_signal(self):
        if not self.signalReceiver:
            self.logger.debug("add GattCharacteristic signal")
            self.signalReceiver = self.bus.add_signal_receiver(
                self.gatt_on_characteristic_changed_cb,
                bus_name=BLUEZ_NAME,
                dbus_interface=DBUS_PROPERTIES,
                signal_name="PropertiesChanged",
                path=self.path,
            )

    def gattCharacteristic_unregister_signal(self):
        if self.signalReceiver:
            self.logger.debug("remove GattCharacteristic signal")

            self.bus.remove_signal_receiver(
                self.signalReceiver,
                bus_name=BLUEZ_NAME,
                signal_name="PropertiesChanged",
                dbus_interface=DBUS_PROPERTIES,
                path=self.path,
            )
            self.signalReceiver = None

    def gatt_on_characteristic_changed_cb(
        self, interface, changed_properties, invalidated_properties
    ):
        self.logger.debug(
            "property change in" + str(self.characteristic) + str(changed_properties)
        )

        if len(changed_properties) == 0:
            return

        if len(invalidated_properties) > 0:
            return

        if interface == CHARACTERISTIC_INTERFACE:
            if "Value" in changed_properties:
                if self.received:
                    self.received(changed_properties["Value"])

    def WriteValue(self, value, options, reply_handler, error_handler, timeout):
        try:
            self.characteristic.WriteValue(
                value,
                options,
                reply_handler=reply_handler,
                error_handler=error_handler,
                timeout=timeout,
            )
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
        except Exception as ex:
            self.logger.debug(traceback.format_exc())

    @property
    def uuid(self):
        try:
            result = uuid.UUID(
                str(
                    self.characteristic_properties.Get(CHARACTERISTIC_INTERFACE, "UUID")
                )
            )
            return result
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return None
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return None

    def StartNotify(self, cbfunct, reply_handler, error_handler, timeout):
        try:
            if not cbfunct:
                self.logger.info("please provide the notify callback function")
            self.received = cbfunct
            self.gattCharacteristic_register_signal()
            self.characteristic.StartNotify(
                reply_handler=reply_handler,
                error_handler=error_handler,
                timeout=timeout,
            )
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
        except Exception as ex:
            self.logger.debug(traceback.format_exc())

    def StopNotify(self, reply_handler, error_handler, timeout):
        try:
            self.logger.debug("stopping notifying")
            self.characteristic.StopNotify(
                reply_handler=reply_handler,
                error_handler=error_handler,
                timeout=timeout,
            )
            self.gattCharacteristic_unregister_signal()
            self.received = None
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
        except Exception as ex:
            self.logger.debug(traceback.format_exc())

    @property
    def Notifying(self):
        try:
            result = self.characteristic_properties.Get(
                CHARACTERISTIC_INTERFACE, "Notifying"
            )
            return bool(result)
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))
            return False
        except Exception as ex:
            self.logger.debug(traceback.format_exc())
            return False


class BluezManager(ChipBleBase):
    def __init__(self, devMgr, logger=None):
        if logger:
            self.logger = logger
        else:
            self.logger = logging.getLogger("ChipBLEMgr")
            logging.basicConfig(
                level=logging.INFO,
                format="%(asctime)s %(name)-12s %(levelname)-8s %(message)s",
            )
        self.scan_quiet = False
        self.peripheral_list = []
        self.device_uuid_list = []
        self.chip_queue = six.moves.queue.Queue()
        self.Gmainloop = None
        self.daemon_thread = None
        self.adapter = None
        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        GObject.threads_init()
        dbus.mainloop.glib.threads_init()
        self.bus = dbus.SystemBus()
        self.bluez = dbus.Interface(
            self.bus.get_object(BLUEZ_NAME, "/"), "org.freedesktop.DBus.ObjectManager"
        )
        self.target = None
        self.service = None
        self.orig_input_hook = None
        self.hookFuncPtr = None
        self.connect_state = False
        self.tx = None
        self.rx = None
        self.setInputHook(self.readlineCB)
        self.devMgr = devMgr
        self.devMgr.SetBlockingCB(self.devMgrCB)

        def HandleBleEventCB():
            return self.GetBleEvent()

        def HandleBleWriteCharCB(connObj, svcId, charId, buffer, length):
            return self.WriteBleCharacteristic(connObj, svcId, charId, buffer, length)

        def HandleBleSubscribeCB(connObj, svcId, charId, subscribe):
            return self.SubscribeBleCharacteristic(connObj, svcId, charId, subscribe)

        def HandleBleCloseCB(connObj):
            return self.CloseBle(connObj)

        self.devMgr.SetBleEventCB(HandleBleEventCB)
        self.devMgr.SetBleWriteCharCB(HandleBleWriteCharCB)
        self.devMgr.SetBleSubscribeCharCB(HandleBleSubscribeCB)
        self.devMgr.SetBleCloseCB(HandleBleCloseCB)

    def __del__(self):
        self.disconnect()
        self.setInputHook(self.orig_input_hook)
        self.devMgr.SetBlockingCB(None)
        self.devMgr.SetBleEventCB(None)

    def ble_adapter_select(self, identifier=None):
        if self.adapter:
            self.adapter.destroy()
            self.adapter = None
        self.adapter = self.get_adapter_by_addr(identifier)
        self.adapter.adapter_register_signal()
        self.adapter.Powered(False)
        self.adapter.Powered(True)

    def ble_adapter_print(self):
        try:
            adapters = [
                BluezDbusAdapter(p["object"], self.bluez, self.bus, self.logger)
                for p in get_bluez_objects(
                    self.bluez, self.bus, ADAPTER_INTERFACE, "/org/bluez"
                )
            ]
            for i in range(len(adapters)):
                self.logger.info("adapter %s = %s" % (i, adapters[i].Address))
        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))

    def get_adapter_by_addr(self, identifier):
        try:
            adapters = [
                BluezDbusAdapter(p["object"], self.bluez, self.bus, self.logger)
                for p in get_bluez_objects(
                    self.bluez, self.bus, ADAPTER_INTERFACE, "/org/bluez"
                )
            ]
            if identifier is None:
                return adapters[0]
            if len(adapters) > 0:
                for adapter in adapters:
                    if str(adapter.Address).upper() == str(identifier).upper():
                        return adapter
            self.logger.info(
                "adapter %s cannot be found, expect the ble mac address" % (identifier)
            )
            return None

        except dbus.exceptions.DBusException as ex:
            self.logger.debug(str(ex))

    def runLoopUntil(self, target=None, **kwargs):
        if target:
            self.daemon_thread = threading.Thread(
                target=self.running_thread, args=(target, kwargs)
            )
            self.daemon_thread.daemon = True
            self.daemon_thread.start()

        try:
            self.Gmainloop = GObject.MainLoop()
            self.Gmainloop.run()
        except KeyboardInterrupt:
            self.Gmainloop.quit()
            sys.exit(1)

    def running_thread(self, target, kwargs):
        try:
            while not self.Gmainloop or not self.Gmainloop.is_running():
                time.sleep(0.00001)
            target(**kwargs)
        except Exception as err:
            traceback.print_exc()
        finally:
            self.Gmainloop.quit()

    def setInputHook(self, hookFunc):
        """Set the PyOS_InputHook to call the specific function."""
        hookFunctionType = CFUNCTYPE(None)
        self.hookFuncPtr = hookFunctionType(hookFunc)
        pyos_inputhook_ptr = c_void_p.in_dll(pythonapi, "PyOS_InputHook")
        # save the original so that on del we can revert it back to the way it was.
        self.orig_input_hook = cast(pyos_inputhook_ptr.value, PYFUNCTYPE(c_int))
        # set the new hook. readLine will call this periodically as it polls for input.
        pyos_inputhook_ptr.value = cast(self.hookFuncPtr, c_void_p).value

    def runIdleLoop(self, **kwargs):
        time.sleep(0)

    def devMgrCB(self):
        self.runLoopUntil(self.runIdleLoop)

    def readlineCB(self):
        self.runLoopUntil(self.runIdleLoop)

        if self.orig_input_hook:
            self.orig_input_hook()

    def scan_connect(self, line):
        """ API to perform both scan and connect operations in one call."""
        args = self.ParseInputLine(line, "scan-connect")
        if not args:
            return False
        if not self.adapter:
            self.logger.info("use default adapter")
            self.ble_adapter_select()
        self.scan_quiet = args[1]
        self.scan(line)
        if self.target:
            return self.connect(args[2])
        else:
            self.logger.info(
                "Failed to scan device named: " + args[2] + ". Connection skipped."
            )
            return False

    def dump_scan_result(self, device):
        self.logger.info("{0:<16}= {1}".format("Name", device.Name))
        self.logger.info("{0:<16}= {1}".format("ID", device.device_id))
        self.logger.info("{0:<16}= {1}".format("RSSI", device.RSSI))
        self.logger.info("{0:<16}= {1}".format("Address", device.Address))

        devIdInfo = self.get_peripheral_devIdInfo(device)
        if devIdInfo != None:
            self.logger.info("{0:<16}= {1}".format("Pairing State", devIdInfo.pairingState))
            self.logger.info("{0:<16}= {1}".format("Discriminator", devIdInfo.discriminator))
            self.logger.info("{0:<16}= {1}".format("Vendor Id", devIdInfo.vendorId))
            self.logger.info("{0:<16}= {1}".format("Product Id", devIdInfo.productId))

        if device.ServiceData:
            for advuuid in device.ServiceData:
                self.logger.info("{0:<16}= {1}".format("Adv UUID", str(advuuid)))
                self.logger.info("{0:<16}= {1}".format("Adv Data", bytes(device.ServiceData[advuuid]).hex()))
        else:
            self.logger.info("")
        self.logger.info("")

    def scan_bg_implementation(self, **kwargs):
        self.adapter.clear_adapter()
        with self.chip_queue.mutex:
            self.chip_queue.queue.clear()
        self.adapter.adapter_bg_scan(True)
        found = False
        identifier = kwargs["identifier"]
        timeout = kwargs["timeout"] + time.time()
        self.device_uuid_list = []
        self.peripheral_list = []

        while time.time() < timeout:
            scanned_peripheral_list = self.adapter.find_devices(
                [
                    chip_service,
                    chip_service_short,
                    chromecast_setup_service,
                    chromecast_setup_service_short,
                ]
            )
            for device in scanned_peripheral_list:
                try:
                    if not self.scan_quiet and device.Address not in self.device_uuid_list:
                        # display all scanned results
                        self.device_uuid_list.append(device.Address)
                        self.peripheral_list.append(device)
                        self.dump_scan_result(device)
                    devIdInfo = self.get_peripheral_devIdInfo(device)
                    if not devIdInfo:
                        # Not a chip device
                        continue
                    if identifier and (device.Name == identifier or str(device.Address).upper() == str(
                        identifier.upper()
                    ) or str(devIdInfo.discriminator) == identifier):
                        if self.scan_quiet:
                            # only display the scanned target's info when quiet
                            self.dump_scan_result(device)
                        self.target = device
                        found = True
                        break
                except Exception:
                    traceback.print_exc()
            if found:
                break

            time.sleep(BLE_IDLE_DELTA)
        self.adapter.adapter_bg_scan(False)

    def scan(self, line):
        args = self.ParseInputLine(line, "scan")
        if not args:
            return False
        self.target = None
        if not self.adapter:
            self.logger.info("use default adapter")
            self.ble_adapter_select()
        del self.peripheral_list[:]
        self.scan_quiet = args[1]
        self.runLoopUntil(
            self.scan_bg_implementation, timeout=args[0], identifier=args[2]
        )
        return True

    def chipServieCharConnect(self):
        gatt_dic = {
            "services": [chip_service, chip_service_short],
            "chars": [chip_tx, chip_rx],
        }
        self.service = self.target.service_discover(gatt_dic)
        if self.service is None:
            self.logger.info("chip service cannot be found")
            return False

        self.rx = self.service.find_characteristic(chip_rx)
        if self.rx is None:
            self.logger.info("chip rx char cannot be found")
            return False

        self.tx = self.service.find_characteristic(chip_tx)
        if self.tx is None:
            self.logger.info("chip tx char cannot be found")
            self.connect_state = False
            return False

        return True

    def connect_bg_implementation(self, **kwargs):
        self.connect_state = False
        if self.adapter is None:
            self.logger.info("adapter is not configured")
            return
        self.target.device_register_signal()
        self.target.device_bg_connect(True)
        if self.chipServieCharConnect():
            self.logger.info("connect success")
            self.connect_state = True
        else:
            self.logger.info("connect fail")
            self.connect_state = False

    def disconnect_bg_implementation(self, **kwargs):
        if self.target:
            self.target.device_bg_connect(False)
        if self.tx:
            self.tx.destroy()
            self.tx = None
        if self.rx:
            self.rx.destroy()
            self.rx = None
        if self.service:
            self.service.destroy()
            self.service = None

    def get_peripheral_devIdInfo(self, peripheral):
        if not peripheral.ServiceData:
            return None
        for advuuid in peripheral.ServiceData:
            if str(advuuid).lower() == str(chip_service).lower():
                return ParseServiceData(bytes(peripheral.ServiceData[advuuid]))
        return None

    def connect(self, identifier):
        found = False
        self.logger.info("trying to connect to " + identifier)
        for p in self.peripheral_list:
            p_discriminator = None
            devIdInfo = self.get_peripheral_devIdInfo(p)
            if not devIdInfo:
                # Not a chip device
                continue
            p_discriminator = devIdInfo.discriminator
            p_id = str(p.device_id)
            p_name = str(p.Name)
            p_address = str(p.Address)
            self.logger.debug(p_id + " vs " + str(identifier))
            self.logger.debug(p_name + " vs " + str(identifier))
            self.logger.debug(p_address + " vs " + str(identifier))
            self.logger.debug(str(p_discriminator) + " vs " + str(identifier))
            if (
                p_id == str(identifier)
                or p_name == str(identifier)
                or p_address.upper() == str(identifier).upper()
                or str(p_discriminator) == str(identifier)
            ):
                self.target = p
                found = True
                break
        if found:
            self.runLoopUntil(
                self.connect_bg_implementation,
                identifier=identifier,
                timeout=BLE_CONNECT_TIMEOUT_SEC,
            )
            if self.connect_state:
                return True
            else:
                return False
        else:
            print("device cannot be found")
            return False

    def disconnect(self):
        self.runLoopUntil(self.disconnect_bg_implementation)
        for i in range(2):
            n = gc.collect()
            self.logger.debug("Unreached objects: %d", n)
            self.logger.debug("Final Garbage:")
            self.logger.debug(pprint.pformat(gc.garbage))

    def WriteCharactertisticSuccessCB(self, *args):
        self.logger.debug("write complete")
        if self.devMgr:
            txEvent = BleTxEvent(
                charId=self.charId_tx, svcId=self.svcId_tx, status=True
            )
            self.chip_queue.put(txEvent)
            self.devMgr.DriveBleIO()

    def WriteCharactertisticErrorCB(self, *args):
        self.logger.debug("write fail, error:" + repr(args))
        if self.devMgr:
            txEvent = BleTxEvent(
                charId=self.charId_tx, svcId=self.svcId_tx, status=False
            )
            self.chip_queue.put(txEvent)
            self.devMgr.DriveBleIO()

    def WriteBleCharacteristic(self, connObj, svcId, charId, buffer, length):
        self.logger.debug("write start")
        result = False
        if self.target and self.target.Connected:
            converted_data = ChipUtility.VoidPtrToByteArray(buffer, length)
            self.charId_tx = bytearray(
                uuid.UUID(str(VoidPtrToUUIDString(charId, 16))).bytes
            )
            self.svcId_tx = bytearray(
                uuid.UUID(str(VoidPtrToUUIDString(svcId, 16))).bytes
            )
            self.tx.WriteValue(
                dbus.Array([dbus.Byte(i) for i in converted_data], "y"),
                options="",
                reply_handler=self.WriteCharactertisticSuccessCB,
                error_handler=self.WriteCharactertisticErrorCB,
                timeout=BLE_WRITE_CHARACTERISTIC_TIMEOUT_SEC,
            )
            result = True
        else:
            self.logger.warning("WARNING: peripheral is no longer connected.")
        return result

    def receivedNotificationCB(self, data):
        self.logger.debug("received data")
        bytes = bytearray(data)
        if self.devMgr:
            rxEvent = BleRxEvent(
                charId=self.charId_rx, svcId=self.svcId_rx, buffer=bytes
            )
            self.chip_queue.put(rxEvent)
            self.devMgr.DriveBleIO()

    def subscribeSuccessCb(self, *args):
        self.logger.debug("subscribe complete")
        if self.rx.Notifying:
            success = True
        else:
            success = False
        operation = BLE_SUBSCRIBE_OPERATION_SUBSCRIBE
        if self.devMgr:
            subscribeEvent = BleSubscribeEvent(
                charId=self.charId_rx,
                svcId=self.svcId_rx,
                status=success,
                operation=operation,
            )
            self.chip_queue.put(subscribeEvent)
            self.devMgr.DriveBleIO()

    def subscribeErrorCb(self, *args):
        self.logger.error("subscribe fail, error:" + repr(args))
        success = False
        operation = BLE_SUBSCRIBE_OPERATION_SUBSCRIBE
        if self.devMgr:
            subscribeEvent = BleSubscribeEvent(
                charId=self.charId_rx,
                svcId=self.svcId_rx,
                status=success,
                operation=operation,
            )
            self.chip_queue.put(subscribeEvent)
            self.devMgr.DriveBleIO()

    def unsubscribeSuccessCb(self, *args):
        self.logger.debug("unsubscribe complete")
        success = True
        operation = BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE
        if self.devMgr:
            subscribeEvent = BleSubscribeEvent(
                charId=self.charId_rx,
                svcId=self.svcId_rx,
                status=success,
                operation=operation,
            )
            self.chip_queue.put(subscribeEvent)
            self.devMgr.DriveBleIO()

    def unsubscribeErrorCb(self, *args):
        self.logger.error("unsubscribe fail, error:" + repr(args))
        success = False
        operation = BLE_SUBSCRIBE_OPERATION_UNSUBSCRIBE
        if self.devMgr:
            subscribeEvent = BleSubscribeEvent(
                charId=self.charId_rx,
                svcId=self.svcId_rx,
                status=success,
                operation=operation,
            )
            self.chip_queue.put(subscribeEvent)
            self.devMgr.DriveBleIO()

    def SubscribeBleCharacteristic(self, connObj, svcId, charId, subscribe):
        result = False
        self.charId_rx = bytearray(uuid.UUID(VoidPtrToUUIDString(charId, 16)).bytes)
        self.svcId_rx = bytearray(uuid.UUID(str(VoidPtrToUUIDString(svcId, 16))).bytes)

        if self.target and self.target.Connected:
            try:
                if subscribe:
                    self.logger.debug("try to subscribe")
                    self.rx.StartNotify(
                        cbfunct=self.receivedNotificationCB,
                        reply_handler=self.subscribeSuccessCb,
                        error_handler=self.subscribeErrorCb,
                        timeout=BLE_SUBSCRIBE_TIMEOUT_SEC,
                    )
                else:
                    self.logger.debug("try to unsubscribe")
                    self.rx.StopNotify(
                        reply_handler=self.unsubscribeSuccessCb,
                        error_handler=self.unsubscribeErrorCb,
                        timeout=BLE_SUBSCRIBE_TIMEOUT_SEC,
                    )
            except Exception as ex:
                self.logger.debug(traceback.format_exc())
                self.logger.debug("(un)subscribe error")
            result = True
        else:
            self.logger.warning("WARNING: peripheral is no longer connected.")
        return result

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
        # Workaround: comment out disconnect because of hang when close, plz call disconnect explicitly after close
        # Need to fix it
        # self.disconnect()
        if self.devMgr:
            dcEvent = BleDisconnectEvent(BLE_ERROR_REMOTE_DEVICE_DISCONNECTED)
            self.chip_queue.put(dcEvent)
            self.devMgr.DriveBleIO()
        return True
