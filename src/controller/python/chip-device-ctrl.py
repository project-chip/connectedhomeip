#!/usr/bin/env python

#
#    Copyright (c) 2020-2021 Project CHIP Authors
#    Copyright (c) 2013-2018 Nest Labs, Inc.
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
#      This file implements the Python-based Chip Device Controller Shell.
#

from __future__ import absolute_import, print_function

import argparse
import base64
import ctypes
import logging
import os
import platform
import random
import shlex
import string
import sys
import textwrap
import time
import traceback
import warnings
from cmd import Cmd
from optparse import OptionParser, OptionValueError

import chip.logging
import coloredlogs
from chip import ChipCommissionableNodeCtrl, ChipStack, exceptions, native
from chip.setup_payload import SetupPayload
from rich import pretty, print

# Extend sys.path with one or more directories, relative to the location of the
# running script, in which the chip package might be found .  This makes it
# possible to run the device manager shell from a non-standard install location,
# as well as directly from its location the CHIP source tree.
#
# Note that relative package locations are prepended to sys.path so as to give
# the local version of the package higher priority over any version installed in
# a standard location.
#
scriptDir = os.path.dirname(os.path.abspath(__file__))
relChipPackageInstallDirs = [
    ".",
    "../lib/python",
    "../lib/python%s.%s" % (sys.version_info.major, sys.version_info.minor),
    "../lib/Python%s%s" % (sys.version_info.major, sys.version_info.minor),
]
for relInstallDir in relChipPackageInstallDirs:
    absInstallDir = os.path.realpath(os.path.join(scriptDir, relInstallDir))
    if os.path.isdir(os.path.join(absInstallDir, "chip")):
        sys.path.insert(0, absInstallDir)


if platform.system() == 'Darwin':
    from chip.ChipCoreBluetoothMgr import CoreBluetoothManager as BleManager
elif sys.platform.startswith('linux'):
    from chip.ChipBluezMgr import BluezManager as BleManager

# The exceptions for CHIP Device Controller CLI


class ChipDevCtrlException(exceptions.ChipStackException):
    pass


class ParsingError(ChipDevCtrlException):
    def __init__(self, msg=None):
        self.msg = "Parsing Error: " + msg

    def __str__(self):
        return self.msg


def DecodeBase64Option(option, opt, value):
    try:
        return base64.standard_b64decode(value)
    except TypeError:
        raise OptionValueError(
            "option %s: invalid base64 value: %r" % (opt, value))


def DecodeHexIntOption(option, opt, value):
    try:
        return int(value, 16)
    except ValueError:
        raise OptionValueError("option %s: invalid value: %r" % (opt, value))


def ParseEncodedString(value):
    if value.find(":") < 0:
        raise ParsingError(
            "value should be encoded in encoding:encodedvalue format")
    enc, encValue = value.split(":", 1)
    if enc == "str":
        return encValue.encode("utf-8") + b'\x00'
    elif enc == "hex":
        return bytes.fromhex(encValue)
    raise ParsingError("only str and hex encoding is supported")


def ParseValueWithType(value, type):
    if type == 'int':
        return int(value)
    elif type == 'str':
        return value
    elif type == 'bytes':
        return ParseEncodedString(value)
    elif type == 'bool':
        return (value.upper() not in ['F', 'FALSE', '0'])
    else:
        raise ParsingError('cannot recognize type: {}'.format(type))


def FormatZCLArguments(args, command):
    commandArgs = {}
    for kvPair in args:
        if kvPair.find("=") < 0:
            raise ParsingError("Argument should in key=value format")
        key, value = kvPair.split("=", 1)
        valueType = command.get(key, None)
        commandArgs[key] = ParseValueWithType(value, valueType)
    return commandArgs


def ShowColoredWarnings(message, category, filename, lineno, file=None, line=None):
    logging.warning(' %s:%s: %s:%s' %
                    (filename, lineno, category.__name__, message))
    return


