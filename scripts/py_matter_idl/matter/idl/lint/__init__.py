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
import coloredlogs

from matter.idl import matter_idl_parser

from .lint_rules_parser import CreateParser

__all__ = ['CreateParser']

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = logging.getLevelNamesMapping()

log = logging.getLogger(__name__)


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help="Determines the verbosity of script output.",
    show_default=True,
)
@click.option(
    "--rules",
    default=".matterlint",
    type=click.Path(exists=True, dir_okay=False),
    help="Matter lint rules file to use.",
    show_default=True,
)
@click.argument("idl_path", type=click.Path(exists=True, dir_okay=False))
def main(log_level, rules, idl_path):
    """
    Lints MATTER IDL files (.matter) using given RULES
    """
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level],
        fmt="%(asctime)s %(levelname)-7s %(message)s",
    )

    lint_rules = []
    log.info("Loading rules from '%s'", rules)
    with open(rules) as f:
        lint_rules.extend(CreateParser().parse(f.read()))

    log.info("Parsing idl from '%s'", idl_path)
    with open(idl_path) as f:
        idl_tree = matter_idl_parser.CreateParser().parse(f.read(), file_name=idl_path)

    log.info("Running %d lint rules", len(lint_rules))

    errors = []
    for rule in lint_rules:
        log.info("   Running '%s'", rule.name)
        errors.extend(rule.LintIdl(idl_tree))
    log.info("Done")

    for e in errors:
        log.error("ERROR: '%s'", e)

    if errors:
        log.error("Found %d lint errors", len(errors))
        sys.exit(1)


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.argument("filename", type=click.Path(exists=True, dir_okay=False))
def parser(log_level, filename):
    """
    Parse Matter IDL linter RULES file.
    """
    logging.basicConfig(
        level=__LOG_LEVELS__[log_level],
        format='%(asctime)s %(levelname)-7s %(message)s',
    )

    log.info("Starting to parse ...")
    with open(filename) as f:
        data = CreateParser().parse(f.read())
    log.info("Parse completed")

    log.info("Data:")
    log.info("%r", data)
