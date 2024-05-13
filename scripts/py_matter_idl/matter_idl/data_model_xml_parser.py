#!/usr/bin/env python3
# Copyright (c) 2023 Project CHIP Authors
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
import os
import sys
from typing import Optional

import click

try:
    from matter_idl.data_model_xml import ParseSource, ParseXmls
except ImportError:
    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.data_model_xml import ParseSource, ParseXmls

from matter_idl.generators import GeneratorStorage
from matter_idl.generators.idl import IdlGenerator
from matter_idl.matter_idl_parser import CreateParser
from matter_idl.matter_idl_types import Idl


class InMemoryStorage(GeneratorStorage):
    def __init__(self):
        super().__init__()
        self.content: Optional[str] = None

    def get_existing_data(self, relative_path: str):
        # Force re-generation each time
        return None

    def write_new_data(self, relative_path: str, content: str):
        if self.content:
            raise Exception(
                "Unexpected extra data: single file generation expected")
        self.content = content


def normalize_order(idl: Idl):
    """Re-sorts contents of things inside a cluster so that
       output is easily diffed by humans
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
       ./scripts/py_matter_idl/matter_idl/data_model_xml_parser.py       \\
          --compare src/controller/data_model/controller-clusters.matter \\
          --compare-output out/orig.matter                               \\
          --output out/from_xml.matter                                   \\
          data_model/clusters/Switch.xml
    """
    logging.basicConfig(
        level=__LOG_LEVELS__[log_level],
        format='%(asctime)s %(levelname)-7s %(message)s',
    )

    if (compare is None) != (compare_output is None):
        logging.error(
            "Either both or none of --compare AND --compare-output must be set")
        sys.exit(1)

    logging.info("Starting to parse ...")

    sources = [ParseSource(source=name) for name in filenames]
    data = ParseXmls(sources)
    logging.info("Parse completed")

    if compare:
        other_idl = CreateParser(skip_meta=True).parse(
            open(compare).read(), file_name=compare)

        # ensure that input file is filtered to only interesting
        # clusters
        loaded_clusters = set([c.code for c in data.clusters])
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


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
