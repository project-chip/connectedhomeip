#!/usr/bin/env python3
#
#    Copyright (c) 2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    See the License for the specific language governing permissions and
#    limitations under the License.
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#
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

from matter_idl.backwards_compatibility import is_backwards_compatible

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
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help='Determines the verbosity of script output')
@click.argument(
    'old_idl',
    type=click.Path(exists=True))
@click.argument(
    'new_idl',
    type=click.Path(exists=True))
def main(log_level, old_idl, new_idl):
    """
    Parses MATTER IDL files (.matter) and validates that <new_idl> is backwards compatible
    when compared to <old_idl>.

    Generally additions are safe, but not deletes or id changes. Actual set of rules
    defined in `backwards_compatibility` module.
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

    logging.info("Parsing OLD idl from %s" % old_idl)
    old_tree = CreateParser().parse(open(old_idl, "rt").read())

    logging.info("Parsing NEW idl from %s" % new_idl)
    new_tree = CreateParser().parse(open(new_idl, "rt").read())

    if not is_backwards_compatible(original=old_tree, updated=new_tree):
        sys.exit(1)

    sys.exit(0)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
