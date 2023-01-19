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
import fcntl
import json
import os
import subprocess
import sys
import tempfile
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from zap_execution import ZapTool


@dataclass
class CmdLineArgs:
    zapFile: str
    zclFile: str
    templateFile: str
    outputDir: str
    runBootstrap: bool
    parallel: bool = True
    prettify_output: bool = True
    version_check: bool = True
    lock_file: Optional[str] = None


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
            path = os.path.abspath(os.path.join(
                os.path.dirname(zapFile), package["path"]))
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
    parser.add_argument('--lock-file', help='serialize zap invocations by using the specified lock file.')
    parser.add_argument('--prettify-output', action='store_true')
    parser.add_argument('--no-prettify-output',
                        action='store_false', dest='prettify_output')
    parser.add_argument('--version-check', action='store_true')
    parser.add_argument('--no-version-check',
                        action='store_false', dest='version_check')
    parser.set_defaults(parallel=True)
    parser.set_defaults(prettify_output=True)
    parser.set_defaults(version_check=True)
    parser.set_defaults(lock_file=None)
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

    return CmdLineArgs(
        zap_file, zcl_file, templates_file, output_dir, args.run_bootstrap,
        parallel=args.parallel,
        prettify_output=args.prettify_output,
        version_check=args.version_check,
        lock_file=args.lock_file,
    )


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


def runGeneration(cmdLineArgs):
    zap_file = cmdLineArgs.zapFile
    zcl_file = cmdLineArgs.zclFile
    templates_file = cmdLineArgs.templateFile
    output_dir = cmdLineArgs.outputDir
    parallel = cmdLineArgs.parallel

    tool = ZapTool()

    if cmdLineArgs.version_check:
        tool.version_check()

    args = ['-z', zcl_file, '-g', templates_file,
            '-i', zap_file, '-o', output_dir]

    if parallel:
        # Parallel-compatible runs will need separate state
        args.append('--tempState')

    tool.run('generate', *args)

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
            # NOTE: clang-format may differ in time. Currently pigweed comes
            #       with clang-format 15. CI may have clang-format-10 installed
            #       on linux.
            #
            #       We generally want consistent formatting, so
            #       at this point attempt to use clang-format 15.
            clang_formats = ['clang-format-15', 'clang-format']
            for clang_format in clang_formats:
                args = [clang_format, '-i']
                args.extend(clangOutputs)
                try:
                    subprocess.check_call(args)
                    err = None
                    print('Formatted using %s (%s)' % (clang_format, subprocess.check_output([clang_format, '--version'])))
                    for outputName in clangOutputs:
                        print('  - %s' % outputName)
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


class LockFileSerializer:
    def __init__(self, path):
        self.lock_file_path = path
        self.lock_file = None

    def __enter__(self):
        if not self.lock_file_path:
            return

        self.lock_file = open(self.lock_file_path, 'wb')
        fcntl.lockf(self.lock_file, fcntl.LOCK_EX)

    def __exit__(self, *args):
        if not self.lock_file:
            return

        fcntl.lockf(self.lock_file, fcntl.LOCK_UN)
        self.lock_file.close()
        self.lock_file = None


def main():
    checkPythonVersion()
    cmdLineArgs = runArgumentsParser()

    with LockFileSerializer(cmdLineArgs.lock_file) as lock:
        if cmdLineArgs.runBootstrap:
            subprocess.check_call(getFilePath("scripts/tools/zap/zap_bootstrap.sh"), shell=True)

        # The maximum memory usage is over 4GB (#15620)
        os.environ["NODE_OPTIONS"] = "--max-old-space-size=8192"

        # `zap-cli` may extract things into a temporary directory. ensure extraction
        # does not conflict.
        with tempfile.TemporaryDirectory(prefix='zap') as temp_dir:
            old_temp = os.environ['TEMP'] if 'TEMP' in os.environ else None
            os.environ['TEMP'] = temp_dir

            runGeneration(cmdLineArgs)

            if old_temp:
                os.environ['TEMP'] = old_temp
            else:
                del os.environ['TEMP']

    if cmdLineArgs.prettify_output:
        prettifiers = [
            runClangPrettifier,
            runJavaPrettifier,
        ]

        for prettifier in prettifiers:
            prettifier(cmdLineArgs.templateFile, cmdLineArgs.outputDir)


if __name__ == '__main__':
    main()
