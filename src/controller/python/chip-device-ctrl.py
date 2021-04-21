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

from __future__ import absolute_import
from __future__ import print_function
from chip import ChipDeviceCtrl
from chip import exceptions
import sys
import os
import platform
import random
from optparse import OptionParser, OptionValueError
import shlex
import base64
import textwrap
import string
import re
from cmd import Cmd
from chip.ChipBleUtility import FAKE_CONN_OBJ_VALUE
from chip.setup_payload import SetupPayload

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


def FormatZCLArguments(args, command):
    commandArgs = {}
    for kvPair in args:
        if kvPair.find("=") < 0:
            raise ParsingError("Argument should in key=value format")
        key, value = kvPair.split("=", 1)
        valueType = command.get(key, None)
        if valueType == 'int':
            commandArgs[key] = int(value)
        elif valueType == 'str':
            commandArgs[key] = value
        elif valueType == 'bytes':
            commandArgs[key] = ParseEncodedString(value)
    return commandArgs


class DeviceMgrCmd(Cmd):
    def __init__(self, rendezvousAddr=None, controllerNodeId=0, bluetoothAdapter=None):
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

        self.devCtrl = ChipDeviceCtrl.ChipDeviceController(
            controllerNodeId=controllerNodeId, bluetoothAdapter=bluetoothAdapter)

        # If we are on Linux and user selects non-default bluetooth adapter.
        if sys.platform.startswith("linux") and (bluetoothAdapter is not None):
            self.bleMgr = BleManager(self.devCtrl)
            self.bleMgr.ble_adapter_select("hci{}".format(bluetoothAdapter))

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
        "resolve",
        "zcl",
        "zclread",
        "zclconfigure",

        "set-pairing-wifi-credential",
        "set-pairing-thread-credential",
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

    def do_closeble(self, line):
        """
        close-ble

        Close the ble connection to the device.
        """

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

        print("Done.")

    def do_setuppayload(self, line):
        """
        setup-payload parse-manual <manual-pairing-code>
        setup-payload parse-qr <qr-code-payload>
        """
        try:
            args = shlex.split(line)
            if (len(args) != 2) or (args[0] not in ("parse-manual", "parse-qr")):
                self.do_help("setup-payload")
                return

            if args[0] == "parse-manual":
                SetupPayload().ParseManualPairingCode(args[1]).Print()

            if args[0] == "parse-qr":
                SetupPayload().ParseQrCode(args[1]).Print()

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

    def do_connect(self, line):
        """
        connect -ip <ip address> <setup pin code> [<nodeid>]
        connect -ble <discriminator> <setup pin code> [<nodeid>]

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

            nodeid = random.randint(1, 1000000)  # Just a random number
            if len(args) == 4:
                nodeid = int(args[3])
            print("Device is assigned with nodeid = {}".format(nodeid))

            if args[0] == "-ip" and len(args) >= 3:
                self.devCtrl.ConnectIP(args[1].encode(
                    "utf-8"), int(args[2]), nodeid)
            elif args[0] == "-ble" and len(args) >= 3:
                self.devCtrl.ConnectBLE(int(args[1]), int(args[2]), nodeid)
            else:
                print("Usage:")
                self.do_help("connect SetupPinCode")
                return
            print(
                "Device temporary node id (**this does not match spec**): {}".format(nodeid))
        except exceptions.ChipStackException as ex:
            print(str(ex))
            return

    def do_resolve(self, line):
        """
        resolve <fabricid> <nodeid>

        Resolve DNS-SD name corresponding with the given fabric and node IDs and
        update address of the node in the device controller.
        """
        try:
            args = shlex.split(line)
            if len(args) == 2:
                err = self.devCtrl.ResolveNode(int(args[0]), int(args[1]))
                if err == 0:
                    address = self.devCtrl.GetAddressAndPort(int(args[1]))
                    address = "{}:{}".format(
                        *address) if address else "unknown"
                    print("Current address: " + address)
            else:
                self.do_help("resolve")
        except exceptions.ChipStackException as ex:
            print(str(ex))
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
                if command == None:
                    raise exceptions.UnknownCommand(args[0], args[1])
                err, res = self.devCtrl.ZCLSend(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]), FormatZCLArguments(args[5:], command), blocking=True)
                if err != 0:
                    print("Failed to receive command response: {}".format(res))
                elif res != None:
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
            import traceback
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
                print('\n'.join(all_attrs.get(args[1])))
            elif len(args) == 5:
                if args[0] not in all_attrs:
                    raise exceptions.UnknownCluster(args[0])
                self.devCtrl.ZCLReadAttribute(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]))
            else:
                self.do_help("zclread")
        except exceptions.ChipStackException as ex:
            print("An exception occurred during reading ZCL attribute:")
            print(str(ex))
        except Exception as ex:
            print("An exception occurred during processing input:")
            print(str(ex))

    def do_zclconfigure(self, line):
        """
        To configure ZCL attribute reporting:
        zclconfigure <cluster> <attribute> <nodeid> <endpoint> <minInterval> <maxInterval> <change>
        """
        try:
            args = shlex.split(line)
            all_attrs = self.devCtrl.ZCLAttributeList()
            if len(args) == 1 and args[0] == '?':
                print('\n'.join(all_attrs.keys()))
            elif len(args) == 2 and args[0] == '?':
                if args[1] not in all_attrs:
                    raise exceptions.UnknownCluster(args[1])
                print('\n'.join(all_attrs.get(args[1])))
            elif len(args) == 7:
                if args[0] not in all_attrs:
                    raise exceptions.UnknownCluster(args[0])
                self.devCtrl.ZCLConfigureAttribute(args[0], args[1], int(
                    args[2]), int(args[3]), int(args[4]), int(args[5]), int(args[6]))
            else:
                self.do_help("zclconfigure")
        except exceptions.ChipStackException as ex:
            print("An exception occurred during configuring reporting of ZCL attribute:")
            print(str(ex))
        except Exception as ex:
            print("An exception occurred during processing input:")
            print(str(ex))

    def do_setpairingwificredential(self, line):
        """
        set-pairing-wifi-credential

        Removed, use network commissioning cluster instead.
        """
        print("Pairing WiFi Credential is nolonger available, use NetworkCommissioning cluster instead.")

    def do_setpairingthreadcredential(self, line):
        """
        set-pairing-thread-credential

        Removed, use network commissioning cluster instead.
        """
        print("Pairing Thread Credential is nolonger available, use NetworkCommissioning cluster instead.")

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
        default=0,
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
            help="Controller bluetooth adapter ID",
            metavar="<bluetooth-adapter>",
        )
    (options, remainingArgs) = optParser.parse_args(sys.argv[1:])

    if len(remainingArgs) != 0:
        print("Unexpected argument: %s" % remainingArgs[0])
        sys.exit(-1)

    adapterId = None
    if sys.platform.startswith("linux"):
        if not options.bluetoothAdapter.startswith("hci"):
            print(
                "Invalid bluetooth adapter: {}, adapter name looks like hci0, hci1 etc.")
            sys.exit(-1)
        else:
            try:
                adapterId = int(options.bluetoothAdapter[3:])
            except:
                print(
                    "Invalid bluetooth adapter: {}, adapter name looks like hci0, hci1 etc.")
                sys.exit(-1)

    devMgrCmd = DeviceMgrCmd(rendezvousAddr=options.rendezvousAddr,
                             controllerNodeId=options.controllerNodeId, bluetoothAdapter=adapterId)
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
    main()
