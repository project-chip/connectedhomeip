#!/usr/bin/env python3

import click
import coloredlogs
import difflib
import logging
import os
import subprocess
import sys
import time

# Supported log levels, mapping string values required for argument
# parsing into logging constants
__LOG_LEVELS__ = {
    'debug': logging.DEBUG,
    'info': logging.INFO,
    'warn': logging.WARN,
    'fatal': logging.FATAL,
}

def sameFile(a: str, b: str) -> bool:
  with open(a, 'rt') as fa:
    a_lines = fa.readlines()

  with open(b, 'rt') as fb:
    b_lines = fb.readlines()

  diffs = [line for line in difflib.unified_diff(a_lines, b_lines, fromfile=a, tofile=b)]

  if diffs:
    logging.error("DIFFERENCE found between %s and %s" % (a, b))
    for l in diffs:
      logging.warning("  " + l.strip())

    return False

  return True


@click.command()
@click.option(
    '--log-level',
    default='INFO',
    type=click.Choice(__LOG_LEVELS__.keys(), case_sensitive=False),
    help='Determines the verbosity of script output.')
@click.option("--touch", type=click.Path(resolve_path=True, dir_okay=False), help="Timestamp file to touch.")
@click.option("--repo", type=click.Path(resolve_path=True, dir_okay=True), help="Repository path used when generating.")
@click.option("--output-root", type=click.Path(resolve_path=True, dir_okay=True), help="Build output directory")
@click.option("--expected", type=click.File("rt"), help="Expected file content.")
@click.option("--expected-out", type=click.File("wt"), help="Where to write expected content.")
@click.option("--actual", type=click.Path(resolve_path=True, dir_okay=False), help="Actual file generated content.")
def main(log_level, touch, repo, output_root, expected, expected_out, actual):
  coloredlogs.install(
    level=__LOG_LEVELS__[log_level],
    fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')

  for l in expected.readlines():
    expected_out.write(l.replace("{root}", repo).replace("{out}", output_root))
  expected_out.close()


  logging.info('Diffing %s and %s' % (expected_out.name, actual))
  if not sameFile(expected_out.name, actual):
    sys.exit(1)

  logging.info('Touching %s' % touch)
  os.makedirs(os.path.dirname(touch), exist_ok=True)
  with open(touch, 'wt') as f:
    f.write("Executed at %s" % time.ctime())


if __name__ == "__main__":
    main()
