#!/usr/bin/env python3
#
#    Copyright (c) 2021 Project CHIP Authors
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

import json
import os

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


def readClangRevision(jsonFileName):
    with open(os.path.join(CHIP_ROOT_DIR, jsonFileName)) as file:
        data = json.load(file)

    packages = data['packages']
    for package in packages:
        if package['path'].startswith('fuchsia/third_party/clang/'):
            return package['tags']

    raise Exception('Could not find clang package in %s' % jsonFileName)


pigweed_revision = readClangRevision('third_party/pigweed/repo/pw_env_setup/py/pw_env_setup/cipd_setup/pigweed.json')

our_file = 'scripts/setup/clang.json'
our_revision = readClangRevision(our_file)

if our_revision != pigweed_revision:
    raise Exception('In %s, "%s" should be changed to "%s"' % (our_file, our_revision, pigweed_revision))
