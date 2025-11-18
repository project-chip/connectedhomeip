#!/usr/bin/env python3
#
# Copyright (c) 2022-2025 Project CHIP Authors
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
#
# Can be executed using uv like this:
#
#   uv run scripts/data_model_compare.py --help
#
# /// script
# requires-python = ">=3.13"
# dependencies = [
#     "click",
#     "coloredlogs",
#     "jinja2",
#     "lark",
#     "python-path",
# ]
# ///

import logging
import sys
import typing
import xml.sax.handler
from dataclasses import dataclass
from typing import List, Optional, Union

import click

# py_matter_idl may not be installed in the pigweed venv.
# Reference it directly from the source tree.
from python_path import PythonPath

with PythonPath('py_matter_idl', relative_to=__file__):
    from matter.idl.generators.idl import IdlGenerator
    from matter.idl.generators.storage import InMemoryStorage
    from matter.idl.matter_idl_parser import CreateParser
    from matter.idl.data_model_xml import ParseXmls, ParseSource

try:
    import coloredlogs
    _has_coloredlogs = True
except ImportError:
    _has_coloredlogs = False

LOGGER = logging.getLogger(__name__)

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARNING,
    'fatal': logging.FATAL,
}

@click.command()
@click.option(
    '--log-level',
    default='INFO',
    show_default=True,
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option(
    '--no-print',
    show_default=True,
    default=False,
    is_flag=True,
    help='Do not pring output data (parsed data)')
@click.option(
    "-o",
    "--output",
    default=None,
    type=click.Path(),
    help="Where to output the parsed IDL."
)
@click.option(
    "--compare",
    default=None,
    type=click.Path(exists=True),
    help="An input .matter IDL to compare with."
)
@click.option(
    "--compare-output",
    default=None,
    type=click.Path(),
    help="Where to output the compare IDL"
)
@click.argument('filenames', nargs=-1)
def main(log_level, no_print, output, compare, compare_output, filenames):
    """
    A program supporting parsing of CSA data model XML files and generating them
    as human readable IDL output.

    Also supports parsing and generating a diff against an existing .matter file,
    such as using:

    \b
       matter-data-model-xml-parser                                      \\
          --compare src/controller/data_model/controller-clusters.matter \\
          --compare-output out/orig.matter                               \\
          --output out/from_xml.matter                                   \\
          data_model/clusters/Switch.xml
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

    if (compare is None) != (compare_output is None):
        LOGGER.error(
            "Either both or none of --compare AND --compare-output must be set")
        sys.exit(1)

    LOGGER.info("Starting to parse ...")

    sources = [ParseSource(source=name) for name in filenames]
    data = ParseXmls(sources)
    LOGGER.info("Parse completed")

    if compare:
        other_idl = CreateParser(skip_meta=True).parse(
            open(compare).read(), file_name=compare)

        # ensure that input file is filtered to only interesting
        # clusters
        loaded_clusters = {c.code for c in data.clusters}
        other_idl.clusters = [
            c for c in other_idl.clusters if c.code in loaded_clusters]

        # Ensure consistent ordering for compares
        normalize_order(data)
        normalize_order(other_idl)

        storage = InMemoryStorage()
        IdlGenerator(storage=storage, idl=other_idl).render(dry_run=False)
        with open(compare_output, 'wt', encoding="utf8") as o:
            o.write(storage.content)

    storage = InMemoryStorage()
    IdlGenerator(storage=storage, idl=data).render(dry_run=False)

    if output:
        with open(output, 'wt', encoding="utf8") as o:
            o.write(storage.content)
    elif not no_print:
        print(storage.content)


if __name__ == "__main__":
    main(auto_envvar_prefix='CHIP')
