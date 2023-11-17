#!/usr/bin/env python3

# Copyright (c) 2023 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
import argparse


def _make_paths_absolute(gni_file: str, root: str):
    with open(gni_file, "rt", encoding="utf8") as f:
        data = f.read()

    # Data will contain key/value pairs like:
    #   pw_env_setup_CIPD_PIGWEED = "//../home/vscode/pigweed/env/cipd/packages/pigweed"
    #
    # Looking to replace relative paths (starting with "//../")
    # to absolute paths from the root
    if not root.endswith("/"):
        root = root + "/"

    new_data = data.replace('"//../', f'"{root}../')

    if new_data == data:
        return

    with open(gni_file, "wt", encoding="utf8") as f:
        f.write(new_data)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        '--root', '-r', required=True,
        help="Set the root used by the build (usually CHIP_ROOT)"
    )
    parser.add_argument('gni_file', type=str, help="GNI file to process")
    _make_paths_absolute(**vars(parser.parse_args()))


if __name__ == '__main__':
    main()
