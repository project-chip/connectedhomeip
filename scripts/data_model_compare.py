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
    from matter.idl.matter_idl_types import Idl

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

def _normalize_order(idl: Idl):
    """Re-sorts contents of things inside a cluster so that
       output is easily diffed by humans.
    """

    # This method exists because `zapt` generation of IDL files
    # are generally based on SQL select query ordering, likely
    # with some sort fields to achieve determinism
    #
    # However overall, especially if manual editing, it seems
    # easier to just fix a sort order instead of trying to
    # match another tool ordering that resides in another
    # code location.

    idl.clusters.sort(key=lambda c: c.name)

    for cluster in idl.clusters:
        cluster.enums.sort(key=lambda e: e.name)
        cluster.bitmaps.sort(key=lambda b: b.name)
        cluster.events.sort(key=lambda e: e.code)
        cluster.attributes.sort(key=lambda a: a.definition.code)
        cluster.structs.sort(key=lambda s: s.name)
        cluster.commands.sort(key=lambda c: c.code)

@click.group()
@click.option(
    '--log-level',
    default='INFO',
    show_default=True,
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output.')
def main(log_level):
    """
    A program supporting parsing of CSA data model XML files and generating them
    as human readable IDL output.

    Also supports parsing and generating a diff against an existing .matter file,
    such as using:

    \b
       uv run scripts/data_model_compare.py parse  \\
          --output -                               \\
          data_model/master/clusters/Switch.xml

    \b
        uv run scripts/data_model_compare.py filter                    \\
        --matter src/controller/data_model/controller-clusters.matter  \\
        --output -                                                     \\
        data_model/master/clusters/Switch.xml
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

@main.command('filter')
@click.option(
    "--matter",
    default=None,
    required=True,
    type=click.Path(exists=True),
    help="An input .matter IDL to select a subset of clusters to include."
)
@click.option(
    "--output",
    default=None,
    required=True,
    type=click.Path(),
    help="Where to output the generated XML from the input data mode."
)
@click.argument('filenames', nargs=-1)
def filter_matter(matter, output, filenames):
    """
    Filter clusters from a ".matter" file to contain only clusters defined
    in the given data_model XML files
    """
    LOGGER.info("Starting to parse ...")
    sources = [ParseSource(source=name) for name in filenames]
    data_model_xmls = ParseXmls(sources)
    LOGGER.info("Parse completed")

    matter_idl = CreateParser(skip_meta=True).parse(
        open(matter).read(), file_name=matter)

    # ensure that input file is filtered to only interesting
    # clusters
    loaded_clusters = {c.code for c in data_model_xmls.clusters}
    matter_idl.clusters = [
        c for c in matter_idl.clusters if c.code in loaded_clusters]
    _normalize_order(matter_idl)

    storage = InMemoryStorage()
    IdlGenerator(storage=storage, idl=matter_idl).render(dry_run=False)
    if output == '-':
        print(storage.content)
    else:
        with open(output, 'wt', encoding="utf8") as o:
            o.write(storage.content)


@main.command('parse')
@click.option(
    "-o",
    "--output",
    default=None,
    type=click.Path(),
    help="Where to output the parsed IDL. Use `-` for output to stdout"
)
@click.argument('filenames', nargs=-1)
def parse(output, filenames):
    """
    Parse data_model XML files and output the resulting .matter file.
    """
    LOGGER.info("Starting to parse ...")
    sources = [ParseSource(source=name) for name in filenames]
    data = ParseXmls(sources)
    LOGGER.info("Parse completed")

    # make sure compares are working well - same ordering
    _normalize_order(data)

    storage = InMemoryStorage()
    IdlGenerator(storage=storage, idl=data).render(dry_run=False)

    if output:
        if output == '-':
            print(storage.content)
        else:
            with open(output, 'wt', encoding="utf8") as o:
                o.write(storage.content)


if __name__ == "__main__":
    main(auto_envvar_prefix='CHIP')
