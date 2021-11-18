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

import chiptest.linux

from .test_definition import TestTarget, TestDefinition


def AllTests(root: str):
    """Gets all the tests that can be found in the ROOT directory based on
       yaml file names.
    """
    for path in Path(os.path.join(root, 'src', 'app',  'tests', 'suites')).rglob("*.yaml"):
        logging.debug('Found YAML: %s' % path)

        # grab the name without the extension
        name = path.stem.lower()

        if name.startswith('tv_'):
            target = TestTarget.ALL_CLUSTERS
            name = 'tv-' + name[3:]
        elif name.startswith('test_'):
            target = TestTarget.TV
            name = 'app-' + name[5:]
        else:
            continue

        yield TestDefinition(yaml_file=path, name=name, target=target)


__all__ = ['TestTarget', 'TestDefinition', 'AllTests', 'linux']
