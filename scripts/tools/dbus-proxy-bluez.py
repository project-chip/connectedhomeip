#!/usr/bin/env python3
#
#  Copyright (c) 2024 Project CHIP Authors
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

import logging
import os.path
from argparse import ArgumentParser
from collections import namedtuple
import typing

from gi.repository import Gio, GLib


def bus_get_connection(address: str):
    """Get a connection object for a given D-Bus bus."""
    if address == "session":
        address = Gio.dbus_address_get_for_bus_sync(Gio.BusType.SESSION)
    elif address == "system":
        address = Gio.dbus_address_get_for_bus_sync(Gio.BusType.SYSTEM)
    logging.info("Connecting to: %s", address)
    conn = Gio.DBusConnection.new_for_address_sync(
        address,
        Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT |
        Gio.DBusConnectionFlags.MESSAGE_BUS_CONNECTION)
    logging.info("Assigned unique name: %s", conn.get_unique_name())
    return conn


def bus_get_name_owner(conn, name: str):
    """Get the unique name of a well known name on a D-Bus bus."""
    params = GLib.Variant("(s)", (name,))
    reply = conn.call_sync("org.freedesktop.DBus", "/org/freedesktop/DBus",
                           "org.freedesktop.DBus", "GetNameOwner",
                           params, None, Gio.DBusCallFlags.NONE, -1)
    return reply.get_child_value(0).get_string()


def bus_introspect_path(conn, client: str, path: str):
    """Introspect a D-Bus object path and return its node info."""
    reply = conn.call_sync(client, path,
                           "org.freedesktop.DBus.Introspectable", "Introspect",
                           None, None, Gio.DBusCallFlags.NONE, -1)
    xml = reply.get_child_value(0).get_string()
    return Gio.DBusNodeInfo.new_for_xml(xml)


