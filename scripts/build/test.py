#!/usr/bin/env python3

import click
import coloredlogs
import difflib
import logging
import os
import subprocess
import sys
import time

from typing import List

SCRIPT_ROOT = os.path.dirname(__file__)

def build_expected_output(root: str, out: str) -> List[str]:
  with open(os.path.join(SCRIPT_ROOT, 'expected_all_platform_commands.txt'), 'rt') as f:
    for l in f.readlines():
      yield l.replace("{root}", root).replace("{out}", out)


def build_actual_output(root: str, out: str) -> List[str]:
  # Fake out that we have a project root
  os.environ['PW_PROJECT_ROOT'] = root

  binary = os.path.join(SCRIPT_ROOT, 'build_examples.py')

  retval = subprocess.run([
    binary,
    '--platform', 'all',
    '--log-level', 'FATAL',
    '--dry-run',
    '--repo', root,
    '--out-prefix', out,
    'build'
  ], stdout=subprocess.PIPE, check=True, encoding='UTF-8')


  return [l + '\n' for l in retval.stdout.split('\n')]


def main():
  coloredlogs.install(level=logging.INFO, fmt='%(asctime)s %(name)s %(levelname)-7s %(message)s')

  ROOT = '/TEST/BUILD/ROOT'
  OUT = '/OUTPUT/DIR'

  expected = [l for l in build_expected_output(ROOT, OUT)]
  actual = [l for l in build_actual_output(ROOT, OUT)]

  diffs = [line for line in difflib.unified_diff(expected, actual)]

  if diffs:
    logging.error("DIFFERENCE between expected and generated output")
    for l in diffs:
      logging.warning("  " + l.strip())
    sys.exit(1)



if __name__ == "__main__":
    main()
