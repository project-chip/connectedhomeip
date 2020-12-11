#!/usr/bin/env python

#
#    Copyright (c) 2020 Project CHIP Authors
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

from __future__ import absolute_import
from __future__ import print_function
from chip import ChipStack
from chip import ChipDeviceCtrl
from builtins import range
import sys
import os
import platform
from optparse import OptionParser, OptionValueError
import shlex
import base64
import textwrap
import string
from cmd import Cmd
from six.moves import range
from chip.ChipBleUtility import FAKE_CONN_OBJ_VALUE

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


class DeviceMgrCmd(Cmd):
    def __init__(self, rendezvousAddr=None):
        self.lastNetworkId = None

        Cmd.__init__(self)

        Cmd.identchars = string.ascii_letters + string.digits + "-"

        if sys.stdin.isatty():
            self.prompt = "chip-device-ctrl > "
        else:
            self.use_rawinput = 0
            self.prompt = ""

        DeviceMgrCmd.command_names.sort()

        self.bleMgr = None

        self.devCtrl = ChipDeviceCtrl.ChipDeviceController(112233)

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
        "close",
        "btp-connect",
        "ble-scan",
        "ble-connect",
        "ble-disconnect",
        "ble-scan-connect",
        "ble-adapter-select",
        "ble-adapter-print",
        "ble-debug-log",

        "connect",
        "set-pairing-wifi-credential",
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

    def do_close(self, line):
        """
        close

        Close the connection to the device.
        """

        args = shlex.split(line)

        if len(args) != 0:
            print("Usage:")
            self.do_help("close")
            return

        try:
            self.devCtrl.Close()
        except ChipStack.ChipStackException as ex:
            print(str(ex))

    def do_setlogoutput(self, line):
        """
        set-log-output [ none | error | progress | detail ]

        Set the level of Chip logging output.
        """

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
        except ChipStack.ChipStackException as ex:
            print(str(ex))
            return

        print("Done.")

    def do_bleadapterselect(self, line):
        """
        ble-adapter-select

        Start BLE adapter select.
        """
        if sys.platform.startswith("linux"):
            if not self.bleMgr:
                self.bleMgr = BleManager(self.devCtrl)

            self.bleMgr.ble_adapter_select(line)
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

    def do_bleconnect(self, line):
        """
        ble-connect <device-name>
        ble-connect <mac-address (linux only)>
        ble-connect <device-uuid>
        ble-connect <discriminator>

        Connect to a BLE peripheral identified by line.
        """

        if not self.bleMgr:
            self.bleMgr = BleManager(self.devCtrl)
        self.bleMgr.connect(line)

        return

    def do_blescanconnect(self, line):
        """
        ble-scan-connect <device-name>
        ble-scan-connect <mac-address (linux only)>
        ble-scan-connect <device-uuid>
        ble-scan-connect <discriminator>

        Scan and connect to a BLE peripheral identified by line.
        """

        if not self.bleMgr:
            self.bleMgr = BleManager(self.devCtrl)

        self.bleMgr.scan_connect(line)

        return

    def do_bledisconnect(self, line):
        """
        ble-disconnect

        Disconnect from a BLE peripheral.
        """

        if not self.bleMgr:
            self.bleMgr = BleManager(self.devCtrl)

        self.bleMgr.disconnect()

        return

    def do_btpconnect(self, line):
        """
        connect .

        """
        try:
            self.devCtrl.ConnectBle(bleConnection=FAKE_CONN_OBJ_VALUE)
        except ChipStack.ChipStackException as ex:
            print(str(ex))
            return

        print("BTP Connected")

    def do_connect(self, line):
        """
        connect -ip <ip address> <setup pin code>
        connect -ble <setup pin code>

        connect command is used for establishing a rendezvous session to the device.
        currently, only connect using setupPinCode is supported.

        TODO: Add more methods to connect to device (like cert for auth, and IP
              for connection)
        """

        try:
            args = shlex.split(line)
            if len(args) <= 1:
                print("Usage:")
                self.do_help("connect SetupPinCode")
                return
            if args[0] == "-ip" and len(args) == 3:
                self.devCtrl.ConnectIP(args[1].encode("utf-8"), int(args[2]))
            elif args[0] == "-ble" and len(args) == 2:
                self.devCtrl.Connect(FAKE_CONN_OBJ_VALUE, int(args[1]))
            else:
                print("Usage:")
                self.do_help("connect SetupPinCode")
                return
        except ChipStack.ChipStackException as ex:
            print(str(ex))
            return
        print("Connected")

    def do_setpairingwificredential(self, line):
        """
        set-pairing-wifi-credential

        Set WiFi credential for pairing, will sent to device
        """
        try:
            args = shlex.split(line)
            self.devCtrl.SetWifiCredential(args[0], args[1])
        except ChipStack.ChipStackException as ex:
            print(str(ex))
            return

        print("WiFi credential set")

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
    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    if len(remainingArgs) != 0:
        print("Unexpected argument: %s" % remainingArgs[0])
        sys.exit(-1)

    devMgrCmd = DeviceMgrCmd(rendezvousAddr=options.rendezvousAddr)
    print("Chip Device Controller Shell")
    if options.rendezvousAddr:
        print("Rendezvous address set to %s" % options.rendezvousAddr)
    print()

    try:
        devMgrCmd.cmdloop()
    except KeyboardInterrupt:
        print("\nQuitting")

    sys.exit(0)


if __name__ == "__main__":
    main()