class DBusServiceProxy:

    MappingKey = namedtuple("MappingKey", ["path", "iface"])

    objects: typing.Dict[MappingKey, int] = {}
    subscriptions: typing.Set[str] = set()
    clients = {}

    def __init__(self, source: str, proxy: str, service: str):
        self.source = bus_get_connection(source)
        self.proxy = bus_get_connection(proxy)
        self.service = service
        Gio.bus_own_name_on_connection(self.proxy, self.service,
                                       Gio.BusNameOwnerFlags.DO_NOT_QUEUE,
                                       self.on_bus_name_acquired,
                                       self.on_bus_name_lost)

    def on_bus_name_acquired(self, conn, name):
        logging.info("Acquired name on proxy bus: %s", name)
        self.mirror_source_on_proxy(self.service, "/")

    def on_bus_name_lost(self, conn, name):
        logging.debug("Lost name on proxy bus: %s", name)

    def proxy_client_save(self, path, client):
        self.clients[path] = client

    def proxy_client_load(self, path):
        return self.clients[path]

    def register_object(self, conn, path, iface):
        key = DBusServiceProxy.MappingKey(path, iface.name)
        if key not in self.objects:
            logging.debug("Registering: %s { %s }", path, iface.name)
            id = conn.register_object(path, iface, self.on_method_call)
            self.objects[key] = id

    def unregister_object(self, conn, path, iface_name):
        key = DBusServiceProxy.MappingKey(path, iface_name)
        if key in self.objects:
            logging.debug("Removing: %s { %s }", path, iface_name)
            conn.unregister_object(self.objects.pop(key))

    def signal_subscribe(self, conn, client):
        """Subscribe for signals from a D-Bus client."""
        if client not in self.subscriptions:
            conn.signal_subscribe(client, None, None, None, None,
                                  Gio.DBusSignalFlags.NONE,
                                  self.on_signal_received)
            self.subscriptions.add(client)

    def mirror_path(self, conn_src, conn_dest, client, path, save=False):
        """Mirror all interfaces and nodes of a D-Bus client object path.

        Parameters:
        conn_src -- source D-Bus connection
        conn_dest -- proxy D-Bus connection
        client -- name of the client on the source bus
        path -- object path to mirror recursively
        save -- save the client name for the path

        """
        info = bus_introspect_path(conn_src, client, path)
        for iface in info.interfaces:
            if save:
                self.proxy_client_save(path, client)
            self.register_object(conn_dest, path, iface)
        for node in info.nodes:
            self.mirror_path(conn_src, conn_dest, client,
                             os.path.join(path, node.path), save)

    def mirror_source_on_proxy(self, client, path):
        """Mirror source bus objects on the proxy bus."""
        self.signal_subscribe(self.source, client)
        self.mirror_path(self.source, self.proxy, client, path)

    def mirror_proxy_on_source(self, client, path):
        """Mirror proxy bus objects on the source bus."""
        self.signal_subscribe(self.proxy, client)
        self.mirror_path(self.proxy, self.source, client, path, True)

    def on_method_call(self, conn, sender, *args, **kwargs):
        if conn == self.source:
            return self.on_method_call_from_source(sender, *args, **kwargs)
        return self.on_method_call_from_proxy(sender, *args, **kwargs)

    def on_signal_received(self, conn, sender, *args, **kwargs):
        if conn == self.source:
            return self.on_signal_from_source(sender, *args, **kwargs)
        return self.on_signal_from_proxy(sender, *args, **kwargs)

    def on_method_call_from_source(self, sender, path, iface, method,
                                   params, invocation):
        logging.debug("Call from source: %s %s.%s()", path, iface, method)
        self.proxy.call(self.proxy_client_load(path), path, iface, method,
                        params, None, Gio.DBusCallFlags.NONE, -1, None,
                        self.on_method_return, invocation)

    def on_method_call_from_proxy(self, sender, path, iface, method,
                                  params, invocation):
        logging.debug("Call from proxy: %s %s.%s()", path, iface, method)
        self.source.call(self.service, path, iface, method,
                         params, None, Gio.DBusCallFlags.NONE, -1, None,
                         self.on_method_return, invocation)

    def on_method_return(self, conn, result, invocation):
        try:
            logging.debug("Finishing call: %s %s.%s()",
                          invocation.get_object_path(),
                          invocation.get_interface_name(),
                          invocation.get_method_name())
            reply = conn.call_with_unix_fd_list_finish(result)
            invocation.return_value_with_unix_fd_list(
                reply[0], reply.out_fd_list)
        except GLib.Error as e:
            invocation.return_gerror(e)

    def on_signal_from_source(self, sender, path, iface, signal, params):
        logging.debug("Signal from source: %s %s.%s", path, iface, signal)
        if iface == "org.freedesktop.DBus.ObjectManager":
            if signal == "InterfacesAdded":
                dest_path = params.get_child_value(0).get_string()
                self.mirror_source_on_proxy(self.service, dest_path)
            if signal == "InterfacesRemoved":
                dest_path = params.get_child_value(0).get_string()
                for dest_iface in params.get_child_value(1).get_strv():
                    self.unregister_object(self.proxy, dest_path, dest_iface)
        self.proxy.emit_signal(None, path, iface, signal, params)

    def on_signal_from_proxy(self, sender, path, iface, signal, params):
        logging.debug("Signal from proxy: %s %s.%s", path, iface, signal)
        self.source.emit_signal(None, path, iface, signal, params)


class BluezProxy(DBusServiceProxy):

    def on_method_call_from_proxy(self, sender, path, iface, method,
                                  params, invocation):

        if (iface == "org.bluez.GattManager1" and
                method == "RegisterApplication"):
            app_path = params.get_child_value(0).get_string()
            logging.info("Mirroring GATT application: %s %s", sender, app_path)
            self.mirror_proxy_on_source(sender, app_path)

        if iface == "org.bluez.LEAdvertisingManager1":
            if method == "RegisterAdvertisement":
                app_path = params.get_child_value(0).get_string()
                logging.info("Mirroring advertiser: %s %s", sender, app_path)
                self.mirror_proxy_on_source(sender, app_path)

        super().on_method_call_from_proxy(sender, path, iface, method,
                                          params, invocation)


parser = ArgumentParser(description="BlueZ D-Bus proxy")
parser.add_argument(
    "-v", "--verbose", action="store_true",
    help="enable debug output")
parser.add_argument(
    "--bus-source", metavar="ADDRESS", default="system",
    help="""address of the source D-Bus bus; it can be a bus address string or
    'session' or 'system' keywords; default is '%(default)s'""")
parser.add_argument(
    "--bus-proxy", metavar="ADDRESS", required=True,
    help="""address of the proxy D-Bus bus""")

args = parser.parse_args()
logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)

BluezProxy(args.bus_source, args.bus_proxy, "org.bluez")
GLib.MainLoop().run()
