#!/usr/bin/env python3
#
#    Copyright (c) 2020 Project CHIP Authors
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

import os
from pathlib import Path
import sys
import subprocess

CHIP_ROOT_DIR = os.path.realpath(os.path.join(os.path.dirname(__file__), '../..'))

def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' + str(sys.version_info[0]))
        exit(1)

def getGlobalTemplatesTargets():
    targets = []
    targets.extend([[str(filepath)] for filepath in Path('./examples').rglob('*.zap')])
    targets.extend([[str(filepath)] for filepath in Path('./src/darwin').rglob('*.zap')])
    targets.extend([[str(filepath)] for filepath in Path('./src/controller/data_model').rglob('*.zap')])
    return targets

def getSpecificTemplatesTargets():
    targets = []
    targets.append(['src/controller/data_model/controller-clusters.zap', '-t', 'src/app/common/templates/templates.json'])
    targets.append(['src/controller/data_model/controller-clusters.zap', '-t', 'examples/chip-tool/templates/templates.json'])
    targets.append(['src/controller/data_model/controller-clusters.zap', '-t', 'src/controller/python/templates/templates.json'])
    targets.append(['src/controller/data_model/controller-clusters.zap', '-t', 'src/darwin/Framework/CHIP/templates/templates.json'])
    targets.append(['src/controller/data_model/controller-clusters.zap', '-t', 'src/controller/java/templates/templates.json'])
    return targets

def getTargets():
    targets = []
    targets.extend(getGlobalTemplatesTargets())
    targets.extend(getSpecificTemplatesTargets())
    return targets

def main():
    checkPythonVersion()
    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets()
    for target in targets:
        subprocess.check_call(['./scripts/tools/zap/generate.py'] + target)

if __name__ == '__main__':
    main()
