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

import argparse
import os
from pathlib import Path
import sys
import subprocess
import logging

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


class ZAPGenerateTarget:
    def __init__(self, zap_config, template=None, output_dir=None):
        self.script = './scripts/tools/zap/generate.py'
        self.zap_config = str(zap_config)
        self.template = template

        if output_dir:
            # make sure we convert  any os.PathLike object to string
            self.output_dir = str(output_dir)
        else:
            self.output_dir = None

    def log_command(self):
        """Log the command that will get run for this target
        """
        logging.info("  %s" % " ".join(self.build_cmd()))

    def build_cmd(self):
        """Builds the command line we would run to generate this target.
        """
        cmd = [self.script, self.zap_config]

        if self.template:
            cmd.append('-t')
            cmd.append(self.template)

        if self.output_dir:
            if not os.path.exists(self.output_dir):
                os.makedirs(self.output_dir)
            cmd.append('-o')
            cmd.append(self.output_dir)

        return cmd

    def generate(self):
        """Runs a ZAP generate command on the configured zap/template/outputs.
        """
        cmd = self.build_cmd()
        logging.info("Generating target: %s" % " ".join(cmd))
        subprocess.check_call(cmd)
        if "chef" in self.zap_config:
            af_gen_event = os.path.join(self.output_dir, "af-gen-event.h")
            with open(af_gen_event, "w+"):  # Empty file needed for linux
                pass
            idl_path = self.zap_config.replace(".zap", ".matter")
            target_path = os.path.join(self.output_dir, os.path.basename(idl_path))
            os.rename(idl_path, target_path)


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def setupArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Generate content from ZAP files')
    parser.add_argument('--type', default='all', choices=['all', 'tests'],
                        help='Choose which content type to generate (default: all)')
    parser.add_argument('--tests', default='all', choices=['all', 'chip-tool', 'chip-tool-darwin', 'darwin', 'app1', 'app2'],
                        help='When generating tests only target, Choose which tests to generate (default: all)')
    parser.add_argument('--dry-run', default=False, action='store_true',
                        help="Don't do any generationl just log what targets would be generated (default: False)")
    return parser.parse_args()


def getGlobalTemplatesTargets():
    targets = []

    for filepath in Path('./examples').rglob('*.zap'):
        example_name = filepath.as_posix()
        example_name = example_name[example_name.index('examples/') + 9:]
        example_name = example_name[:example_name.index('/')]

        # Place holder has apps within each build
        if example_name == "placeholder":
            example_name = filepath.as_posix()
            example_name = example_name[example_name.index(
                'apps/') + len('apps/'):]
            example_name = example_name[:example_name.index('/')]
            logging.info("Found example %s (via %s)" %
                         (example_name, str(filepath)))

            # The name zap-generated is to make includes clear by using
            # a name like <zap-generated/foo.h>
            output_dir = os.path.join(
                'zzz_generated', 'placeholder', example_name, 'zap-generated')
            template = 'examples/placeholder/templates/templates.json'

            targets.append(ZAPGenerateTarget(filepath, output_dir=output_dir))
            targets.append(
                ZAPGenerateTarget(filepath, output_dir=output_dir, template=template))
            continue

        if example_name == "chef":
            if os.path.join("chef", "devices") not in str(filepath):
                continue
            example_name = "chef-"+os.path.basename(filepath)[:-len(".zap")]

        logging.info("Found example %s (via %s)" %
                     (example_name, str(filepath)))

        # The name zap-generated is to make includes clear by using
        # a name like <zap-generated/foo.h>
        output_dir = os.path.join(
            'zzz_generated', example_name, 'zap-generated')
        targets.append(ZAPGenerateTarget(filepath, output_dir=output_dir))

    targets.append(ZAPGenerateTarget(
        'src/controller/data_model/controller-clusters.zap',
        output_dir=os.path.join('zzz_generated/controller-clusters/zap-generated')))

    return targets


def getTestsTemplatesTargets(test_target):
    templates = {
        'chip-tool': {
            'zap': 'src/controller/data_model/controller-clusters.zap',
            'template': 'examples/chip-tool/templates/tests/templates.json',
            'output_dir': 'zzz_generated/chip-tool/zap-generated'
        },
        'chip-tool-darwin': {
            'zap': 'src/controller/data_model/controller-clusters.zap',
            'template': 'examples/chip-tool-darwin/templates/tests/templates.json',
            'output_dir': 'zzz_generated/chip-tool-darwin/zap-generated'
        },
        'darwin': {
            'zap': 'src/controller/data_model/controller-clusters.zap',
            'template': 'src/darwin/Framework/CHIP/templates/tests/templates.json',
            'output_dir': None
        }
    }

    # Place holder has apps within each build
    for filepath in Path('./examples/placeholder').rglob('*.zap'):
        example_name = filepath.as_posix()
        example_name = example_name[example_name.index(
            'apps/') + len('apps/'):]
        example_name = example_name[:example_name.index('/')]

        templates[example_name] = {
            'zap': filepath,
            'template': 'examples/placeholder/templates/templates.json',
            'output_dir': os.path.join('zzz_generated', 'placeholder', example_name, 'zap-generated')
        }

    targets = []
    for key, target in templates.items():
        if test_target == 'all' or test_target == key:
            logging.info("Found test target %s (via %s)" %
                         (key, target['template']))
            targets.append(ZAPGenerateTarget(
                target['zap'], template=target['template'], output_dir=target['output_dir']))

    return targets


def getSpecificTemplatesTargets():
    zap_filepath = 'src/controller/data_model/controller-clusters.zap'

    # Mapping of required template and output directory
    templates = {
        'src/app/common/templates/templates.json': 'zzz_generated/app-common/app-common/zap-generated',
        'examples/chip-tool/templates/templates.json': 'zzz_generated/chip-tool/zap-generated',
        'examples/chip-tool-darwin/templates/templates.json': 'zzz_generated/chip-tool-darwin/zap-generated',
        'src/controller/python/templates/templates.json': None,
        'src/darwin/Framework/CHIP/templates/templates.json': None,
        'src/controller/java/templates/templates.json': None,
        'src/app/tests/suites/templates/templates.json': 'zzz_generated/controller-clusters/zap-generated',
    }

    targets = []
    for template, output_dir in templates.items():
        logging.info("Found specific template %s" % template)
        targets.append(ZAPGenerateTarget(
            zap_filepath, template=template, output_dir=output_dir))

    return targets


def getTargets(type, test_target):
    targets = []

    if type == 'all':
        targets.extend(getGlobalTemplatesTargets())
        targets.extend(getTestsTemplatesTargets('all'))
        targets.extend(getSpecificTemplatesTargets())
    elif type == 'tests':
        targets.extend(getTestsTemplatesTargets(test_target))

    logging.info("Targets to be generated:")
    for target in targets:
        target.log_command()

    return targets


def main():
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s'
    )
    checkPythonVersion()
    os.chdir(CHIP_ROOT_DIR)
    args = setupArgumentsParser()

    targets = getTargets(args.type, args.tests)

    if (not args.dry_run):
        for target in targets:
            target.generate()


if __name__ == '__main__':
    main()
