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
import logging
import multiprocessing
import os
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from enum import Flag, auto
from pathlib import Path

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


# Type of targets that can be re-generated
class TargetType(Flag):

    # Tests for golden images
    TESTS = auto()

    # Global templates: generally examples and chip controller
    GLOBAL = auto()

    # App-specific templates (see getSpecificTemplatesTargets)
    SPECIFIC = auto()

    # Golden compares for unit tests of zap codegen
    GOLDEN_TEST_IMAGES = auto()

    # All possible targets. Convenience constant
    ALL = TESTS | GLOBAL | SPECIFIC | GOLDEN_TEST_IMAGES


__TARGET_TYPES__ = {
    'tests': TargetType.TESTS,
    'global': TargetType.GLOBAL,
    'specific': TargetType.SPECIFIC,
    'golden_test_images': TargetType.GOLDEN_TEST_IMAGES,
    'all': TargetType.ALL,
}


@dataclass
class TargetRunStats:
    config: str
    template: str
    generate_time: float


@dataclass(eq=True, frozen=True)
class ZapDistinctOutput:
    """Defines the properties that determine if some output seems unique or
       not, for the purposes of detecting codegen overlap.

       Not perfect, since separate templates may use the same file names, but
       better than nothing.
    """

    input_template: str
    output_directory: str


class ZAPGenerateTarget:
    def __init__(self, zap_config, template, output_dir=None):
        self.script = './scripts/tools/zap/generate.py'
        self.zap_config = str(zap_config)
        self.template = template

        if output_dir:
            # make sure we convert  any os.PathLike object to string
            self.output_dir = str(output_dir)
        else:
            self.output_dir = None

    def distinct_output(self):
        return ZapDistinctOutput(input_template=self.template, output_directory=self.output_dir)

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

    def generate(self) -> TargetRunStats:
        """Runs a ZAP generate command on the configured zap/template/outputs.
        """
        cmd = self.build_cmd()
        logging.info("Generating target: %s" % " ".join(cmd))

        generate_start = time.time()
        subprocess.check_call(cmd)
        generate_end = time.time()

        if "chef" in self.zap_config:
            af_gen_event = os.path.join(self.output_dir, "af-gen-event.h")
            with open(af_gen_event, "w+"):  # Empty file needed for linux
                pass
            idl_path = self.zap_config.replace(".zap", ".matter")
            target_path = os.path.join("examples",
                                       "chef",
                                       "devices",
                                       os.path.basename(idl_path))
            os.rename(idl_path, target_path)
        return TargetRunStats(
            generate_time=generate_end - generate_start,
            config=self.zap_config,
            template=self.template,
        )


