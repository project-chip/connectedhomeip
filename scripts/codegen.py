#!/usr/bin/env python

import click
import logging
import coloredlogs
import enum

try:
    from .idl.matter_idl_parser import CreateParser
    from .idl.generators.java import JavaGenerator
except:
    import os
    import sys
    sys.path.append(os.path.abspath(os.path.dirname(__file__)))

    from idl.matter_idl_parser import CreateParser
    from idl.generators.java import JavaGenerator


class CodeGeneratorTypes(enum.Enum):
    JAVA = enum.auto()

    def CreateGenerator(self, *args, **kargs):
        if self == CodeGeneratorTypes.JAVA:
            return JavaGenerator(*args, **kargs)
        else:
            raise Error("Unknown code generator type")


# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}

__GENERATORS__ = {
    'java': CodeGeneratorTypes.JAVA
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
    type=click.Choice(__GENERATORS__.keys(), case_sensitive=False),
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
@click.argument(
    'idl_path',
    type=click.Path(exists=True))
def main(log_level, generator, output_dir, dry_run, idl_path):
    coloredlogs.install(level=__LOG_LEVELS__[
                        log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')
    logging.info("Parsing idl from %s" % idl_path)
    idl_tree = CreateParser().parse(open(idl_path, "rt").read())

    logging.info("Running code generator %s" % generator)
    generator = __GENERATORS__[generator].CreateGenerator(
        output_dir=output_dir, idl=idl_tree)
    generator.render(dry_run)
    logging.info("Done")


if __name__ == '__main__':
    main()
