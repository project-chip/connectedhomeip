# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import shlex


class PrintOnlyRunner:
    def __init__(self, output_file, root: str):
        self.output_file = output_file
        self.dry_run = True
        self.root = root

    def StartCommandExecution(self):
        self.output_file.write(
            "# Commands will be run in CHIP project root.\n")
        self.output_file.write('cd "%s"\n\n' % self.root)

    def Run(self, cmd, title=None):
        if title:
            self.output_file.write("# " + title + "\n")

        self.output_file.write(
            " ".join([shlex.quote(part) for part in cmd]) + "\n")

        self.output_file.write("\n")
