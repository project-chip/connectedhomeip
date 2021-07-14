#!/usr/bin/env python3

import click
import coloredlogs
import difflib
import logging
import os
import subprocess
import sys
import time

def buildExpected(root: str, out: str):
  with open(os.path.join(os.path.dirname(__file__), 'expected_all_platform_commands.txt'), 'rt') as f:
    for l in f.readlines():
      yield l.replace("{root}", root).replace("{out}", out)


def buildActual(root: str, out: str):
  return []


def main():
  coloredlogs.install(level=logging.INFO, fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')

  # @click.command()
  # @click.option("--touch", type=click.Path(resolve_path=True, dir_okay=False), help="Timestamp file to touch.")
  # @click.option("--repo", type=click.Path(resolve_path=True, dir_okay=True), help="Repository path used when generating.")
  # @click.option("--output-root", type=click.Path(resolve_path=True, dir_okay=True), help="Build output directory")
  # @click.option("--expected", type=click.File("rt"), help="Expected file content.")
  # @click.option("--expected-out", type=click.File("wt"), help="Where to write expected content.")
  # @click.option("--actual", type=click.Path(resolve_path=True, dir_okay=False), help="Actual file generated content.")

  ROOT = '/BUILD/ROOT'
  OUT = '/OUTPUT/DIR'

  expected = [l for l in buildExpected(ROOT, OUT)]
  actual = [l for l in buildActual(ROOT, OUT)]

  diffs = [line for line in difflib.unified_diff(expected, actual)]

  if diffs:
    logging.error("DIFFERENCE between expected and generated output")
    for l in diffs:
      logging.warning("  " + l.strip())

    sys.exit(1)

  logging.info('Touching %s' % touch)
  os.makedirs(os.path.dirname(touch), exist_ok=True)
  with open(touch, 'wt') as f:
    f.write("Executed at %s" % time.ctime())


if __name__ == "__main__":
    main()
