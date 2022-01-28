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

from pathlib import Path
import os
import logging
import subprocess
import re

import chiptest.linux
import chiptest.runner

from .test_definition import TestTarget, TestDefinition, ApplicationPaths


def AllTests(chip_tool: str):
    """Executes `chip_tool` binary to see what tests are available.
    """

    result = subprocess.run([chip_tool, 'tests', 'list'], capture_output=True)

    for name in result.stdout.decode('utf8').split('\n'):
        if not name:
            continue

        if name.startswith('TV_'):
            target = TestTarget.TV
        elif name.startswith('DL_'):
            target = TestTarget.DOOR_LOCK
        else:
            target = TestTarget.ALL_CLUSTERS

        yield TestDefinition(run_name=name, name=name, target=target)


__all__ = ['TestTarget', 'TestDefinition', 'AllTests', 'ApplicationPaths']
