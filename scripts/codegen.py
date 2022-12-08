#!/usr/bin/env python
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

import click
import logging
import enum
import sys

try:
    import coloredlogs
    _has_coloredlogs = True
except:
    _has_coloredlogs = False

try:
    from idl.matter_idl_parser import CreateParser
except:
    import os
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))
    from idl.matter_idl_parser import CreateParser

from idl.generators import FileSystemGeneratorStorage, GeneratorStorage
from idl.generators.registry import CodeGenerator, GENERATORS


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
    '--generator',
    default='JAVA',
    type=click.Choice(GENERATORS.keys(), case_sensitive=False),
    help='What code generator to run')
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
def main(log_level, generator, output_dir, dry_run, name_only, expected_outputs, idl_path):
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

    logging.info("Parsing idl from %s" % idl_path)
    idl_tree = CreateParser().parse(open(idl_path, "rt").read())

    if name_only:
        storage = ListGeneratedFilesStorage()
    else:
        storage = FileSystemGeneratorStorage(output_dir)

    logging.info("Running code generator %s" % generator)
    generator = CodeGenerator.FromString(generator).Create(storage, idl=idl_tree)
    generator.render(dry_run)

    if expected_outputs:
        with open(expected_outputs, 'rt') as fin:
            expected = set()
            for l in fin.readlines():
                l = l.strip()
                if l:
                    expected.add(l)

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
