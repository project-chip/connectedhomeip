#!/usr/bin/env python

#
#    Copyright (c) 2021 Project CHIP Authors
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

import IPython
import chip
import chip.logging
import coloredlogs
import logging
from traitlets.config import Config
from rich import print
from rich import pretty
from rich import inspect
import builtins
import argparse
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--storagepath", help="Path to persistent storage configuration file (default: /tmp/repl-storage.json)",
                        action="store", default="/tmp/repl-storage.json")

    args = parser.parse_args()

    c = Config()
    c.InteractiveShellApp.exec_lines = [
        "import pkgutil",
        "module = pkgutil.get_loader('chip.ChipReplStartup')",
        "%run {module.path} --storagepath " + f"{args.storagepath}"
    ]

    sys.argv = [sys.argv[0]]

    IPython.start_ipython(config=c)


if __name__ == "__main__":
    main()
