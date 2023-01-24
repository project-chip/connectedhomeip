#!/usr/bin/env python3
#
#    Copyright (c) 2021 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import argparse
import json
import os
import subprocess
import sys

from zap_execution import ZapTool

CHIP_ROOT_DIR = os.path.realpath(
    os.path.join(os.path.dirname(__file__), '../../..'))


def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' +
              str(sys.version_info[0]))
        exit(1)


def checkFileExists(path):
    if not os.path.isfile(path):
        print('Error: ' + path + ' does not exists or is not a file.')
        exit(1)


def checkDirExists(path):
    if not os.path.isdir(path):
        print('Error: ' + path + ' does not exists or is not a directory.')
        exit(1)


def getFilePath(name):
    fullpath = os.path.join(CHIP_ROOT_DIR, name)
    checkFileExists(fullpath)
    return fullpath


def getDirPath(name):
    fullpath = os.path.join(CHIP_ROOT_DIR, name)
    checkDirExists(fullpath)
    return fullpath


def runArgumentsParser():
    parser = argparse.ArgumentParser(
        description='Convert .zap files to the current zap version')
    parser.add_argument('zap', help='Path to the application .zap file')
    parser.add_argument('--run-bootstrap', default=None, action='store_true',
                        help='Automatically run ZAP bootstrap. By default the bootstrap is not triggered')
    args = parser.parse_args()

    zap_file = getFilePath(args.zap)

    return zap_file, args.run_bootstrap


def detectZclFile(zapFile):
    print(f"Searching for zcl file from {zapFile}")

    path = 'src/app/zap-templates/zcl/zcl.json'

    data = json.load(open(zapFile))
    for package in data["package"]:
        if package["type"] != "zcl-properties":
            continue

        # found the right path, try to figure out the actual path
        if package["pathRelativity"] == "relativeToZap":
            path = os.path.abspath(os.path.join(
                os.path.dirname(zapFile), package["path"]))
        else:
            path = package["path"]

    return getFilePath(path)


def runConversion(zap_file):
    templates_file = getFilePath('src/app/zap-templates/app-templates.json')
    zcl_file = detectZclFile(zap_file)
    tool = ZapTool()
    tool.run('convert', '-z', zcl_file, '-g',
             templates_file, '-o', zap_file, zap_file)


def main():
    checkPythonVersion()
    zap_file, run_bootstrap = runArgumentsParser()

    if run_bootstrap:
        subprocess.check_call(getFilePath(
            "scripts/tools/zap/zap_bootstrap.sh"), shell=True)

    os.chdir(CHIP_ROOT_DIR)

    runConversion(zap_file)


if __name__ == '__main__':
    main()
