#!/usr/bin/env python

import click
import logging
import enum
import os
import sys

try:
    import coloredlogs
    _has_coloredlogs = True
except:
    _has_coloredlogs = False

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
    '--sdk-root',
    default=None,
    help='Path to the SDK root (where .zap/.matter files exist)')
@click.argument('output_dir')
def main(log_level, sdk_root, output_dir):
    if _has_coloredlogs:
        coloredlogs.install(level=__LOG_LEVELS__[
                            log_level], fmt='%(asctime)s %(levelname)-7s %(message)s')
    else:
        logging.basicConfig(
            level=__LOG_LEVELS__[log_level],
            format='%(asctime)s %(levelname)-7s %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )

    if not sdk_root:
        sdk_root = os.path.join(os.path.realpath(__file__), '..')
    sdk_root = os.path.abspath(sdk_root)

    if not output_dir:
        raise Exception("Missing output directory")

    output_dir = os.path.abspath(output_dir)

    logging.info(f"Pre-generating {sdk_root} data into {output_dir}")

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # FIXME: implement

    logging.info("Done")


if __name__ == '__main__':
    main()
