#!/usr/bin/env python3
# Copyright (c) 2022 Project CHIP Authors
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

# py_matter_idl may not be installed in the pigweed venv.
# Reference it directly from the source tree.
from python_path import PythonPath

with PythonPath('py_matter_idl', relative_to=__file__):
    from matter.idl.generators.path_resolution import expand_path_for_idl
    from matter.idl.generators.registry import GENERATORS, CodeGenerator
    from matter.idl.generators.storage import FileSystemGeneratorStorage, GeneratorStorage
    from matter.idl.matter_idl_parser import CreateParser

import logging
import os.path
import subprocess
import sys
import tempfile
import traceback
import urllib.request

import click
from pathlib import Path
from tools.zap.clang_format import getClangFormatBinary

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False


class ListGeneratedFilesStorage(GeneratorStorage):
    """
    A storage that prints out file names that would have content in them.
    """

    def __init__(self):
        super().__init__()

    def get_existing_data(self, relative_path: str):
        return None  # stdout has no pre-existing data

    def write_new_data(self, relative_path: str, content: str):
        print(relative_path)


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output')
@click.option(
    '--generator', '-g',
    default='java-jni',
    help='What code generator to run.  The choices are: '+'|'.join(GENERATORS.keys())+'. ' +
         'When using custom, provide the plugin path using `--generator custom:<path_to_plugin>:<plugin_module_name>` syntax. ' +
         'For example, `--generator custom:./my_plugin:my_plugin_module` will load `./my_plugin/my_plugin_module/__init.py__` ' +
         'that defines a subclass of CodeGenerator named CustomGenerator.')
@click.option(
    '--option',
    multiple=True,
    help="Extra generator options, of the form: --option key:value")
@click.option(
    '--output-dir',
    type=click.Path(exists=False),
    default=".",
    help='Where to generate the code')
@click.option(
    '--dry-run',
    default=False,
    is_flag=True,
    help='If to actually generate')
@click.option(
    '--name-only',
    default=False,
    is_flag=True,
    help='Output just a list of file names that would be generated')
@click.option(
    '--expected-outputs',
    type=click.Path(exists=True),
    default=None,
    help='A file containing all expected outputs. Script will fail if outputs do not match')
@click.argument(
    'idl_path',
    type=click.Path(exists=True))
def main(log_level, generator, option, output_dir, dry_run, name_only, expected_outputs, idl_path):

    def formatKotlinFiles(paths):
        try:
            logging.info("Prettifying %d kotlin files:", len(paths))
            for name in paths:
                logging.info("    %s" % name)

            VERSION = "0.58"
            JAR_NAME = f"ktfmt-{VERSION}-with-dependencies.jar"
            jar_url = f"https://repo1.maven.org/maven2/com/facebook/ktfmt/{VERSION}/{JAR_NAME}"

            # ensure we have some headers otherwise maven seems to 403 us
            opener = urllib.request.build_opener()
            opener.addheaders = [('User-agent', 'Mozilla/5.0')]
            urllib.request.install_opener(opener)

            with tempfile.TemporaryDirectory(prefix='ktfmt') as tmpdir:
                path, _ = urllib.request.urlretrieve(jar_url, Path(tmpdir).joinpath(JAR_NAME).as_posix())
                subprocess.check_call(['java', '-jar', path, '--google-style'] + paths)
        except Exception:
            traceback.print_exc()
            raise
    """
    Parses MATTER IDL files (.matter) and performs SDK code generation
    as set up by the program arguments.
    """
    if _has_coloredlogs:
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    if name_only:
        storage = ListGeneratedFilesStorage()
    else:
        storage = FileSystemGeneratorStorage(output_dir)

    logging.info("Parsing idl from %s" % idl_path)
    idl_tree = CreateParser().parse(open(idl_path, "rt").read(), file_name=idl_path)

    plugin_module = None
    if generator.startswith('custom:'):
        # check that the plugin path is provided
        custom_params = generator.split(':')
        if len(custom_params) != 3:
            logging.fatal("Custom generator format not valid. Please use --generator custom:<path>:<module>")
            sys.exit(1)
        (generator, plugin_path, plugin_module) = custom_params

        logging.info("Using CustomGenerator at plugin path %s.%s" % (plugin_path, plugin_module))
        sys.path.append(plugin_path)
        generator = 'CUSTOM'

    extra_args = {}
    for o in option:
        if ':' not in o:
            logging.fatal("Please specify options as '<key>:<value>'. %r is not valid. " % o)
            sys.exit(1)
        key, value = o.split(':')
        extra_args[key] = value

    logging.info("Running code generator %s" % generator)
    generator = CodeGenerator.FromString(generator).Create(storage, idl=idl_tree, plugin_module=plugin_module, **extra_args)
    generator.render(dry_run)

    outputs = [os.path.join(output_dir, name) for name in storage.generated_paths if name]

    # Split output files by extension,
    name_dict = {}
    for name in outputs:
        _, extension = os.path.splitext(name)
        name_dict[extension] = name_dict.get(extension, []) + [name]

    if name_dict.get('.kt', []):
        try:
            logging.debug("Formatting kt_files: %s" % name_dict['.kt'])
        except Exception:
            traceback.print_exc()
        formatKotlinFiles(name_dict['.kt'])

    cpp_files = []
    for ext in [".h", ".cpp", ".c", ".hpp"]:
        cpp_files.extend(name_dict.get(ext, []))
    if cpp_files:
        try:
            logging.debug("Formatting cpp_files: %s", cpp_files)
            subprocess.check_call([getClangFormatBinary(), "-i"] + cpp_files)
        except Exception:
            traceback.print_exc()

    if expected_outputs:
        with open(expected_outputs, 'rt') as fin:
            expected = set()
            for line in fin.readlines():
                line = line.strip()
                for expanded_path in expand_path_for_idl(idl_tree, line):
                    expected.add(expanded_path)

            if expected != storage.generated_paths:
                logging.fatal("expected and generated files do not match.")

                extra = storage.generated_paths - expected
                missing = expected - storage.generated_paths

                for name in extra:
                    logging.fatal("   '%s' was generated but not expected" % name)

                for name in missing:
                    logging.fatal("   '%s' was expected but not generated" % name)

                sys.exit(1)

    logging.info("Done")


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