class DeviceMgrCmd(Cmd):
    def __init__(self, rendezvousAddr=None, controllerNodeId=1, bluetoothAdapter=None):
        self.lastNetworkId = None
        self.replHint = None

        pretty.install(indent_guides=True, expand_all=True)

        coloredlogs.install(level='DEBUG')
        chip.logging.RedirectToPythonLogging()

        logging.getLogger().setLevel(logging.DEBUG)
        warnings.showwarning = ShowColoredWarnings

        Cmd.__init__(self)

        Cmd.identchars = string.ascii_letters + string.digits + "-"

        if sys.stdin.isatty():
            self.prompt = "chip-device-ctrl > "
        else:
            self.use_rawinput = 0
            self.prompt = ""

        DeviceMgrCmd.command_names.sort()

        self.bleMgr = None

        self.chipStack = ChipStack.ChipStack(
            bluetoothAdapter=bluetoothAdapter, persistentStoragePath='/tmp/chip-device-ctrl-storage.json')
        self.certificateAuthorityManager = chip.CertificateAuthority.CertificateAuthorityManager(chipStack=self.chipStack)
        self.certificateAuthority = self.certificateAuthorityManager.NewCertificateAuthority()
        self.fabricAdmin = self.certificateAuthority.NewFabricAdmin(vendorId=0xFFF1, fabricId=1)
        self.devCtrl = self.fabricAdmin.NewController(
            nodeId=controllerNodeId, useTestCommissioner=True)

        self.commissionableNodeCtrl = ChipCommissionableNodeCtrl.ChipCommissionableNodeController(
            self.chipStack)

        # If we are on Linux and user selects non-default bluetooth adapter.
        if sys.platform.startswith("linux") and (bluetoothAdapter is not None):
            try:
                self.bleMgr = BleManager(self.devCtrl)
                self.bleMgr.ble_adapter_select(
                    "hci{}".format(bluetoothAdapter))
            except Exception as ex:
                traceback.print_exc()
                print(
                    "Failed to initialize BLE, if you don't have BLE, run chip-device-ctrl with --no-ble")
                raise ex

        self.historyFileName = os.path.expanduser(
            "~/.chip-device-ctrl-history")

        try:
            import readline

            if "libedit" in readline.__doc__:
                readline.parse_and_bind("bind ^I rl_complete")
            readline.set_completer_delims(" ")
            try:
                readline.read_history_file(self.historyFileName)
            except IOError:
                pass
        except ImportError:
            pass

    command_names = [
        "setup-payload",

        "ble-scan",
        "ble-adapter-select",
        "ble-adapter-print",
        "ble-debug-log",

        "connect",
        "close-ble",
        "close-session",
        "resolve",
        "paseonly",
        "commission",
        "zcl",
        "zclread",
        "zclsubscribe",

        "discover",

        "set-pairing-wifi-credential",
        "set-pairing-thread-credential",

        "open-commissioning-window",

        "get-fabricid",
    ]

    def parseline(self, line):
        cmd, arg, line = Cmd.parseline(self, line)
        if cmd:
            cmd = self.shortCommandName(cmd)
            line = cmd + " " + arg
        return cmd, arg, line

    def completenames(self, text, *ignored):
        return [
            name + " "
            for name in DeviceMgrCmd.command_names
            if name.startswith(text) or self.shortCommandName(name).startswith(text)
        ]

    def shortCommandName(self, cmd):
        return cmd.replace("-", "")

    def precmd(self, line):
        if not self.use_rawinput and line != "EOF" and line != "":
            print(">>> " + line)
        return line

    def postcmd(self, stop, line):
        if self.replHint is not None:
            print("Try the following command in repl: ")
            print(self.replHint)
            print("")
        self.replHint = None
        if not stop and self.use_rawinput:
            self.prompt = "chip-device-ctrl > "
        return stop

    def postloop(self):
        try:
            import readline

            try:
                readline.write_history_file(self.historyFileName)
            except IOError:
                pass
        except ImportError:
            pass

    def do_help(self, line):
        if line:
            cmd, arg, unused = self.parseline(line)
            try:
                doc = getattr(self, "do_" + cmd).__doc__
            except AttributeError:
                doc = None
            if doc:
                self.stdout.write("%s\n" % textwrap.dedent(doc))
            else:
                self.stdout.write("No help on %s\n" % (line))
        else:
            self.print_topics(
                "\nAvailable commands (type help <name> for more information):",
                DeviceMgrCmd.command_names,
                15,
                80,
            )

    def do_closeble(self, line):
        """
        close-ble

        Close the ble connection to the device.
        """

        warnings.warn(
            "This method is being deprecated. "
            "Please use the DeviceController.CloseBLEConnection method directly in the REPL", DeprecationWarning)

        args = shlex.split(line)

        if len(args) != 0:
            print("Usage:")
            self.do_help("close")
            return

        try:
            self.devCtrl.CloseBLEConnection()
        except exceptions.ChipStackException as ex:
            print(str(ex))

    def do_setlogoutput(self, line):
        """
        set-log-output [ none | error | progress | detail ]

        Set the level of Chip logging output.
        """

        warnings.warn(
            "This method is being deprecated. "
            "Please use the DeviceController.SetLogFilter method directly in the REPL", DeprecationWarning)

        args = shlex.split(line)

        if len(args) == 0:
            print("Usage:")
            self.do_help("set-log-output")
            return
        if len(args) > 1:
            print("Unexpected argument: " + args[1])
            return

        category = args[0].lower()
        if category == "none":
            category = 0
        elif category == "error":
            category = 1
        elif category == "progress":
            category = 2
        elif category == "detail":
            category = 3
        else:
            print("Invalid argument: " + args[0])
            return

        try:
            self.devCtrl.SetLogFilter(category)
        except exceptions.ChipStackException as ex:
            print(str(ex))
            return

    def do_setuppayload(self, line):
        """
        setup-payload generate [options]

        Options:
          -vr  Version
          -vi  Vendor ID
          -pi  Product ID
          -cf  Custom Flow [Standard = 0, UserActionRequired = 1, Custom = 2]
          -dc  Discovery Capabilities [SoftAP = 1 | BLE = 2 | OnNetwork = 4]
          -dv  Discriminator Value
          -ps  Passcode

        setup-payload parse-manual <manual-pairing-code>
        setup-payload parse-qr <qr-code-payload>
        """

        warnings.warn(
            "This method is being deprecated. "
            "Please use the SetupPayload function in the chip.setup_payload package directly", DeprecationWarning)

        try:
            arglist = shlex.split(line)
            if arglist[0] not in ("generate", "parse-manual", "parse-qr"):
                self.do_help("setup-payload")
                return

            if arglist[0] == "generate":
                parser = argparse.ArgumentParser()
                parser.add_argument("-vr", type=int, default=0, dest='version')
                parser.add_argument(
                    "-pi", type=int, default=0, dest='productId')
                parser.add_argument(
                    "-vi", type=int, default=0, dest='vendorId')
                parser.add_argument(
                    '-cf', type=int, default=0, dest='customFlow')
                parser.add_argument(
                    "-dc", type=int, default=0, dest='capabilities')
                parser.add_argument(
                    "-dv", type=int, default=0, dest='discriminator')
                parser.add_argument("-ps", type=int, dest='passcode')
                args = parser.parse_args(arglist[1:])

                SetupPayload().PrintOnboardingCodes(args.passcode, args.vendorId, args.productId,
                                                    args.discriminator, args.customFlow, args.capabilities, args.version)

            if arglist[0] == "parse-manual":
                SetupPayload().ParseManualPairingCode(arglist[1]).Print()

            if arglist[0] == "parse-qr":
                SetupPayload().ParseQrCode(arglist[1]).Print()

        except exceptions.ChipStackException as ex:
            print(str(ex))
            return

    def do_bleadapterselect(self, line):
        """
        ble-adapter-select

        Start BLE adapter select, deprecated, you can select adapter by command line arguments.
        """
        if sys.platform.startswith("linux"):
            if not self.bleMgr:
                self.bleMgr = BleManager(self.devCtrl)

            self.bleMgr.ble_adapter_select(line)
            print(
                "This change only applies to ble-scan\n"
                "Please run device controller with --bluetooth-adapter=<adapter-name> to select adapter\n" +
                "e.g. chip-device-ctrl --bluetooth-adapter hci0"
            )
        else:
            print(
                "ble-adapter-select only works in Linux, ble-adapter-select mac_address"
            )

        return

    def do_bleadapterprint(self, line):
        """
        ble-adapter-print

        Print attached BLE adapter.
        """
        if sys.platform.startswith("linux"):
            if not self.bleMgr:
                self.bleMgr = BleManager(self.devCtrl)

            self.bleMgr.ble_adapter_print()
        else:
            print("ble-adapter-print only works in Linux")

        return

    def do_bledebuglog(self, line):
        """
        ble-debug-log 0:1
          0: disable BLE debug log
          1: enable BLE debug log
        """
        if not self.bleMgr:
            self.bleMgr = BleManager(self.devCtrl)

        self.bleMgr.ble_debug_log(line)

        return

    def do_blescan(self, line):
        """
        ble-scan

        Start BLE scanning operations.
        """

        if not self.bleMgr:
            self.bleMgr = BleManager(self.devCtrl)

        self.bleMgr.scan(line)

        return

    def ConnectFromSetupPayload(self, setupPayload, nodeid):
        # TODO(cecille): Get this from the C++ code?
        ble = 1 << 1
        # Devices may be uncommissioned, or may already be on the network. Need to check both ways.
        # TODO(cecille): implement soft-ap connection.

        # Any device that is already commissioned into a fabric needs to use on-network
        # pairing, so look first on the network regardless of the QR code contents.
        print("Attempting to find device on Network")
        longDiscriminator = ctypes.c_uint16(
            int(setupPayload.attributes['Discriminator']))
        self.devCtrl.DiscoverCommissionableNodesLongDiscriminator(
            longDiscriminator)
        print("Waiting for device responses...")
        strlen = 100
        addrStrStorage = ctypes.create_string_buffer(strlen)
        # If this device is on the network and we're looking specifically for 1 device,
        # expect a quick response.
        if self.wait_for_one_discovered_device():
            self.devCtrl.GetIPForDiscoveredDevice(
                0, addrStrStorage, strlen)
            addrStr = addrStrStorage.value.decode('utf-8')
            print("Connecting to device at " + addrStr)
            pincode = ctypes.c_uint32(
                int(setupPayload.attributes['SetUpPINCode']))
            try:
                self.devCtrl.CommissionIP(addrStrStorage, pincode, nodeid)
                print("Connected")
                return 0
            except Exception as ex:
                print(f"Unable to connect on network: {ex}")
        else:
            print("Unable to locate device on network")

        if int(setupPayload.attributes["RendezvousInformation"]) & ble:
            print("Attempting to connect via BLE")
            longDiscriminator = ctypes.c_uint16(
                int(setupPayload.attributes['Discriminator']))
            pincode = ctypes.c_uint32(
                int(setupPayload.attributes['SetUpPINCode']))
            try:
                self.devCtrl.ConnectBLE(longDiscriminator, pincode, nodeid)
                print("Connected")
                return 0
            except Exception as ex:
                print(f"Unable to connect: {ex}")
        return -1

    def do_paseonly(self, line):
        """
        paseonly -ip <ip address> <setup pin code> [<nodeid>]

        TODO: Add more methods to connect to device (like cert for auth, and IP
              for connection)
        """

        try:
            args = shlex.split(line)
            if len(args) <= 1:
                print("Usage:")
                self.do_help("paseonly")
                return
            nodeid = random.randint(1, 1000000)  # Just a random number
            if len(args) == 4:
                nodeid = int(args[3])
            print("Device is assigned with nodeid = {}".format(nodeid))
            self.replHint = f"devCtrl.EstablishPASESessionIP({repr(args[1])}, {int(args[2])}, {nodeid})"
            if args[0] == "-ip" and len(args) >= 3:
                self.devCtrl.EstablishPASESessionIP(args[1], int(args[2]), nodeid)
            else:
                print("Usage:")
                self.do_help("paseonly")
                return
            print(
                "Device temporary node id (**this does not match spec**): {}".format(nodeid))
        except Exception as ex:
            print(str(ex))
            return

    def do_commission(self, line):
        """
        commission nodeid

        Runs commissioning on a device that has been connected with paseonly
        """
        try:
            args = shlex.split(line)
            if len(args) != 1:
                print("Usage:")
                self.do_help("commission")
                return
            nodeid = int(args[0])
            self.replHint = f"devCtrl.Commission({nodeid})"
            self.devCtrl.Commission(nodeid)
        except Exception as ex:
            print(str(ex))
            return

    def do_connect(self, line):
        """
        connect -ip <ip address> <setup pin code> [<nodeid>]
        connect -ble <discriminator> <setup pin code> [<nodeid>]
        connect -qr <qr code> [<nodeid>]
        connect -code <manual pairing code> [<nodeid>]

        connect command is used for establishing a rendezvous session to the device.
        currently, only connect using setupPinCode is supported.
        -qr option will connect to the first device with a matching long discriminator.

        TODO: Add more methods to connect to device (like cert for auth, and IP
              for connection)
        """

        warnings.warn(
            "This method is being deprecated. "
            "Please use the DeviceController.[ConnectBLE|CommissionIP] methods directly in the REPL", DeprecationWarning)

        try:
            args = shlex.split(line)
            if len(args) <= 1:
                print("Usage:")
                self.do_help("connect SetupPinCode")
                return

            nodeid = random.randint(1, 1000000)  # Just a random number
            if len(args) == 4:
                nodeid = int(args[3])
            print("Device is assigned with nodeid = {}".format(nodeid))

            if args[0] == "-ip" and len(args) >= 3:
                self.replHint = f"devCtrl.CommissionIP({repr(args[1])}, {int(args[2])}, {nodeid})"
                self.devCtrl.CommissionIP(args[1], int(args[2]), nodeid)
            elif args[0] == "-ble" and len(args) >= 3:
                self.replHint = f"devCtrl.ConnectBLE({int(args[1])}, {int(args[2])}, {nodeid})"
                self.devCtrl.ConnectBLE(int(args[1]), int(args[2]), nodeid)
            elif args[0] in ['-qr', '-code'] and len(args) >= 2:
                if len(args) == 3:
                    nodeid = int(args[2])
                print("Parsing QR code {}".format(args[1]))

                setupPayload = None
                if args[0] == '-qr':
                    setupPayload = SetupPayload().ParseQrCode(args[1])
                elif args[0] == '-code':
                    setupPayload = SetupPayload(
                    ).ParseManualPairingCode(args[1])

                if not int(setupPayload.attributes.get("RendezvousInformation", 0)):
                    print("No rendezvous information provided, default to all.")
                    setupPayload.attributes["RendezvousInformation"] = 0b111
                setupPayload.Print()
                self.replHint = f"devCtrl.CommissionWithCode(setupPayload={repr(setupPayload)}, nodeid={nodeid})"
                self.ConnectFromSetupPayload(setupPayload, nodeid)
            else:
                print("Usage:")
                self.do_help("connect SetupPinCode")
                return
            print(
                "Device temporary node id (**this does not match spec**): {}".format(nodeid))
        except exceptions.ChipStackException as ex:
            print(str(ex))
            return

    def do_closesession(self, line):
        """
        close-session <nodeid>

        Close any session associated with a given node ID.
        """
        try:
            parser = argparse.ArgumentParser()
            parser.add_argument('nodeid', type=int, help='Peer node ID')
            args = parser.parse_args(shlex.split(line))
            self.replHint = f"devCtrl.CloseSession({args.nodeid})"
            self.devCtrl.CloseSession(args.nodeid)
        except exceptions.ChipStackException as ex:
            print(str(ex))
        except Exception:
            self.do_help("close-session")

    def do_resolve(self, line):
        """
        resolve <nodeid>

        Resolve DNS-SD name corresponding with the given node ID and
        update address of the node in the device controller.
        """
        try:
            args = shlex.split(line)
            if len(args) == 1:
                try:
                    self.replHint = f"devCtrl.ResolveNode({int(args[0])});devCtrl.GetAddressAndPort({int(args[0])})"
                    self.devCtrl.ResolveNode(int(args[0]))
                    address = self.devCtrl.GetAddressAndPort(int(args[0]))
                    address = "{}:{}".format(
                        *address) if address else "unknown"
                    print("Current address: " + address)
                except exceptions.ChipStackException as ex:
                    print(str(ex))
            else:
                self.do_help("resolve")
        except exceptions.ChipStackException as ex:
            print(str(ex))
            return

    def wait_for_one_discovered_device(self):
        print("Waiting for device responses...")
        strlen = 100
        addrStrStorage = ctypes.create_string_buffer(strlen)
        count = 0
        maxWaitTime = 2
        while (not self.devCtrl.GetIPForDiscoveredDevice(0, addrStrStorage, strlen) and count < maxWaitTime):
            time.sleep(0.2)
            count = count + 0.2
        return count < maxWaitTime

    def wait_for_many_discovered_devices(self):
        # Discovery happens through mdns, which means we need to wait for responses to come back.
        # TODO(cecille): I suppose we could make this a command line arg. Or Add a callback when
        # x number of responses are received. For now, just 2 seconds. We can all wait that long.
        print("Waiting for device responses...")
        time.sleep(2)

    def do_discover(self, line):
        """
        discover -qr qrcode
        discover -all
        discover -l long_discriminator
        discover -s short_discriminator
        discover -v vendor_id
        discover -t device_type
        discover -c

        discover command is used to discover available devices.
        """
        try:
            arglist = shlex.split(line)
            if len(arglist) < 1:
                print("Usage:")
                self.do_help("discover")
                return
            parser = argparse.ArgumentParser()
            group = parser.add_mutually_exclusive_group()
            group.add_argument(
                '-all', help='discover all commissionable nodes and commissioners', action='store_true')
            group.add_argument(
                '-qr', help='discover commissionable nodes matching provided QR code', type=str)
            group.add_argument(
                '-l', help='discover commissionable nodes with given long discriminator', type=int)
            group.add_argument(
                '-s', help='discover commissionable nodes with given short discriminator', type=int)
            group.add_argument(
                '-v', help='discover commissionable nodes with given vendor ID', type=int)
            group.add_argument(
                '-t', help='discover commissionable nodes with given device type', type=int)
            group.add_argument(
                '-c', help='discover commissionable nodes in commissioning mode', action='store_true')
            args = parser.parse_args(arglist)
            if args.all:
                self.commissionableNodeCtrl.DiscoverCommissioners()
                self.wait_for_many_discovered_devices()
                self.commissionableNodeCtrl.PrintDiscoveredCommissioners()
                self.devCtrl.DiscoverAllCommissioning()
                self.wait_for_many_discovered_devices()
            elif args.qr is not None:
                setupPayload = SetupPayload().ParseQrCode(args.qr)
                longDiscriminator = ctypes.c_uint16(
                    int(setupPayload.attributes['Discriminator']))
                self.devCtrl.DiscoverCommissionableNodesLongDiscriminator(
                    longDiscriminator)
                self.wait_for_one_discovered_device()
            elif args.l is not None:
                self.devCtrl.DiscoverCommissionableNodesLongDiscriminator(
                    ctypes.c_uint16(args.l))
                self.wait_for_one_discovered_device()
            elif args.s is not None:
                self.devCtrl.DiscoverCommissionableNodesShortDiscriminator(
                    ctypes.c_uint16(args.s))
                self.wait_for_one_discovered_device()
            elif args.v is not None:
                self.devCtrl.DiscoverCommissionableNodesVendor(
                    ctypes.c_uint16(args.v))
                self.wait_for_many_discovered_devices()
            elif args.t is not None:
                self.devCtrl.DiscoverCommissionableNodesDeviceType(
                    ctypes.c_uint16(args.t))
                self.wait_for_many_discovered_devices()
            elif args.c is not None:
                self.devCtrl.DiscoverCommissionableNodesCommissioningEnabled()
                self.wait_for_many_discovered_devices()
            else:
                self.do_help("discover")
                return
            self.devCtrl.PrintDiscoveredDevices()
        except exceptions.ChipStackException as ex:
            print('exception')
            print(str(ex))
            return
        except Exception:
            self.do_help("discover")
            return

    def do_zcl(self, line):
        """
        To send ZCL message to device:
        zcl <cluster> <command> <nodeid> <endpoint> <groupid> [key=value]...
        To get a list of clusters:
        zcl ?
        To get a list of commands in cluster:
        zcl ? <cluster>

        Send ZCL command to device nodeid
        """
        try:
            args = shlex.split(line)
            all_commands = self.devCtrl.ZCLCommandList()
            if len(args) == 1 and args[0] == '?':
                print('\n'.join(all_commands.keys()))
            elif len(args) == 2 and args[0] == '?':
                if args[1] not in all_commands:
                    raise exceptions.UnknownCluster(args[1])
                for commands in all_commands.get(args[1]).items():
                    args = ", ".join(["{}: {}".format(argName, argType)
                                      for argName, argType in commands[1].items()])
                    print(commands[0])
                    if commands[1]:
                        print("  ", args)
                    else:
                        print("  <no arguments>")
            elif len(args) > 4:
                if args[0] not in all_commands:
                    raise exceptions.UnknownCluster(args[0])
                command = all_commands.get(args[0]).get(args[1], None)
                # When command takes no arguments, (not command) is True
                if command is None:
                    raise exceptions.UnknownCommand(args[0], args[1])
                req = eval(f"Clusters.{args[0]}.Commands.{args[1]}")(**FormatZCLArguments(args[5:], command))
                self.replHint = f"await devCtrl.SendCommand({int(args[2])}, {int(args[3])}, Clusters.{repr(req)})"
                err, res = self.devCtrl.ZCLSend(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]), FormatZCLArguments(args[5:], command), blocking=True)
                if err != 0:
                    print("Failed to receive command response: {}".format(res))
                elif res is not None:
                    print("Received command status response:")
                    print(res)
                else:
                    print("Success, no status code is attached with response.")
            else:
                self.do_help("zcl")
        except exceptions.ChipStackException as ex:
            print("An exception occurred during process ZCL command:")
            print(str(ex))
        except Exception as ex:
            print("An exception occurred during processing input:")
            traceback.print_exc()
            print(str(ex))

    def do_zclread(self, line):
        """
        To read ZCL attribute:
        zclread <cluster> <attribute> <nodeid> <endpoint> <groupid>
        """
        try:
            args = shlex.split(line)
            all_attrs = self.devCtrl.ZCLAttributeList()
            if len(args) == 1 and args[0] == '?':
                print('\n'.join(all_attrs.keys()))
            elif len(args) == 2 and args[0] == '?':
                if args[1] not in all_attrs:
                    raise exceptions.UnknownCluster(args[1])
                print('\n'.join(all_attrs.get(args[1]).keys()))
            elif len(args) == 5:
                if args[0] not in all_attrs:
                    raise exceptions.UnknownCluster(args[0])
                self.replHint = (f"await devCtrl.ReadAttribute({int(args[2])}, [({int(args[3])}, "
                                 f"Clusters.{args[0]}.Attributes.{args[1]})])")
                res = self.devCtrl.ZCLReadAttribute(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]))
                if res is not None:
                    print(repr(res))
            else:
                self.do_help("zclread")
        except exceptions.ChipStackException as ex:
            print("An exception occurred during reading ZCL attribute:")
            print(str(ex))
        except Exception as ex:
            print("An exception occurred during processing input:")
            print(str(ex))

    def do_zclwrite(self, line):
        """
        To write ZCL attribute:
        zclwrite <cluster> <attribute> <nodeid> <endpoint> <groupid> <value>
        """
        try:
            args = shlex.split(line)
            all_attrs = self.devCtrl.ZCLAttributeList()
            if len(args) == 1 and args[0] == '?':
                print('\n'.join(all_attrs.keys()))
            elif len(args) == 2 and args[0] == '?':
                if args[1] not in all_attrs:
                    raise exceptions.UnknownCluster(args[1])
                cluster_attrs = all_attrs.get(args[1], {})
                print('\n'.join(["{}: {}".format(key, cluster_attrs[key]["type"])
                                 for key in cluster_attrs.keys() if cluster_attrs[key].get("writable", False)]))
            elif len(args) == 6:
                if args[0] not in all_attrs:
                    raise exceptions.UnknownCluster(args[0])
                attribute_type = all_attrs.get(args[0], {}).get(
                    args[1], {}).get("type", None)
                self.replHint = (
                    f"await devCtrl.WriteAttribute({int(args[2])}, [({int(args[3])}, "
                    f"Clusters.{args[0]}.Attributes.{args[1]}(value={repr(ParseValueWithType(args[5], attribute_type))}))])")
                res = self.devCtrl.ZCLWriteAttribute(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]), ParseValueWithType(args[5], attribute_type))
                print(repr(res))
            else:
                self.do_help("zclwrite")
        except exceptions.ChipStackException as ex:
            print("An exception occurred during writing ZCL attribute:")
            print(str(ex))
        except Exception as ex:
            print("An exception occurred during processing input:")
            print(str(ex))

    def do_zclsubscribe(self, line):
        """
        To subscribe ZCL attribute reporting:
        zclsubscribe <cluster> <attribute> <nodeid> <endpoint> <minInterval> <maxInterval>

        To shut down a subscription:
        zclsubscribe -shutdown <subscriptionId>
        """
        try:
            args = shlex.split(line)
            all_attrs = self.devCtrl.ZCLAttributeList()
            if len(args) == 1 and args[0] == '?':
                print('\n'.join(all_attrs.keys()))
            elif len(args) == 2 and args[0] == '?':
                if args[1] not in all_attrs:
                    raise exceptions.UnknownCluster(args[1])
                cluster_attrs = all_attrs.get(args[1], {})
                print('\n'.join([key for key in cluster_attrs.keys(
                ) if cluster_attrs[key].get("reportable", False)]))
            elif len(args) == 6:
                if args[0] not in all_attrs:
                    raise exceptions.UnknownCluster(args[0])
                res = self.devCtrl.ZCLSubscribeAttribute(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]), int(args[5]))
                self.replHint = (f"sub = await devCtrl.ReadAttribute({int(args[2])}, [({int(args[3])}, "
                                 f"Clusters.{args[0]}.Attributes.{args[1]})], reportInterval=({int(args[4])}, {int(args[5])}))")
                print(res.GetAllValues())
                print(f"Subscription Established: {res}")
            elif len(args) == 2 and args[0] == '-shutdown':
                subscriptionId = int(args[1], base=0)
                self.replHint = "You can call sub.Shutdown() (sub is the return value of ReadAttribute() called before)"
                self.devCtrl.ZCLShutdownSubscription(subscriptionId)
            else:
                self.do_help("zclsubscribe")
        except exceptions.ChipStackException as ex:
            print("An exception occurred during configuring reporting of ZCL attribute:")
            print(str(ex))
        except Exception as ex:
            print("An exception occurred during processing input:")
            print(str(ex))

    def do_setpairingwificredential(self, line):
        """
        set-pairing-wifi-credential ssid credentials
        """
        try:
            args = shlex.split(line)
            if len(args) < 2:
                print("Usage:")
                self.do_help("set-pairing-wifi-credential")
                return
            self.devCtrl.SetWiFiCredentials(
                args[0], args[1])
            self.replHint = f"devCtrl.SetWiFiCredentials({repr(args[0])}, {repr(args[1])})"
        except Exception as ex:
            print(str(ex))
            return

    def do_setpairingthreadcredential(self, line):
        """
        set-pairing-thread-credential threadOperationalDataset
        """
        try:
            args = shlex.split(line)
            if len(args) < 1:
                print("Usage:")
                self.do_help("set-pairing-thread-credential")
                return
            self.replHint = f"devCtrl.SetThreadOperationalDataset(bytes.fromhex({repr(args[0])}))"
            self.devCtrl.SetThreadOperationalDataset(bytes.fromhex(args[0]))
        except Exception as ex:
            print(str(ex))
            return

    def do_opencommissioningwindow(self, line):
        """
        open-commissioning-window <nodeid> [options]

        Options:
          -t  Timeout (in seconds)
          -o  Option  [TokenWithRandomPIN = 1, TokenWithProvidedPIN = 2]
          -d  Discriminator Value
          -i  Iteration

          This command is used by a current Administrator to instruct a Node to go into commissioning mode
        """
        try:
            arglist = shlex.split(line)

            if len(arglist) <= 1:
                print("Usage:")
                self.do_help("open-commissioning-window")
                return
            parser = argparse.ArgumentParser()
            parser.add_argument(
                "-t", type=int, default=0, dest='timeout')
            parser.add_argument(
                "-o", type=int, default=1, dest='option')
            parser.add_argument(
                "-i", type=int, default=0, dest='iteration')
            parser.add_argument(
                "-d", type=int, default=0, dest='discriminator')
            args = parser.parse_args(arglist[1:])

            if args.option < 1 or args.option > 2:
                print("Invalid option specified!")
                raise ValueError("Invalid option specified")

            self.replHint = (f"devCtrl.OpenCommissioningWindow(nodeid={int(arglist[0])}, timeout={args.timeout}, "
                             f"iteration={args.iteration}, discriminator={args.discriminator}, option={args.option})")

            self.devCtrl.OpenCommissioningWindow(
                int(arglist[0]), args.timeout, args.iteration, args.discriminator, args.option)

        except exceptions.ChipStackException as ex:
            print(str(ex))
            return
        except Exception:
            self.do_help("open-commissioning-window")
            return

    def do_getfabricid(self, line):
        """
          get-fabricid

          Read the current Compressed Fabric Id of the controller device, return 0 if not available.
        """
        try:
            args = shlex.split(line)

            if (len(args) > 0):
                print("Unexpected argument: " + args[1])
                return

            compressed_fabricid = self.devCtrl.GetCompressedFabricId()
            raw_fabricid = self.devCtrl.fabricId

            self.replHint = "devCtrl.GetCompressedFabricId(), devCtrl.fabricId"
        except exceptions.ChipStackException as ex:
            print("An exception occurred during reading FabricID:")
            print(str(ex))
            return

        print("Get fabric ID complete")

        print("Raw Fabric ID: 0x{:016x}".format(raw_fabricid)
              + " (" + str(raw_fabricid) + ")")

        print("Compressed Fabric ID: 0x{:016x}".format(compressed_fabricid)
              + " (" + str(compressed_fabricid) + ")")

    def do_history(self, line):
        """
        history

        Show previously executed commands.
        """

        try:
            import readline

            h = readline.get_current_history_length()
            for n in range(1, h + 1):
                print(readline.get_history_item(n))
        except ImportError:
            pass

    def do_h(self, line):
        self.do_history(line)

    def do_exit(self, line):
        return True

    def do_quit(self, line):
        return True

    def do_q(self, line):
        return True

    def do_EOF(self, line):
        print()
        return True

    def emptyline(self):
        pass


