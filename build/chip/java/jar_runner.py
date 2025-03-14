#!/usr/bin/env python
# Copyright (c) 2020 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Wrapper script to run javac command as an action with gn."""

import os
import subprocess
import sys

EXIT_SUCCESS = 0
EXIT_FAILURE = 1


def IsExecutable(path):
    """Returns whether file at |path| exists and is executable.

    Args:
      path: absolute or relative path to test.

    Returns:
      True if the file at |path| exists, False otherwise.
    """
    return os.path.isfile(path) and os.access(path, os.X_OK)


def FindCommand(command):
    """Looks up for |command| in PATH.

    Args:
      command: name of the command to lookup, if command is a relative or
        absolute path (i.e. contains some path separator) then only that
        path will be tested.

    Returns:
      Full path to command or None if the command was not found.

      On Windows, this respects the PATHEXT environment variable when the
      command name does not have an extension.
    """
    fpath, _ = os.path.split(command)
    if fpath:
        if IsExecutable(command):
            return command

    if sys.platform == 'win32':
        # On Windows, if the command does not have an extension, cmd.exe will
        # try all extensions from PATHEXT when resolving the full path.
        command, ext = os.path.splitext(command)
        if not ext:
            exts = os.environ['PATHEXT'].split(os.path.pathsep)
        else:
            exts = [ext]
    else:
        exts = ['']

    for path in os.environ['PATH'].split(os.path.pathsep):
        for ext in exts:
            path = os.path.join(path, command) + ext
            if IsExecutable(path):
                return path

    return None


def main():
    java_home = FindCommand('jar')
    if not java_home:
        sys.stderr.write('jar: command not found\n')
        sys.exit(EXIT_FAILURE)

    args = sys.argv[1:]
    if len(args) < 1:
        sys.stderr.write('usage: %s [jar_args]...\n' % sys.argv[0])
        sys.exit(EXIT_FAILURE)

    return subprocess.check_call([java_home] + args)


if __name__ == '__main__':
    sys.exit(main())
