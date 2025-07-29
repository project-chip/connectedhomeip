#!/usr/bin/env -S python3 -B

# Copyright (c) 2021 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import json
import logging
import os
import subprocess

#from runner import ShellRunner

#sys.path.append(os.path.abspath(os.path.dirname(__file__)))


# Supported log levels, mapping string values required for argument
# parsing into logging constants
#__LOG_LEVELS__ = {
#    'debug': logging.DEBUG,
#    'info': logging.INFO,
#    'warn': logging.WARN,
#    'fatal': logging.FATAL,
#}

def main(sdk_dir: str, sdk_config: str):
    cmd = ['make', sdk_config]
    sdk = sdk_dir + '/wise-sdk'
    with subprocess.Popen(cmd, cwd=sdk, stdout=subprocess.PIPE) as s:
        outs, errs = s.communicate()
        code = s.wait()
        if code != 0:
            raise Exception('Command %r failed: %d' % (cmd, code))

    cmd = ['make']
    with subprocess.Popen(cmd, cwd=sdk, stdout=subprocess.PIPE) as s:
        code = s.wait()
        if code != 0:
            raise Exception('Command %r failed: %d' % (cmd, code))

if __name__ == '__main__':
    main(sdk_dir=sys.argv[1], sdk_config=sys.argv[2])
