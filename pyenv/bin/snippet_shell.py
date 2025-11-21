#!/Users/niveditasarkar/workspace/Matter/connectedhomeip/pyenv/bin/python3.11
#
# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Tool to interactively call methods of code snippets.

Mobly Code Snippet Lib (https://github.com/google/mobly-snippet-lib/) is a
library for triggering custom actions on Android devices by means of an RPC
service.

Usage:
$ snippet_shell com.my.package.snippets
>>> s.mySnippet('example')
u'You said: example'
"""

import argparse
import logging
import sys

from mobly.controllers import android_device
from mobly.controllers.android_device_lib import jsonrpc_shell_base


class SnippetShell(jsonrpc_shell_base.JsonRpcShellBase):

  def __init__(self, package):
    self._package = package

  def _start_services(self, console_env):
    """Overrides superclass."""
    self._ad.load_snippet(name='snippet', package=self._package)
    console_env['snippet'] = self._ad.snippet
    console_env['s'] = self._ad.snippet

  def _get_banner(self, serial):
    lines = [
        'Connected to %s.' % serial,
        'Call methods against:',
        '    ad (android_device.AndroidDevice)',
        '    snippet or s (Snippet)',
    ]
    return '\n'.join(lines)


if __name__ == '__main__':
  parser = argparse.ArgumentParser(
      description='Interactive client for Mobly code snippets.'
  )
  parser.add_argument(
      '-s',
      '--serial',
      help='Device serial to connect to (if more than one device is connected)',
  )
  parser.add_argument(
      'package',
      metavar='PACKAGE_NAME',
      type=str,
      nargs='?',
      help='The package name of the snippet to use.',
  )
  parser.add_argument(
      '--mbs',
      help='Whether to connect to Mobly Bundled Snippets',
      action='store_true',
  )
  args = parser.parse_args()
  if args.package and args.mbs:
    print('Cannot specify both --package and --mbs', file=sys.stderr)
    sys.exit(1)
  if args.mbs:
    package = android_device.MBS_PACKAGE
  else:
    package = args.package

  logging.basicConfig(level=logging.INFO)
  SnippetShell(package).main(args.serial)