class GoldenTestImageTarget():
    def __init__(self):
        # NOTE: output-path is inside the tree. This is because clang-format
        #       will search for a .clang-format file in the directory tree
        #       so attempts to format outside the tree will generate diffs.
        # NOTE: relative path because this script generally does a
        #       os.chdir to CHIP_ROOT anyway.
        os.makedirs('./out', exist_ok=True)
        self.tempdir = tempfile.mkdtemp(prefix='test_golden', dir='./out')

        # This runs a test, but the important bit is we pass `--regenerate`
        # to it and this will cause it to OVERWRITE golden images.
        self.command = ["./scripts/tools/zap/test_generate.py",
                        "--output", self.tempdir, "--regenerate"]

    def __del__(self):
        # Clean up
        if os.path.isdir(self.tempdir):
            shutil.rmtree(self.tempdir)

    def generate(self) -> TargetRunStats:
        generate_start = time.time()
        subprocess.check_call(self.command)
        generate_end = time.time()

        return TargetRunStats(
            generate_time=generate_end - generate_start,
            config='./scripts/tools/zap/test_generate.py',
            template='./scripts/tools/zap/test_generate.py',
        )

    def distinct_output(self):
        # Fake output - this is a single target that generates golden images
        return ZapDistinctOutput(input_template='GOLDEN_IMAGES', output_directory='GOLDEN_IMAGES')

    def log_command(self):
        logging.info("  %s" % " ".join(self.command))


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def setupArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Generate content from ZAP files')
    parser.add_argument('--type', action='append', choices=__TARGET_TYPES__.keys(),
                        help='Choose which content type to generate (default: all)')
    parser.add_argument('--tests', default='all', choices=['all', 'chip-tool', 'darwin-framework-tool', 'app1', 'app2'],
                        help='When generating tests only target, Choose which tests to generate (default: all)')
    parser.add_argument('--dry-run', default=False, action='store_true',
                        help="Don't do any generation, just log what targets would be generated (default: False)")
    parser.add_argument('--run-bootstrap', default=None, action='store_true',
                        help='Automatically run ZAP bootstrap. By default the bootstrap is not triggered')

    parser.add_argument('--parallel', action='store_true')
    parser.add_argument('--no-parallel', action='store_false', dest='parallel')
    parser.set_defaults(parallel=True)

    args = parser.parse_args()

    # Convert a list of target_types (as strings)
    # into a single flag value
    if not args.type:
        args.type = TargetType.ALL  # default instead of a list
    else:
        # convert the list into a single flag value
        types = [t for t in map(lambda x: __TARGET_TYPES__[
                                x.lower()], args.type)]
        args.type = types[0]
        for t in types:
            args.type = args.type | t

    return args


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

            targets.append(ZAPGenerateTarget(
                filepath, output_dir=output_dir, template="src/app/zap-templates/matter-idl.json"))
            targets.append(
                ZAPGenerateTarget(filepath, output_dir=output_dir, template=template))
            continue

        if example_name == "chef":
            if os.path.join("chef", "devices") not in str(filepath):
                continue
            example_name = "chef-"+os.path.basename(filepath)[:-len(".zap")]

        logging.info("Found example %s (via %s)" %
                     (example_name, str(filepath)))

        generate_subdir = example_name

        # Special casing lighting app because separate folders
        if example_name == "lighting-app":
            if 'nxp' in str(filepath):
                generate_subdir = f"{example_name}/nxp"

        # The name zap-generated is to make includes clear by using
        # a name like <zap-generated/foo.h>
        output_dir = os.path.join(
            'zzz_generated', generate_subdir, 'zap-generated')
        targets.append(ZAPGenerateTarget(filepath, output_dir=output_dir,
                       template="src/app/zap-templates/matter-idl.json"))

    targets.append(ZAPGenerateTarget(
        'src/controller/data_model/controller-clusters.zap',
        template="src/app/zap-templates/matter-idl.json",
        output_dir=os.path.join('zzz_generated/controller-clusters/zap-generated')))

    # This generates app headers for darwin only, for easier/clearer include
    # in .pbxproj files.
    #
    # TODO: These files can be code generated at compile time, we should figure
    #       out a path for this codegen to not be required.
    targets.append(ZAPGenerateTarget(
        'src/controller/data_model/controller-clusters.zap',
        template="src/app/zap-templates/app-templates.json",
        output_dir=os.path.join('zzz_generated/darwin/controller-clusters/zap-generated')))

    return targets


def getTestsTemplatesTargets(test_target):
    templates = {
        'chip-tool': {
            'zap': 'src/controller/data_model/controller-clusters.zap',
            'template': 'examples/chip-tool/templates/tests/templates.json',
            'output_dir': 'zzz_generated/chip-tool/zap-generated'
        },
        'darwin-framework-tool': {
            'zap': 'src/controller/data_model/controller-clusters.zap',
            'template': 'examples/darwin-framework-tool/templates/tests/templates.json',
            'output_dir': 'zzz_generated/darwin-framework-tool/zap-generated'
        }
    }

    targets = []
    for key, target in templates.items():
        if test_target == 'all' or test_target == key:
            logging.info("Found test target %s (via %s)" %
                         (key, target['template']))
            targets.append(ZAPGenerateTarget(
                target['zap'], template=target['template'], output_dir=target['output_dir']))

    return targets


