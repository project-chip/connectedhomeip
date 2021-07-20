#!/usr/bin/env python3
"""
Copyright (c) 2020 Project CHIP Authors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import sys
import subprocess
import logging
from multiprocessing.connection import Listener, Client

log = logging.getLogger()
log.setLevel(logging.DEBUG)

sh = logging.StreamHandler()
sh.setFormatter(
    logging.Formatter(
        '%(asctime)s [%(name)s] %(levelname)s %(message)s'))
log.addHandler(sh)


class ShellCommand:
    def __init__(self, args=None):
        self._args = args

    def __call__(self):
        if not self._args:
            return [1, "Cannot spwan background process"]
        log.info("Will run command: {}".format(self._args))
        subprocess.Popen(
            self._args, stdout=sys.stdout, stderr=sys.stderr)
        return [0, ""]


class InvalidCommand:
    def __init__(self, args=None):
        self._args = args

    def __call__(self):
        return [1, "Invalid command"]


address = "/tmp/cirque-helper.socket"


def CommandFactory(args):
    commands = {
        "run": ShellCommand
    }
    if len(args) == 0:
        return InvalidCommand()
    return commands.get(args[0], InvalidCommand)(args[1:])


def ServerMain(args):
    extraOptions = {
        "otbr-agent": ShellCommand(["otbr-agent", "-I", "wpan0", "spinel+hdlc+uart:///dev/ttyUSB0"])
    }

    for extraOption in args:
        cmd = extraOptions.get(extraOption, InvalidCommand())
        cmd()

    with Listener(address) as listener:
        while True:
            with listener.accept() as conn:
                log.info("Received connection")
                cmd = CommandFactory(conn.recv())
                conn.send(cmd())


def ClientMain(args):
    if len(args) == 0:
        sys.exit(1)
    with Client(address) as conn:
        conn.send(args)
        res = conn.recv()
        print(res)
        if not res:
            sys.exit(0)
        sys.exit(res[0])


if __name__ == "__main__":
    if len(sys.argv) < 2:
        exit(1)
    elif sys.argv[1] == "server":
        ServerMain(sys.argv[2:])
    else:
        ClientMain(sys.argv[1:])
