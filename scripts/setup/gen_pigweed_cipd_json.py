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
import json

_LIST_OF_PACKAGES_TO_EXCLUDE = ['fuchsia/third_party/rust/']


def include_package(package: dict) -> bool:
    if 'path' in package:
        path = package['path']
        exclusion_match = any(
            path.startswith(package_to_exclude)
            for package_to_exclude in _LIST_OF_PACKAGES_TO_EXCLUDE
        )
        if exclusion_match:
            return False
    return True


def generate_new_cipd_package_json(input, output):
    with open(input) as ins:
        packages = json.load(ins)
        file_packages = packages.get('packages')
        new_file_packages = [x for x in file_packages if include_package(x)]

    new_packages = {'packages': new_file_packages}
    with open(output, 'w') as f:
        json.dump(new_packages, f, indent=2)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        '--input', '-i', required=True
    )
    parser.add_argument(
        '--output', '-o', required=True
    )
    generate_new_cipd_package_json(**vars(parser.parse_args()))


if __name__ == '__main__':
    main()
