#
#    Copyright (c) 2021 Project CHIP Authors
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

from chip.server import (
    GetLibraryHandle,
    NativeLibraryHandleMethodArguments,
    PostAttributeChangeCallback,
)

from chip.exceptions import ChipStackError

from ctypes import CFUNCTYPE, c_char_p, c_int32, c_uint8

import sys
import os

import textwrap
import string

from cmd import Cmd

import asyncio
import threading

from dali.driver.hid import tridonic
from dali.gear.general import RecallMaxLevel, Off, DAPC
from dali.address import Broadcast, Short

dali_loop = None
dev = None


async def dali_on(is_on: bool):
    global dali_loop
    global dev

    await dev.connected.wait()
    if (is_on):
        await dev.send(RecallMaxLevel(Broadcast()))
    else:
        await dev.send(Off(Broadcast()))


async def dali_level(level: int):
    global dali_loop
    global dev

    await dev.connected.wait()
    await dev.send(DAPC(Broadcast(), level))


def daliworker():
    global dali_loop
    global dev

    dali_loop = asyncio.new_event_loop()
    dev = tridonic("/dev/dali/daliusb-*", glob=True, loop=dali_loop)
    dev.connect()

    asyncio.set_event_loop(dali_loop)
    dali_loop.run_forever()


class LightingMgrCmd(Cmd):
    def __init__(self, rendezvousAddr=None, controllerNodeId=0, bluetoothAdapter=None):
        self.lastNetworkId = None

        Cmd.__init__(self)

        Cmd.identchars = string.ascii_letters + string.digits + "-"

        if sys.stdin.isatty():
            self.prompt = "chip-lighting > "
        else:
            self.use_rawinput = 0
            self.prompt = ""

        LightingMgrCmd.command_names.sort()

        self.historyFileName = os.path.expanduser("~/.chip-lighting-history")

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
        "help"
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
            for name in LightingMgrCmd.command_names
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
            self.prompt = "chip-lighting > "
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
        """
        help

        Print the help
        """
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
                LightingMgrCmd.command_names,
                15,
                80,
            )


@PostAttributeChangeCallback
def attributeChangeCallback(
    endpoint: int,
    clusterId: int,
    attributeId: int,
    manufacturerCode: int,
    xx_type: int,
    size: int,
    value: bytes,
):
    global dali_loop
    if endpoint == 1:
        if clusterId == 6 and attributeId == 0:
            if len(value) == 1 and value[0] == 1:
                # print("[PY] light on")
                future = asyncio.run_coroutine_threadsafe(
                    dali_on(True), dali_loop)
                future.result()
            else:
                # print("[PY] light off")
                future = asyncio.run_coroutine_threadsafe(
                    dali_on(False), dali_loop)
                future.result()
        elif clusterId == 8 and attributeId == 0:
            if len(value) == 2:
                # print("[PY] level {}".format(value[0]))
                future = asyncio.run_coroutine_threadsafe(
                    dali_level(value[0]), dali_loop)
                future.result()
            else:
                print("[PY] no level")
        else:
            # print("[PY] [ERR] unhandled cluster {} or attribute {}".format(
            #     clusterId, attributeId))
            pass
    else:
        print("[PY] [ERR] unhandled endpoint {} ".format(endpoint))


class Lighting:
    def __init__(self):
        self.chipLib = GetLibraryHandle(attributeChangeCallback)


if __name__ == "__main__":
    l = Lighting()

    lightMgrCmd = LightingMgrCmd()
    print("Chip Lighting Device Shell")
    print()

    print("Starting DALI async")
    threads = []
    t = threading.Thread(target=daliworker)
    threads.append(t)
    t.start()

    try:
        lightMgrCmd.cmdloop()
    except KeyboardInterrupt:
        print("\nQuitting")

    sys.exit(0)
