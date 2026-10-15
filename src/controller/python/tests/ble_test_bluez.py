import argparse
import json

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

ADAPTER = '/org/bluez/hci0'
INTERFACE = 'org.bluez.Adapter1'
CONTROL = 'org.bluez.TestControl'


class Adapter(dbus.service.Object):
    def __init__(self, name):
        super().__init__(name, ADAPTER)
        self.scenario = 'success'
        self.starts = 0
        self.stops = 0
        self.properties = {
            'Address': '00:11:22:33:44:55',
            'AddressType': 'public',
            'Name': 'isolated-ble-test',
            'Alias': 'isolated-ble-test',
            'Powered': dbus.Boolean(True),
            'Discoverable': dbus.Boolean(False),
            'Discovering': dbus.Boolean(False),
            'UUIDs': dbus.Array([], signature='s'),
        }

    @dbus.service.method('org.freedesktop.DBus.Properties', in_signature='s', out_signature='a{sv}')
    def GetAll(self, interface):
        return self.properties if interface == INTERFACE else {}

    @dbus.service.method('org.freedesktop.DBus.Properties', in_signature='ss', out_signature='v')
    def Get(self, interface, name):
        return self.GetAll(interface)[name]

    @dbus.service.method('org.freedesktop.DBus.Properties', in_signature='ssv', out_signature='')
    def Set(self, interface, name, value):
        if interface != INTERFACE:
            raise dbus.exceptions.DBusException('Unknown interface', name='org.bluez.Error.InvalidArguments')
        self.properties[name] = value
        self.PropertiesChanged(interface, {name: value}, [])

    @dbus.service.signal('org.freedesktop.DBus.Properties', signature='sa{sv}as')
    def PropertiesChanged(self, interface, changed, invalidated):
        pass

    @dbus.service.method(INTERFACE, in_signature='a{sv}', out_signature='')
    def SetDiscoveryFilter(self, discovery_filter):
        pass

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def StartDiscovery(self):
        self.starts += 1
        if self.scenario == 'start-error':
            raise dbus.exceptions.DBusException('Injected start failure', name='org.bluez.Error.Failed')
        self.properties['Discovering'] = dbus.Boolean(True)
        self.PropertiesChanged(INTERFACE, {'Discovering': self.properties['Discovering']}, [])

    @dbus.service.method(INTERFACE, in_signature='', out_signature='')
    def StopDiscovery(self):
        self.stops += 1
        if self.scenario == 'stop-error':
            raise dbus.exceptions.DBusException('Injected stop failure', name='org.bluez.Error.InProgress')
        self.properties['Discovering'] = dbus.Boolean(False)
        self.PropertiesChanged(INTERFACE, {'Discovering': self.properties['Discovering']}, [])


class Bluez(dbus.service.Object):
    def __init__(self, bus):
        self.name = dbus.service.BusName('org.bluez', bus)
        super().__init__(self.name, '/')
        self.adapter = Adapter(self.name)

    @dbus.service.method('org.freedesktop.DBus.ObjectManager', in_signature='', out_signature='a{oa{sa{sv}}}')
    def GetManagedObjects(self):
        return {ADAPTER: {INTERFACE: self.adapter.properties}}

    @dbus.service.method(CONTROL, in_signature='s', out_signature='')
    def Configure(self, scenario):
        if scenario not in ('success', 'start-error', 'stop-error'):
            raise dbus.exceptions.DBusException('Unknown scenario', name='org.bluez.Error.InvalidArguments')
        self.adapter.scenario = scenario
        self.adapter.starts = 0
        self.adapter.stops = 0
        self.adapter.properties['Discovering'] = dbus.Boolean(False)
        self.adapter.PropertiesChanged(INTERFACE, {'Discovering': dbus.Boolean(False)}, [])

    @dbus.service.method(CONTROL, in_signature='', out_signature='uub')
    def Counts(self):
        return self.adapter.starts, self.adapter.stops, self.adapter.properties['Discovering']


def main() -> None:
    parser = argparse.ArgumentParser(description='BlueZ fault fixture for an isolated D-Bus session')
    parser.add_argument('--self-test', action='store_true')
    parser.add_argument('--control', choices=('success', 'start-error', 'stop-error', 'counts'))
    arguments = parser.parse_args()
    DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    if arguments.control:
        control = dbus.Interface(bus.get_object('org.bluez', '/'), CONTROL)
        if arguments.control == 'counts':
            starts, stops, discovering = control.Counts()
            print(json.dumps([int(starts), int(stops), bool(discovering)]))
        else:
            control.Configure(arguments.control)
        return
    service = Bluez(bus)
    if arguments.self_test:
        service.Configure('stop-error')
        service.adapter.StartDiscovery()
        try:
            service.adapter.StopDiscovery()
        except dbus.exceptions.DBusException as error:
            assert error.get_dbus_name() == 'org.bluez.Error.InProgress'
        else:
            raise AssertionError('StopDiscovery did not inject an error')
        assert service.Counts() == (1, 1, True)
        print('PASS: isolated BlueZ stop-error fixture and counts')
        return
    print('READY', flush=True)
    GLib.MainLoop().run()


if __name__ == '__main__':
    main()
