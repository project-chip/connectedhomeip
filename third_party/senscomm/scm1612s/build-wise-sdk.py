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
import glob
import shutil

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

def main(sdk_dir: str, sdk_config: str, regen_wise_sdk: str):
    should_regen = regen_wise_sdk.lower() == 'true'

    if should_regen:
        script_dir = os.path.abspath(os.path.dirname(__file__))
        wise_sdk_dir = os.path.join(script_dir, 'wise-sdk')

        if os.path.isdir(wise_sdk_dir):
            shutil.rmtree(wise_sdk_dir)

        wise_cust_dirs = sorted(glob.glob(os.path.join(script_dir, 'wise_cust_*')))
        if not wise_cust_dirs:
            raise Exception('No wise_cust_* directory found in %s' % script_dir)

        wise_cust_dir = wise_cust_dirs[0]
        cmd = ['./generate_sdk.sh', 'ayla']
        with subprocess.Popen(cmd, cwd=wise_cust_dir, stdout=subprocess.PIPE) as s:
            outs, errs = s.communicate()
            code = s.wait()
            if code != 0:
                raise Exception('Command %r failed: %d' % (cmd, code))

        generated_sdk_dir = os.path.join(wise_cust_dir, 'wise-sdk-ayla')
        if not os.path.isdir(generated_sdk_dir):
            generated_sdk_dir = os.path.join(script_dir, 'wise-sdk-ayla')
        if not os.path.isdir(generated_sdk_dir):
            raise Exception('Generated wise-sdk-ayla not found in %s or %s' % (wise_cust_dir, script_dir))

        shutil.move(generated_sdk_dir, wise_sdk_dir)

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
    main(sdk_dir=sys.argv[1], sdk_config=sys.argv[2], regen_wise_sdk=sys.argv[3])
