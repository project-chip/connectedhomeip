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
from pathlib import Path
import tempfile
import subprocess
import sys
import urllib.request
from dataclasses import dataclass


@dataclass
class CmdLineArgs:
    zapFile: str
    zclFile: str
    templateFile: str
    outputDir: str
    runBootstrap: bool
    parallel: bool = True


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


def detectZclFile(zapFile):
    print(f"Searching for zcl file from {zapFile}")

    path = 'src/app/zap-templates/zcl/zcl.json'

    data = json.load(open(zapFile))
    for package in data["package"]:
        if package["type"] != "zcl-properties":
            continue

        # found the right path, try to figure out the actual path
        if package["pathRelativity"] == "relativeToZap":
            path = os.path.abspath(os.path.join(os.path.dirname(zapFile), package["path"]))
        else:
            path = package["path"]

    return getFilePath(path)


def runArgumentsParser() -> CmdLineArgs:
    default_templates = 'src/app/zap-templates/app-templates.json'
    default_output_dir = 'zap-generated/'

    parser = argparse.ArgumentParser(
        description='Generate artifacts from .zapt templates')
    parser.add_argument('zap', help='Path to the application .zap file')
    parser.add_argument('-t', '--templates', default=default_templates,
                        help='Path to the .zapt templates records to use for generating artifacts (default: "' + default_templates + '")')
    parser.add_argument('-z', '--zcl',
                        help='Path to the zcl templates records to use for generating artifacts (default: autodetect read from zap file)')
    parser.add_argument('-o', '--output-dir', default=None,
                        help='Output directory for the generated files (default: automatically selected)')
    parser.add_argument('--run-bootstrap', default=None, action='store_true',
                        help='Automatically run ZAP bootstrap. By default the bootstrap is not triggered')
    parser.add_argument('--parallel', action='store_true')
    parser.add_argument('--no-parallel', action='store_false', dest='parallel')
    parser.set_defaults(parallel=True)
    args = parser.parse_args()

    # By default, this script assumes that the global CHIP template is used with
    # a default 'zap-generated/' output folder relative to APP_ROOT_DIR.
    # If needed, the user may specify a specific template as a second argument. In
    # this case the output folder is relative to CHIP_ROOT_DIR.
    if args.output_dir:
        output_dir = args.output_dir
    elif args.templates == default_templates:
        output_dir = os.path.join(Path(args.zap).parent, default_output_dir)
    else:
        output_dir = ''

    zap_file = getFilePath(args.zap)

    if args.zcl:
        zcl_file = getFilePath(args.zcl)
    else:
        zcl_file = detectZclFile(zap_file)

    templates_file = getFilePath(args.templates)
    output_dir = getDirPath(output_dir)

    return CmdLineArgs(zap_file, zcl_file, templates_file, output_dir, args.run_bootstrap)


def extractGeneratedIdl(output_dir, zap_config_path):
    """Find a file Clusters.matter in the output directory and
       place it along with the input zap file.

       Intent is to make the "zap content" more humanly understandable.
    """
    idl_path = os.path.join(output_dir, "Clusters.matter")
    if not os.path.exists(idl_path):
        return

    target_path = zap_config_path.replace(".zap", ".matter")
    if not target_path.endswith(".matter"):
        # We expect "something.zap" and don't handle corner cases of
        # multiple extensions. This is to work with existing codebase only
        raise Error("Unexpected input zap file  %s" % self.zap_config)

    os.rename(idl_path, target_path)


def runGeneration(zap_file, zcl_file, templates_file, output_dir):
    # Accepted environment variables, in order:
    #
    # ZAP_DEVELOPMENT_PATH - the path to a zap development environment. This is
    #                        a zap checkout, used for local development
    # ZAP_INSTALL_PATH     - the path where zap-cli exists. This is if zap-cli
    #                        is NOT in the current path

    if 'ZAP_DEVELOPMENT_PATH' in os.environ:
        generate_cmd = ['node', 'src-script/zap-start.js', 'generate']
        working_directory = os.environ['ZAP_DEVELOPMENT_PATH']
    elif 'ZAP_INSTALL_PATH' in os.environ:
        generate_cmd = [os.path.join(os.environ['ZAP_INSTALL_PATH'], 'zap-cli'), 'generate']
        working_directory = None
    else:
        generate_cmd = ['zap-cli', 'generate']
        working_directory = None

    try:
        subprocess.check_call(generate_cmd + ['-z', zcl_file, '-g', templates_file,
                              '-i', zap_file, '-o', output_dir], cwd=working_directory)
    except FileNotFoundError as e:
        print(f'FAILED TO EXECUTE ZAP GENERATION: {e.strerror} - "{e.filename}"')
        print('*'*80)
        print('* You may need to install zap. Please ensure one of these applies:')
        print('* - `zap-cli` is in $PATH. Install from https://github.com/project-chip/zap/releases')
        print('*   see docs/guides/BUILDING.md for details')
        print('* - `zap-cli` is in $ZAP_INSTALL_PATH. Use this option if you')
        print('*   installed zap but do not want to update $PATH')
        print('* - Point $ZAP_DEVELOPMENT_PATH to your local copy of zap that you')
        print('*   develop on (to use a developer build of zap)')
        print('*'*80)
        sys.exit(1)

    extractGeneratedIdl(output_dir, zap_file)


