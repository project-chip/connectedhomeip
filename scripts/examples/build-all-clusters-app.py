#!/usr/bin/env python3

import argparse
import os
import subprocess
import logging
import coloredlogs

_LOG = logging.getLogger(__name__)

ROOT = 'examples/all-clusters-app/esp32'
idf_path = os.environ['IDF_PATH']

class IDFExecutor:
  """Runs specified commands via an executor that activates the CHIP build environment."""


  def __init__(self):
    script_path = os.path.dirname(os.path.realpath(__file__))
    self.chip_root = os.path.realpath(os.path.join(script_path, '..', '..'))

    logging.info("CHIP Root directory: %s" % self.chip_root)

    self.run_cmd = os.path.join(self.chip_root, "scripts", "run_in_build_env.sh")
    logging.info("Executing via: %s" % self.run_cmd)

  
  def execute(self, command):
    os.chdir(self.chip_root)
    subprocess.call([self.run_cmd, 'source "%s/export.sh"; cd %s; idf.py %s' % (idf_path, ROOT, command)])


def main():
  """Main task if executed standalone."""
  parser = argparse.ArgumentParser(description='Build all-clusters-app example')
  parser.add_argument(
      '--log-level',
      default=logging.INFO,
      type=lambda x: getattr(logging, x),
      help='Configure the logging level.')
  parser.add_argument(
      '--clear-config',
      default=None,
      choices=['m5stack', 'devkit'],
  )
  args = parser.parse_args()

  # Ensures somewhat pretty logging of what is going on
  logging.basicConfig(
      level=args.log_level,
      format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
  coloredlogs.install()

  e = IDFExecutor()

  if args.clear_config:
    logging.info('Building a clear configuration')
    sdkconfig = 'sdkconfig_%s.defaults' % args.clear_config

    curr_sdkconfig = os.path.join(ROOT, 'sdkconfig')
    if os.path.exists(curr_sdkconfig):
      os.remove(curr_sdkconfig)

    e.execute('-D SDKCONFIG_DEFAULTS="{sdkname}" build'.format(
      sdkname=sdkconfig
    ))


  logging.info('Compiling')
  e.execute('build')


if __name__ == '__main__':
  # execute only if run as a script
  main()
