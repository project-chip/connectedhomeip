#!/usr/bin/env python3

import argparse
import os
import subprocess
import logging
import coloredlogs

_LOG = logging.getLogger(__name__)

ROOT = 'examples/wifi-echo/server/esp32'
IDF_PATH = os.getenv('IDF_PATH')


def GetEnvironment(script):
  """Executes the given script and extracts the subprocess environment."""

  prog = subprocess.Popen(
      ['bash', '-c',
       'source "%s" && echo ENV_FOLLOWS && set' % script],
      stdout=subprocess.PIPE,
      stderr=subprocess.PIPE)

  out, err = prog.communicate()

  for l in err.decode('utf8').split('\n'):
    if l:
      logging.error(l)

  env = None
  for l in out.decode('utf8').split('\n'):
    if env is not None:
      logging.debug('Environment: %r', l)
      idx = l.find('=')
      if idx >= 0:
        key, value = l[:idx], l[idx + 1:]
        env[key] = value
    elif l == 'ENV_FOLLOWS':
      env = {}  # starts an envorinment
    else:
      logging.info(l)

  env['CHIP_ROOT'] = os.path.realpath(os.path.curdir)
  env['PW_CHECKOUT_ROOT'] = os.path.join(env['CHIP_ROOT'], 'third_party', 'pigweed',
                                'repo')
  env['CHIP_BUILD_WITH_GN'] = 'y'

  return env


def main():
  """Main task if executed standalone."""
  parser = argparse.ArgumentParser(description='Build wifi example app')
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

  build_env = GetEnvironment(os.path.join(IDF_PATH, 'export.sh'))

  if args.clear_config:
    logging.info('Building a clear configuration')
    build_env[
        'SDK_CONFIG_DEFAULTS'] = 'sdkconfig_%s.defaults' % args.clear_config
    os.remove(os.path.join(ROOT, 'sdkconfig'))
    subprocess.call(
        ['make', '-j%d' % os.cpu_count(), '-C', ROOT, 'defconfig'],
        env=build_env)

  logging.info('Compiling')
  subprocess.call(['make', '-j%d' % os.cpu_count(), '-C', ROOT], env=build_env)


if __name__ == '__main__':
  # execute only if run as a script
  main()
