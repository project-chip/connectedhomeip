#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2019 Google LLC.
#    All rights reserved.
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

#
#    Description:
#      Builds a Python wheel package for CHIP.
#

from __future__ import absolute_import
from datetime import datetime
from setuptools import setup
from wheel.bdist_wheel import bdist_wheel

import argparse
import json
import os
import platform
import shutil


parser = argparse.ArgumentParser(
    description='build the pip package for chip using chip components generated during the build and python source code')
parser.add_argument('--package_name', default='chip',
                    help='configure the python package name')
parser.add_argument('--build_number', default='0.0',
                    help='configure the chip build number')
parser.add_argument('--build_dir', help='directory to build in')
parser.add_argument('--dist_dir', help='directory to place distribution in')
parser.add_argument('--manifest', help='list of files to package')
parser.add_argument(
    '--plat-name', help='platform name to embed in generated filenames')

args = parser.parse_args()


class InstalledScriptInfo:
    """Information holder about a script that is to be installed."""

    def __init__(self, name):
        self.name = name
        self.installName = os.path.splitext(name)[0]


chipDLLName = 'PyChip.so'
packageName = args.package_name
chipPackageVer = args.build_number

installScripts = [
    # InstalledScriptInfo('chip-device-ctrl.py'),
    # InstalledScriptInfo('chip-repl.py'),
]

# Record the current directory at the start of execution.
curDir = os.curdir

manifestFile = os.path.abspath(args.manifest)
buildDir = os.path.abspath(args.build_dir)
distDir = os.path.abspath(args.dist_dir)

# Use a temporary directory within the build directory to assemble the components
# for the installable package.
tmpDir = os.path.join(buildDir, 'pychip-wheel-components')

manifest = json.load(open(manifestFile, 'r'))

try:

    #
    # Perform a series of setup steps prior to creating the chip package...
    #

    # Create the temporary components directory.
    if os.path.isdir(tmpDir):
        shutil.rmtree(tmpDir)
    os.makedirs(tmpDir, exist_ok=True)

    # Switch to the temporary directory. (Foolishly, setuptools relies on the current directory
    # for many of its features.)
    os.chdir(tmpDir)

    manifestBase = os.path.dirname(manifestFile)
    for entry in manifest['files']:
        srcDir = os.path.join(manifestBase, entry['src_dir'])
        for path in entry['sources']:
            srcFile = os.path.join(srcDir, path)
            dstFile = os.path.join(tmpDir, path)
            os.makedirs(os.path.dirname(dstFile), exist_ok=True)
            shutil.copyfile(srcFile, dstFile)

    for script in installScripts:
        os.rename(os.path.join(tmpDir, script.name),
                  os.path.join(tmpDir, script.installName))

    # Define a custom version of the bdist_wheel command that configures the
    # resultant wheel as platform-specific (i.e. not "pure").
    class bdist_wheel_override(bdist_wheel):
        def finalize_options(self):
            bdist_wheel.finalize_options(self)
            self.root_is_pure = False

    requiredPackages = [
        "coloredlogs",
        'construct',
        'ipython',
    ]

    if platform.system() == 'Darwin':
        requiredPackages.append('pyobjc-framework-corebluetooth')

    if platform.system() == 'Linux':
        requiredPackages.append('dbus-python==1.2.18')
        requiredPackages.append('pygobject')

    #
    # Build the chip package...
    #
    packages = [
        'pychip',
    ]

    # Invoke the setuptools 'bdist_wheel' command to generate a wheel containing
    # the CHIP python packages, shared libraries and scripts.
    setup(
        name=packageName,
        version=chipPackageVer,
        description='Python-base APIs and tools for CHIP.',
        url='https://github.com/project-chip/connectedhomeip',
        license='Apache',
        classifiers=[
            'Intended Audience :: Developers',
            'License :: OSI Approved :: Apache Software License',
            'Programming Language :: Python :: 2',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3',
        ],
        python_requires='>=2.7',
        packages=packages,
        package_dir={
            # By default, look in the tmp directory for packages/modules to be included.
            '': tmpDir,
        },
        package_data={
            packageName: [
                # Include the wrapper DLL as package data in the "chip" package.
                chipDLLName
            ]
        },
        scripts=[name for name in map(
            lambda script: os.path.join(tmpDir, script.installName),
            installScripts
        )],
        install_requires=requiredPackages,
        options={
            'bdist_wheel': {
                'universal': False,
                # Place the generated .whl in the dist directory.
                'dist_dir': distDir,
                'py_limited_api': 'cp37',
                'plat_name': args.plat_name,
            },
            'egg_info': {
                # Place the .egg-info subdirectory in the tmp directory.
                'egg_base': tmpDir
            }
        },
        cmdclass={
            'bdist_wheel': bdist_wheel_override
        },
        script_args=['clean', '--all', 'bdist_wheel']
    )

finally:

    # Switch back to the initial current directory.
    os.chdir(curDir)

    # Remove the temporary directory.
    if os.path.isdir(tmpDir):
        shutil.rmtree(tmpDir)
