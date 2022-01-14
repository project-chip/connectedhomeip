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
    parser.add_argument(
        "-n", "--noautoinit", help="Disable the default auto-initialization of the controller", action="store_true")
    args = parser.parse_args()

    c = Config()
    c.InteractiveShellApp.exec_lines = [
        "from rich import print",
        "from rich.pretty import pprint",
        "from rich import pretty",
        "from rich import inspect",
        "from rich.console import Console",
        "import logging",
        "from chip import ChipDeviceCtrl",
        "import chip.clusters as Clusters",
        "import coloredlogs",
        "import chip.logging",
        "import argparse",
        "from chip.ChipReplStartup import *",
        "ReplInit()",
    ]

    if (not(args.noautoinit)):
        c.InteractiveShellApp.exec_lines.extend([
            "import builtins",
            "devCtrl = ChipDeviceCtrl.ChipDeviceController(controllerNodeId=12344321)",
            "builtins.devCtrl = devCtrl",
            "console = Console()",
            "console.print('\\n\\n[blue]CHIP Device Controller has been initialized, and is available as [bold red]devCtrl')"
        ])

    sys.argv = [sys.argv[0]]

    IPython.start_ipython(config=c)


if __name__ == "__main__":
    main()
