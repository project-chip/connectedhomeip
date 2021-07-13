#!/usr/bin/env python3

import argparse
import os
import shutil
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
      choices=['m5stack', 'devkit', 'curr', 'default'],
  )
  args = parser.parse_args()

  # Ensures somewhat pretty logging of what is going on
  logging.basicConfig(
      level=args.log_level,
      format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
  coloredlogs.install()

  e = IDFExecutor()

  if args.clear_config:
    old_default_sdkconfig = None
    clear_curr = args.clear_config != 'curr'
    logging.info('Building a clear configuration')

    if args.clear_config == "m5stack" or args.clear_config == "devkit":
      desired_sdkconfig_name = 'sdkconfig_%s.defaults' % args.clear_config
      logging.info('Using default' + desired_sdkconfig_name)
      desired_sdkconfig = os.path.join(ROOT, desired_sdkconfig_name)
      default_sdkconfig = os.path.join(ROOT, 'sdkconfig.defaults')
      old_default_sdkconfig = os.path.join(ROOT, 'sdkconfig.defaults.old')
      shutil.copy(default_sdkconfig, old_default_sdkconfig)
      shutil.copy(desired_sdkconfig, default_sdkconfig)

    if clear_curr:
      logging.info('Clearing current config')
      sdkconfig = os.path.join(ROOT, 'sdkconfig')
      os.remove(sdkconfig)

    e.execute('menuconfig')

    if old_default_sdkconfig is not None:
      shutil.move(old_default_sdkconfig, default_sdkconfig)

  logging.info('Compiling')
  e.execute('build')


if __name__ == '__main__':
  # execute only if run as a script
  main()