def main():
    optParser = OptionParser()
    optParser.add_option(
        "-r",
        "--rendezvous-addr",
        action="store",
        dest="rendezvousAddr",
        help="Device rendezvous address",
        metavar="<ip-address>",
    )
    optParser.add_option(
        "-n",
        "--controller-nodeid",
        action="store",
        dest="controllerNodeId",
        default=1,
        type='int',
        help="Controller node ID",
        metavar="<nodeid>",
    )

    if sys.platform.startswith("linux"):
        optParser.add_option(
            "-b",
            "--bluetooth-adapter",
            action="store",
            dest="bluetoothAdapter",
            default="hci0",
            type="str",
            help="Controller bluetooth adapter ID, use --no-ble to disable bluetooth functions.",
            metavar="<bluetooth-adapter>",
        )
        optParser.add_option(
            "--no-ble",
            action="store_true",
            dest="disableBluetooth",
            help="Disable bluetooth, calling BLE related feature with this flag results in undefined behavior.",
        )
    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    if len(remainingArgs) != 0:
        print("Unexpected argument: %s" % remainingArgs[0])
        sys.exit(-1)

    adapterId = None
    if sys.platform.startswith("linux"):
        if options.disableBluetooth:
            adapterId = None
        elif not options.bluetoothAdapter.startswith("hci"):
            print(
                "Invalid bluetooth adapter: {}, adapter name looks like hci0, hci1 etc.")
            sys.exit(-1)
        else:
            try:
                adapterId = int(options.bluetoothAdapter[3:])
            except ValueError:
                print(
                    "Invalid bluetooth adapter: {}, adapter name looks like hci0, hci1 etc.")
                sys.exit(-1)
    native.Init(bluetoothAdapter=adapterId)
    try:
        devMgrCmd = DeviceMgrCmd(rendezvousAddr=options.rendezvousAddr,
                                 controllerNodeId=options.controllerNodeId, bluetoothAdapter=adapterId)
    except Exception as ex:
        print(ex)
        print("Failed to bringup CHIPDeviceController CLI")
        sys.exit(1)

    print("Chip Device Controller Shell")
    if options.rendezvousAddr:
        print("Rendezvous address set to %s" % options.rendezvousAddr)

    # Adapter ID will always be 0
    if adapterId != 0:
        print("Bluetooth adapter set to hci{}".format(adapterId))
    print()

    try:
        devMgrCmd.cmdloop()
    except KeyboardInterrupt:
        print("\nQuitting")

    sys.exit(0)


if __name__ == "__main__":
    print("""
    chip-device-ctrl will be deprecated and will be removed in the future. Please try chip-repl, which provides a lot of features.

    - Multi-fabric support,
    - Better complex type support for sending commands,
    - Native command highlight,
    - Parallel commands with asyncio,
    - Writing complex logic inline.

    You can still use chip-device-ctrl as usual for now, and you will learn how to do the same thing in chip-repl.

    Feel free to file an issue if some features are not supported by chip-repl yet.
    """)
    main()
