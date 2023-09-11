#!/usr/bin/env python

#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

import sys

import IPython
from traitlets.config import Config


def main():
    c = Config()
    c.InteractiveShellApp.exec_lines = [
        "import pkgutil",
        "module = pkgutil.get_loader('chip.ChipReplStartup')",
        "%run {module.path} " + " ".join(sys.argv[1:])
    ]

    sys.argv = [sys.argv[0]]

    IPython.start_ipython(config=c)


if __name__ == "__main__":
    main()