def runClangPrettifier(templates_file, output_dir):
    listOfSupportedFileExtensions = [
        '.js', '.h', '.c', '.hpp', '.cpp', '.m', '.mm']

    try:
        jsonData = json.loads(Path(templates_file).read_text())
        outputs = [(os.path.join(output_dir, template['output']))
                   for template in jsonData['templates']]
        clangOutputs = list(filter(lambda filepath: os.path.splitext(
            filepath)[1] in listOfSupportedFileExtensions, outputs))

        if len(clangOutputs) > 0:
            # The "clang-format" pigweed comes with is now version 14, which
            # changed behavior from version 13 and earlier regarding some
            # whitespace formatting.  Unfortunately, all the CI bits run
            # clang-format 13 or earlier, so we get styling mismatches.
            #
            # Try some older clang-format versions just in case they are
            # installed.  In particular, clang-format-13 is available on various
            # Linux distributions and clang-format-11 is available via homebrew
            # on Mac.  If all else fails, fall back to clang-format.
            clang_formats = ['clang-format-13', 'clang-format-12', 'clang-format-11', 'clang-format']
            for clang_format in clang_formats:
                args = [clang_format, '-i']
                args.extend(clangOutputs)
                try:
                    subprocess.check_call(args)
                    err = None
                    break
                except Exception as thrown:
                    err = thrown
                    # Press on to the next binary name
            if err is not None:
                raise err
    except Exception as err:
        print('clang-format error:', err)


def runJavaPrettifier(templates_file, output_dir):
    try:
        jsonData = json.loads(Path(templates_file).read_text())
        outputs = [(os.path.join(output_dir, template['output']))
                   for template in jsonData['templates']]
        javaOutputs = list(
            filter(lambda filepath: os.path.splitext(filepath)[1] == ".java", outputs))

        if len(javaOutputs) > 0:
            # Keep this version in sync with what restyler uses (https://github.com/project-chip/connectedhomeip/blob/master/.restyled.yaml).
            google_java_format_version = "1.6"
            google_java_format_url = 'https://github.com/google/google-java-format/releases/download/google-java-format-' + \
                google_java_format_version + '/'
            google_java_format_jar = 'google-java-format-' + \
                google_java_format_version + '-all-deps.jar'
            jar_url = google_java_format_url + google_java_format_jar

            home = str(Path.home())
            path, http_message = urllib.request.urlretrieve(
                jar_url, home + '/' + google_java_format_jar)
            args = ['java', '-jar', path, '--replace']
            args.extend(javaOutputs)
            subprocess.check_call(args)
    except Exception as err:
        print('google-java-format error:', err)


def main():
    checkPythonVersion()
    cmdLineArgs = runArgumentsParser()

    if cmdLineArgs.runBootstrap:
        subprocess.check_call(getFilePath("scripts/tools/zap/zap_bootstrap.sh"), shell=True)

    # The maximum memory usage is over 4GB (#15620)
    os.environ["NODE_OPTIONS"] = "--max-old-space-size=8192"

    if cmdLineArgs.parallel:
        # Parallel-compatible runs will need separate state
        os.environ["ZAP_TEMPSTATE"] = "1"

    # `zap-cli` may extract things into a temporary directory. ensure extraction
    # does not conflict.
    with tempfile.TemporaryDirectory(prefix='zap') as temp_dir:
        old_temp = os.environ['TEMP'] if 'TEMP' in os.environ else None
        os.environ['TEMP'] = temp_dir

        runGeneration(cmdLineArgs.zapFile, cmdLineArgs.zclFile, cmdLineArgs.templateFile, cmdLineArgs.outputDir)

        if old_temp:
            os.environ['TEMP'] = old_temp
        else:
            del os.environ['TEMP']

    prettifiers = [
        runClangPrettifier,
        runJavaPrettifier,
    ]

    for prettifier in prettifiers:
        prettifier(cmdLineArgs.templateFile, cmdLineArgs.outputDir)


if __name__ == '__main__':
    main()
