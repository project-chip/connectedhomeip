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

import logging
import sys

import click

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False

try:
    from matter_idl.matter_idl_parser import CreateParser
except ImportError:
    import os
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), 'py_matter_idl')))
    from matter_idl.matter_idl_parser import CreateParser

# isort: off
from matter_idl.generators import FileSystemGeneratorStorage, GeneratorStorage
from matter_idl.generators.registry import CodeGenerator, GENERATORS


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
    idl_tree = CreateParser().parse(open(idl_path, "rt").read())

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

    if expected_outputs:
        with open(expected_outputs, 'rt') as fin:
            expected = set()
            for line in fin.readlines():
                line = line.strip()
                if line:
                    expected.add(line)

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
