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
import subprocess
import sys
import urllib.request

CHIP_ROOT_DIR = os.path.realpath(os.path.join(os.path.dirname(__file__), '../../..'))

def checkPythonVersion():
    if sys.version_info[0] < 3:
        print('Must use Python 3. Current version is ' + str(sys.version_info[0]))
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
    default_templates = 'src/app/zap-templates/app-templates.json'
    default_zcl = 'src/app/zap-templates/zcl/zcl.json'
    default_output_dir = 'gen/'

    parser = argparse.ArgumentParser(description='Generate artifacts from .zapt templates')
    parser.add_argument('zap', help='Path to the application .zap file')
    parser.add_argument('-t', '--templates', default=default_templates, help='Path to the .zapt templates records to use for generating artifacts (default: "' + default_templates + '")')
    parser.add_argument('-z', '--zcl', default=default_zcl, help='Path to the zcl templates records to use for generating artifacts (default: "' + default_zcl + '")')
    args = parser.parse_args()

    # By default, this script assumes that the global CHIP template is used with
    # a default 'gen/' output folder relative to APP_ROOT_DIR.
    # If needed, the user may specify a specific template as a second argument. In
    # this case the output folder is relative to CHIP_ROOT_DIR.
    if args.templates == default_templates:
        output_dir = os.path.join(Path(args.zap).parent, default_output_dir)
    else:
        output_dir = ''

    zap_file = getFilePath(args.zap)
    zcl_file = getFilePath(args.zcl)
    templates_file = getFilePath(args.templates)
    output_dir = getDirPath(output_dir)

    return (zap_file, zcl_file, templates_file, output_dir)

def runGeneration(zap_file, zcl_file, templates_file, output_dir):
    generator_dir = getDirPath('third_party/zap/repo')
    os.chdir(generator_dir)
    subprocess.check_call(['node', './src-script/zap-generate.js', '-z', zcl_file, '-g', templates_file, '-i', zap_file, '-o', output_dir])

def runClangPrettifier(templates_file, output_dir):
    listOfSupportedFileExtensions = ['.js', '.h', '.c', '.hpp', '.cpp', '.m', '.mm']

    try:
        jsonData = json.loads(Path(templates_file).read_text())
        outputs = [(os.path.join(output_dir, template['output'])) for template in jsonData['templates']]
        clangOutputs = list(filter(lambda filepath: os.path.splitext(filepath)[1] in listOfSupportedFileExtensions, outputs))

        if len(clangOutputs) > 0:
            args = ['clang-format', '-i']
            args.extend(clangOutputs)
            subprocess.check_call(args)
    except Exception as err:
        print('clang-format error:', err)

def runJavaPrettifier(templates_file, output_dir):
    try:
        jsonData = json.loads(Path(templates_file).read_text())
        outputs = [(os.path.join(output_dir, template['output'])) for template in jsonData['templates']]
        javaOutputs = list(filter(lambda filepath: os.path.splitext(filepath)[1] == ".java", outputs))

        if len(javaOutputs) > 0:
            # Keep this version in sync with what restyler uses (https://github.com/project-chip/connectedhomeip/blob/master/.restyled.yaml).
            google_java_format_version = "1.6"
            google_java_format_url = 'https://github.com/google/google-java-format/releases/download/google-java-format-' + google_java_format_version + '/'
            google_java_format_jar = 'google-java-format-' + google_java_format_version + '-all-deps.jar'
            jar_url = google_java_format_url + google_java_format_jar
            
            home = str(Path.home())
            path, http_message = urllib.request.urlretrieve(jar_url, home + '/' + google_java_format_jar)
            args = ['java', '-jar', path, '--replace']
            args.extend(javaOutputs)
            subprocess.check_call(args)
    except Exception as err:
        print('google-java-format error:', err)

def main():
    checkPythonVersion()

    zap_file, zcl_file, templates_file, output_dir = runArgumentsParser()
    runGeneration(zap_file, zcl_file, templates_file, output_dir)
    runClangPrettifier(templates_file, output_dir)
    runJavaPrettifier(templates_file, output_dir)

if __name__ == '__main__':
    main()
