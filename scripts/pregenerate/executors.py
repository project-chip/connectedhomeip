# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

import logging
import os
import shlex
import subprocess


class ShellRunner:
    def run(self, cmd, cwd=None):
        logging.debug(f"Executing {cmd}")
        subprocess.check_call(cmd, cwd=cwd)

    def ensure_directory_exists(self, dir):
        os.makedirs(dir, exist_ok=True)


class DryRunner:
    def run(self, cmd, cwd=None):
        if cwd:
            logging.info(f"DRY-RUN: {shlex.join(cmd)} in {cwd}")
        else:
            logging.info(f"DRY-RUN: {shlex.join(cmd)}")

    def ensure_directory_exists(self, dir):
        logging.info(f"DRY-RUN: mkdir {dir}")
