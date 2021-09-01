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
import logging

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def getGlobalTemplatesTargets():
    targets = []

    for filepath in Path('./examples').rglob('*.zap'):
        example_name = filepath.as_posix()
        example_name = example_name[example_name.index('examples/') + 9:]
        example_name = example_name[:example_name.index('/')]

        logging.info("Found example %s (via %s)" %
                     (example_name, str(filepath)))

        # The name zap-generated is to make includes clear by using
        # a name like <zap-generated/foo.h>
        output_dir = os.path.join(
            'zzz_generated', example_name, 'zap-generated')
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        targets.append([str(filepath), '-o', output_dir])

    targets.extend([
        [
            './src/controller/data_model/controller-clusters.zap',
            '-o',
            os.path.join('zzz_generated/controller-clusters/zap-generated')]])

    return targets


def getSpecificTemplatesTargets():
    targets = []

    # Mapping of required template and output directory
    templates = {
        'src/app/common/templates/templates.json': 'zzz_generated/app-common/app-common/zap-generated',
        'examples/chip-tool/templates/templates.json': 'zzz_generated/chip-tool/zap-generated',
        'src/controller/python/templates/templates.json': None,
        'src/darwin/Framework/CHIP/templates/templates.json': None,
        'src/controller/java/templates/templates.json': None,
    }

    for template, output_dir in templates.items():
        target = [
            'src/controller/data_model/controller-clusters.zap', '-t', template]
        if output_dir is not None:
            if not os.path.exists(output_dir):
                os.makedirs(output_dir)
            target.extend(['-o', output_dir])

        targets.append(target)

    return targets


def getTargets():
    targets = []
    targets.extend(getGlobalTemplatesTargets())
    targets.extend(getSpecificTemplatesTargets())
    return targets


def main():
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s'
    )
    checkPythonVersion()
    os.chdir(CHIP_ROOT_DIR)

    targets = getTargets()
    for target in targets:
        exec_list = ['./scripts/tools/zap/generate.py'] + target
        logging.info("Generating target: %s" % " ".join(exec_list))
        subprocess.check_call(exec_list)


if __name__ == '__main__':
    main()
