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
import os.path
import shutil
import subprocess
import sys
import tempfile
import time
import traceback
import urllib.request
from dataclasses import dataclass
from enum import Flag, auto
from pathlib import Path
from typing import List

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../..'))


class TargetType(Flag):
    """Type of targets that can be re-generated"""

    # Global templates: generally examples and chip controller
    GLOBAL = auto()

    # codgen.py templates (generally java templates, which cannot be built
    # at compile time currently)
    IDL_CODEGEN = auto()

    # App-specific templates (see getSpecificTemplatesTargets)
    SPECIFIC = auto()

    # Golden compares for unit tests of zap codegen
    GOLDEN_TEST_IMAGES = auto()

    # All possible targets. Convenience constant
    ALL = GLOBAL | IDL_CODEGEN | SPECIFIC | GOLDEN_TEST_IMAGES


__TARGET_TYPES__ = {
    'global': TargetType.GLOBAL,
    'idl_codegen': TargetType.IDL_CODEGEN,
    'specific': TargetType.SPECIFIC,
    'golden_test_images': TargetType.GOLDEN_TEST_IMAGES,
    'all': TargetType.ALL,
}


class ZapInput:
    """ZAP may be run from a .zap configuration or from just cluster XML.

    Running from a '.zap' configuration will load the cluster XML and
    it will also load cluster enabling and settings defined in the .zap
    configuration.

    For `client-side` code generation, CHIP wants to explicitly not depend
    on zap enabling/disabling as everything should be enabled.
    """

    @staticmethod
    def FromZap(f):
        return ZapInput(zap_file=str(f))

    @staticmethod
    def FromPropertiesJson(f):
        """ Build without a ".zap" file, use the `-z/--zclProperties` command in zap. """
        return ZapInput(properties_json=str(f))

    def __init__(self, zap_file=None,  properties_json=None):
        if zap_file and properties_json:
            raise Exception("only one of zap/zcl should be specified")
        self.zap_file = zap_file
        self.properties_json = properties_json

    @property
    def value(self) -> str:
        if self.zap_file:
            return f"ZAP:{self.zap_file}"
        return f"ZCL:{self.properties_json}"

    @property
    def is_for_chef_example(self) -> bool:
        if self.zap_file is None:
            return False

        return "chef" in self.zap_file

    def build_command(self, script: str) -> List[str]:
        """What command to execute for this zap input. """
        if self.zap_file:
            return [script, self.zap_file]
        return [script, '-z', self.properties_json]


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

    @staticmethod
    def MatterIdlTarget(zap_config: ZapInput, client_side=False, matter_file_name=None):
        if client_side:
            return ZAPGenerateTarget(zap_config, matter_file_name=matter_file_name, template="src/app/zap-templates/matter-idl-client.json", output_dir=None)
        else:
            # NOTE: this assumes `src/app/zap-templates/matter-idl-server.json` is the
            #       DEFAULT generation target and it needs no output_dir
            return ZAPGenerateTarget(zap_config, matter_file_name=matter_file_name, template=None, output_dir=None)

    def __init__(self, zap_config: ZapInput, template, output_dir=None, matter_file_name=None):
        self.script = './scripts/tools/zap/generate.py'
        self.zap_config = zap_config
        self.template = template
        self.matter_file_name = matter_file_name

        if output_dir:
            # make sure we convert  any os.PathLike object to string
            self.output_dir = str(output_dir)
        else:
            self.output_dir = None

    @property
    def is_matter_idl_generation(self):
        return (self.output_dir is None)

    def distinct_output(self):
        if not self.template and not self.output_dir:
            # Matter IDL templates have no template/output dir as they go with the
            # default.
            #
            # output_directory is MIS-USED here because zap files may reside in the same
            # directory (e.g. chef) so we claim the zap config is an output directory
            # for uniqueness
            return ZapDistinctOutput(input_template=None, output_directory=self.zap_config.value)
        else:
            return ZapDistinctOutput(input_template=self.template, output_directory=self.output_dir)

    def log_command(self):
        """Log the command that will get run for this target
        """
        logging.info("  %s" % " ".join(self.build_cmd()))

    def build_cmd(self):
        """Builds the command line we would run to generate this target.
        """
        cmd = self.zap_config.build_command(self.script)

        if self.template:
            cmd.append('-t')
            cmd.append(self.template)

        if self.output_dir:
            if not os.path.exists(self.output_dir):
                os.makedirs(self.output_dir)
            cmd.append('-o')
            cmd.append(self.output_dir)

        if self.matter_file_name:
            cmd.append('-m')
            cmd.append(self.matter_file_name)

        return cmd

    def generate(self) -> TargetRunStats:
        """Runs a ZAP generate command on the configured zap/template/outputs.
        """
        cmd = self.build_cmd()
        logging.info("Generating target: %s" % " ".join(cmd))

        generate_start = time.time()
        subprocess.check_call(cmd)
        generate_end = time.time()

        if self.zap_config.is_for_chef_example:
            idl_path = self.zap_config.zap_file.replace(".zap", ".matter")
            target_path = os.path.join("examples",
                                       "chef",
                                       "devices",
                                       os.path.basename(idl_path))
            os.rename(idl_path, target_path)
        return TargetRunStats(
            generate_time=generate_end - generate_start,
            config=self.zap_config.value,
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


class JinjaCodegenTarget():
    def __init__(self, generator: str, output_directory: str, idl_path: str):
        # This runs a test, but the important bit is we pass `--regenerate`
        # to it and this will cause it to OVERWRITE golden images.
        self.idl_path = idl_path
        self.generator = generator
        self.output_directory = output_directory
        self.command = ["./scripts/codegen.py", "--output-dir", output_directory,
                        "--generator", generator, idl_path]

    def formatKotlinFiles(self, paths):
        try:
            logging.info("Prettifying %d kotlin files:", len(paths))
            for name in paths:
                logging.info("    %s" % name)

            VERSION = "0.44"
            JAR_NAME = f"ktfmt-{VERSION}-jar-with-dependencies.jar"
            jar_url = f"https://repo1.maven.org/maven2/com/facebook/ktfmt/{VERSION}/{JAR_NAME}"

            with tempfile.TemporaryDirectory(prefix='ktfmt') as tmpdir:
                path, http_message = urllib.request.urlretrieve(jar_url, Path(tmpdir).joinpath(JAR_NAME).as_posix())
                subprocess.check_call(['java', '-jar', path, '--google-style'] + paths)
        except Exception:
            traceback.print_exc()

    def codeFormat(self):
        outputs = subprocess.check_output(["./scripts/codegen.py", "--name-only", "--generator",
                                           self.generator, "--log-level", "fatal", self.idl_path]).decode("utf8").split("\n")
        outputs = [os.path.join(self.output_directory, name) for name in outputs if name]

        # Split output files by extension,
        name_dict = {}
        for name in outputs:
            _, extension = os.path.splitext(name)
            name_dict[extension] = name_dict.get(extension, []) + [name]

        if '.kt' in name_dict:
            self.formatKotlinFiles(name_dict['.kt'])

    def generate(self) -> TargetRunStats:
        generate_start = time.time()

        subprocess.check_call(self.command)

        self.codeFormat()

        generate_end = time.time()

        return TargetRunStats(
            generate_time=generate_end - generate_start,
            config=f'codegen:{self.generator}',
            template=self.idl_path,
        )

    def distinct_output(self):
        # Fake output - this is a single target that generates golden images
        return ZapDistinctOutput(input_template=f'{self.generator}{self.idl_path}', output_directory=self.output_directory)

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

        if example_name == "chef":
            if os.path.join("chef", "devices") not in str(filepath):
                continue

            if filepath.name == "template.zap":
                continue

            example_name = "chef-"+os.path.basename(filepath)[:-len(".zap")]

        logging.info("Found example %s (via %s)" %
                     (example_name, str(filepath)))

        targets.append(ZAPGenerateTarget.MatterIdlTarget(ZapInput.FromZap(filepath)))

    targets.append(ZAPGenerateTarget.MatterIdlTarget(ZapInput.FromPropertiesJson('src/app/zap-templates/zcl/zcl.json'),
                   client_side=True, matter_file_name="src/controller/data_model/controller-clusters.matter"))

    return targets


def getCodegenTemplates():
    targets = []

    targets.append(JinjaCodegenTarget(
        generator="java-class",
        idl_path="src/controller/data_model/controller-clusters.matter",
        output_directory="src/controller/java/generated"))

    targets.append(JinjaCodegenTarget(
        generator="kotlin-class",
        idl_path="src/controller/data_model/controller-clusters.matter",
        output_directory="src/controller/java/generated"))

    targets.append(JinjaCodegenTarget(
        generator="summary-markdown",
        idl_path="src/controller/data_model/controller-clusters.matter",
        output_directory="docs"))

    return targets


def getGoldenTestImageTargets():
    return [GoldenTestImageTarget()]


def getSpecificTemplatesTargets():
    zap_input = ZapInput.FromPropertiesJson('src/app/zap-templates/zcl/zcl.json')

    # Mapping of required template and output directory
    templates = {
        'src/app/common/templates/templates.json': 'zzz_generated/app-common/app-common/zap-generated',
        'examples/chip-tool/templates/templates.json': 'zzz_generated/chip-tool/zap-generated',
        'examples/darwin-framework-tool/templates/templates.json': 'zzz_generated/darwin-framework-tool/zap-generated',
        'src/controller/python/templates/templates.json': None,
        'src/darwin/Framework/CHIP/templates/templates.json': None,
        'src/controller/java/templates/templates.json': None,
        'examples/tv-casting-app/darwin/MatterTvCastingBridge/MatterTvCastingBridge/templates/templates.json': None,
    }

    targets = []
    for template, output_dir in templates.items():
        logging.info("Found specific template %s" % template)
        targets.append(ZAPGenerateTarget(zap_input, template=template, output_dir=output_dir))

    return targets


def getTargets(type):
    targets = []

    if type & TargetType.GLOBAL:
        targets.extend(getGlobalTemplatesTargets())

    if type & TargetType.SPECIFIC:
        targets.extend(getSpecificTemplatesTargets())

    if type & TargetType.IDL_CODEGEN:
        targets.extend(getCodegenTemplates())

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

    targets = getTargets(args.type)

    if args.dry_run:
        sys.exit(0)

    if args.run_bootstrap:
        subprocess.check_call(os.path.join(
            CHIP_ROOT_DIR, "scripts/tools/zap/zap_bootstrap.sh"), shell=True)

    timings = []
    if args.parallel:
        # Ensure each zap run is independent
        os.environ['ZAP_TEMPSTATE'] = '1'

        # There is a sequencing here:
        #   - ZAP will generate ".matter" files
        #   - various codegen may generate from ".matter" files (like java)
        # We split codegen into two generations to not be racy
        first, second = [], []
        for target in targets:
            if isinstance(target, ZAPGenerateTarget) and target.is_matter_idl_generation:
                first.append(target)
            else:
                second.append(target)

        for items in [first, second]:
            with multiprocessing.Pool() as pool:
                for timing in pool.imap_unordered(_ParallelGenerateOne, items):
                    timings.append(timing)
    else:
        for target in targets:
            timings.append(target.generate())

    timings.sort(key=lambda t: t.generate_time)

    print(" Time (s) | {:^50} | {:^50}".format("Config", "Template"))
    for timing in timings:
        tmpl = timing.template

        if tmpl is None:
            tmpl = '[NONE (matter idl generation)]'

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
