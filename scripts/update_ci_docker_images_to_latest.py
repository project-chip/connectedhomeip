#!/usr/bin/env python3
#
# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import os
import fileinput

WORKFLOWS_DIR = '.github/workflows/'
GCB_DIR = 'integrations/cloudbuild/'
VERSION_FILE = 'integrations/docker/images/chip-build/version'

def get_version(version_file: str) -> str:
    """Reads the provided images version file and returns the current images version."""
    with open(version_file) as f:
        l = f.readline()
        return l[:l.index(' ')]

def update_line(version: str, line: str, end_quote=False) -> str:
    """Replaces the end of a string after the last colon with the specified version.
    end_quote ends the line with a closing double quote.
    """
    if not end_quote:
        return line[:line.rindex(':')+1]+version
    else:
        return line[:line.rindex(':')+1]+version+'"'

def update_file(version: str, file: str, search_term: str, end_quote=False) -> None:
    """Reads the specified file and looks for lines containing search_term.
    If a line containts search_term, update it and specify if closing double quotes are needed.
    fileinput.input replaces lines with std out.
    print(update_line... here updates the file.
    print(line, end='') maintains the existing line.
    """
    with fileinput.input(file, inplace=True) as f:
        for line in f:
            if search_term in line:
                print(update_line(version, line, end_quote=end_quote))
            else:
                print(line, end='')

def update_workflows(version: str, directory: str) -> None:
    """Iterate over workflow configs and update.
    Closing quotes are not needed.
    """
    for workflow in os.listdir(directory):
        update_file(version,
            os.path.join(directory, workflow),
            'image:')

def update_gcb_configs(version: str, directory: str) -> None:
    """Iterate over GCB configs and update.
    Closing quotes ARE needed.
    """
    for item in os.listdir(directory):
        item_path = os.path.join(directory, item)
        if str(item_path).endswith('.yaml'):
            update_file(version, item_path, 'name: "connectedhomeip', end_quote=True)

def main() -> None:
    """Update workflow configs in WORKFLOWS_DIR
    and gcb configs in GCB_DIR
    with the version in VERSION_FILE.
    Run this script from the root of the repository.
    """
    version = get_version(VERSION_FILE)
    update_workflows(version, WORKFLOWS_DIR)
    update_gcb_configs(version, GCB_DIR)

if __name__ == '__main__':
    main()
