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

import logging
import os
import sys

import click
import coloredlogs

try:
    from matter_idl import matter_idl_parser
except ImportError:
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), 'py_matter_idl')))
    from matter_idl import matter_idl_parser

# isort: off
import matter_idl.lint


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help="Determines the verbosity of script output",
)
@click.option(
    "--rules",
    default=os.path.abspath(os.path.join(os.path.dirname(__file__), "rules.matterlint")),
    type=click.Path(exists=True),
    help="Rules file to use",
)
@click.argument("idl_path", type=click.Path(exists=True))
def main(log_level, rules, idl_path):
    """
    Lints MATTER IDL files (.matter) using given RULES
    """
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )

    lint_rules = []
    logging.info("Loading rules from %s" % rules)
    lint_rules.extend(matter_idl.lint.CreateParser(rules).parse())

    logging.info("Parsing idl from %s" % idl_path)
    idl_tree = matter_idl_parser.CreateParser().parse(open(idl_path, "rt").read(), file_name=idl_path)

    logging.info("Running %d lint rules" % len(lint_rules))

    errors = []
    for rule in lint_rules:
        logging.info("   Running %s" % rule.name)
        errors.extend(rule.LintIdl(idl_tree))
    logging.info("Done")

    for e in errors:
        logging.error("ERROR: %s" % e)

    if errors:
        logging.error("Found %d lint errors" % len(errors))
        sys.exit(1)


if __name__ == "__main__":
    main(auto_envvar_prefix='CHIP')