def getGoldenTestImageTargets():
    return [GoldenTestImageTarget()]


def getSpecificTemplatesTargets():
    zap_filepath = 'src/controller/data_model/controller-clusters.zap'

    # Mapping of required template and output directory
    templates = {
        'src/app/common/templates/templates.json': 'zzz_generated/app-common/app-common/zap-generated',
        'src/app/tests/suites/templates/templates.json': 'zzz_generated/app-common/app-common/zap-generated',
        'examples/chip-tool/templates/templates.json': 'zzz_generated/chip-tool/zap-generated',
        'examples/darwin-framework-tool/templates/templates.json': 'zzz_generated/darwin-framework-tool/zap-generated',
        'src/controller/python/templates/templates.json': None,
        'src/darwin/Framework/CHIP/templates/templates.json': None,
        'src/controller/java/templates/templates.json': None,
    }

    targets = []
    for template, output_dir in templates.items():
        logging.info("Found specific template %s" % template)
        targets.append(ZAPGenerateTarget(
            zap_filepath, template=template, output_dir=output_dir))

    return targets


def getTargets(type, test_target):
    targets = []

    if type & TargetType.TESTS:
        targets.extend(getTestsTemplatesTargets('all'))

    if type & TargetType.GLOBAL:
        targets.extend(getGlobalTemplatesTargets())

    if type & TargetType.SPECIFIC:
        targets.extend(getSpecificTemplatesTargets())

    if type & TargetType.GOLDEN_TEST_IMAGES:
        targets.extend(getGoldenTestImageTargets())

    logging.info("Targets to be generated:")
    for target in targets:
        target.log_command()

    # validate that every target as a DISTINCT directory (we had bugs here
    # for various examples duplicating zap files)
    distinct_outputs = set()
    for target in targets:
        o = target.distinct_output()

        if o in distinct_outputs:
            logging.error("Same output %r:" % o)
            for t in targets:
                if t.distinct_output() == o:
                    logging.error("   %s" % t.zap_config)

            raise Exception("Duplicate/overlapping output directory: %r" % o)

        distinct_outputs.add(o)

    return targets


def _ParallelGenerateOne(target):
    """
    Helper method to be passed to multiprocessing parallel generation of
    items.
    """
    return target.generate()


def main():
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(name)s %(levelname)-7s %(message)s'
    )
    checkPythonVersion()
    os.chdir(CHIP_ROOT_DIR)
    args = setupArgumentsParser()

    targets = getTargets(args.type, args.tests)

    if args.dry_run:
        sys.exit(0)

    if args.run_bootstrap:
        subprocess.check_call(os.path.join(
            CHIP_ROOT_DIR, "scripts/tools/zap/zap_bootstrap.sh"), shell=True)

    timings = []
    if args.parallel:
        # Ensure each zap run is independent
        os.environ['ZAP_TEMPSTATE'] = '1'
        with multiprocessing.Pool() as pool:
            for timing in pool.imap_unordered(_ParallelGenerateOne, targets):
                timings.append(timing)
    else:
        for target in targets:
            timings.append(target.generate())

    timings.sort(key=lambda t: t.generate_time)

    print(" Time (s) | {:^50} | {:^50}".format("Config", "Template"))
    for timing in timings:
        tmpl = timing.template

        if len(tmpl) > 50:
            # easier to distinguish paths ... shorten common in-fixes
            tmpl = tmpl.replace("/zap-templates/", "/../")
            tmpl = tmpl.replace("/templates/", "/../")

        print(" %8d | %50s | %50s" % (
            timing.generate_time,
            ".." + timing.config[len(timing.config) -
                                 48:] if len(timing.config) > 50 else timing.config,
            ".." + tmpl[len(tmpl) - 48:] if len(tmpl) > 50 else tmpl,
        ))


if __name__ == '__main__':
    main()
