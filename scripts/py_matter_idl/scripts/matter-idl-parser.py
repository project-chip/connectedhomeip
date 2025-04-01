#!/usr/bin/env python

# The IDL parser is generally not intended to be run as a stand-alone binary.
# The ability to run is for debug and to print out the parsed AST.

import logging
import pprint
import sys
from pathlib import Path

import click

try:
    from matter import idl
except ImportError:
    sys.path.append(str(Path(__file__).resolve().parent / ".." / "matter"))
    import idl


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
    help='Determines the verbosity of script output.')
@click.argument('filename')
def main(log_level, filename=None):
    logging.basicConfig(
        level=__LOG_LEVELS__[log_level],
        format='%(asctime)s %(levelname)-7s %(message)s',
    )

    logging.info("Starting to parse ...")
    data = idl.CreateParser().parse(open(filename).read(), file_name=filename)
    logging.info("Parse completed")

    logging.info("Data:")
    pprint.pp(data)


if __name__ == '__main__':
    main(auto_envvar_prefix='CHIP')
