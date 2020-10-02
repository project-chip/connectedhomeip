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
import getpass
import os
import platform
import shutil
import stat


parser = argparse.ArgumentParser(description='build the pip package for chip using chip components generated during the build and python source code')
parser.add_argument('--package_name', default='chip', help='configure the python package name')
parser.add_argument('--build_number', default='0.0', help='configure the chip build number')

args = parser.parse_args()

chipDLLName = '_ChipDeviceCtrl.so'
deviceManagerShellName = 'chip-device-ctrl'
chipControllerShellInstalledName = os.path.splitext(deviceManagerShellName)[0]
packageName = args.package_name
chipPackageVer = args.build_number

# Record the current directory at the start of execution.
curDir = os.curdir

# Presume that the current directory is the build directory.
buildDir = os.path.dirname(os.path.abspath(__file__))

# Use a temporary directory within the build directory to assemble the components
# for the installable package.
tmpDir = os.path.join(buildDir, 'chip-wheel-components')

try:

    #
    # Perform a series of setup steps prior to creating the chip package...
    #

    # Create the temporary components directory.
    if os.path.isdir(tmpDir):
        shutil.rmtree(tmpDir)
    os.mkdir(tmpDir)

    # Switch to the temporary directory. (Foolishly, setuptools relies on the current directory
    # for many of its features.)
    os.chdir(tmpDir)

    # Make a copy of the chip package in the tmp directory and ensure the copied
    # directory is writable.
    chipPackageDir = os.path.join(tmpDir, packageName)
    if os.path.isdir(chipPackageDir):
        shutil.rmtree(chipPackageDir)
    shutil.copytree(os.path.join(buildDir, 'chip'), chipPackageDir)
    os.chmod(chipPackageDir, os.stat(chipPackageDir).st_mode|stat.S_IWUSR)
    
    # Copy the chip wrapper DLL from where libtool places it (.libs) into
    # the root of the chip package directory.  This is necessary because
    # setuptools will only add package data files that are relative to the
    # associated package source root.
    shutil.copy2(os.path.join(buildDir, 'chip', chipDLLName),
                 os.path.join(chipPackageDir, chipDLLName))
    
    # Make a copy of the Chip Device Controller Shell script in the tmp directory,
    # but without the .py suffix. This is how we want it to appear when installed
    # by the wheel.
    shutil.copy2(os.path.join(buildDir, deviceManagerShellName),
                 os.path.join(tmpDir, chipControllerShellInstalledName))
    
    # Search for the CHIP LICENSE file in the parents of the source
    # directory and make a copy of the file called LICENSE.txt in the tmp
    # directory.  
    def _AllDirsToRoot(dir):
        dir = os.path.abspath(dir)
        while True:
            yield dir
            parent = os.path.dirname(dir)
            if parent == '' or parent == dir:
                break
            dir = parent
    for dir in _AllDirsToRoot(buildDir):
        licFileName = os.path.join(dir, 'LICENSE')
        if os.path.isfile(licFileName):
            shutil.copy2(licFileName,
                         os.path.join(tmpDir, 'LICENSE.txt'))
            break
    else:
        raise FileNotFoundError('Unable to find CHIP LICENSE file')
    
    # Define a custom version of the bdist_wheel command that configures the
    # resultant wheel as platform-specific (i.e. not "pure"). 
    class bdist_wheel_override(bdist_wheel):
        def finalize_options(self):
            bdist_wheel.finalize_options(self)
            self.root_is_pure = False
    
    # Generate a description string with information on how/when the package
    # was built. 

    buildDescription = 'Build by %s on %s\n' % (
                            getpass.getuser(),
                            datetime.now().strftime('%Y/%m/%d %H:%M:%S'))
    
    # Select required packages based on the target system.
    if platform.system() == 'Linux':
        requiredPackages = [
            'dbus-python',
            'pgi'
        ]
    else:
        requiredPackages = []
    
    #
    # Build the chip package...
    #
     
    # Invoke the setuptools 'bdist_wheel' command to generate a wheel containing
    # the CHIP python packages, shared libraries and scripts.
    setup(
        name=packageName,
        version=chipPackageVer,
        description='Python-base APIs and tools for CHIP.',
        long_description=buildDescription,
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
        packages=[
            packageName                     # Arrange to install a package named "chip"
        ],
        package_dir={
            '':tmpDir,                      # By default, look in the tmp directory for packages/modules to be included.
        },
        package_data={
            packageName:[
                chipDLLName                   # Include the wrapper DLL as package data in the "chip" package.
            ]
        },
        scripts=[                           # Install the Device controller Shell as an executable script in the 'bin' directory.
            os.path.join(tmpDir, chipControllerShellInstalledName)
        ],
        install_requires=requiredPackages,
        options={
            'bdist_wheel':{
                'universal':False,
                'dist_dir':buildDir         # Place the generated .whl in the build directory.
            },
            'egg_info':{
                'egg_base':tmpDir           # Place the .egg-info subdirectory in the tmp directory.
            }
        },
        cmdclass={
            'bdist_wheel':bdist_wheel_override
        },
        script_args=[ 'clean', '--all', 'bdist_wheel' ]
    )

finally:
    
    # Switch back to the initial current directory.
    os.chdir(curDir)

    # Remove the temporary directory.
    if os.path.isdir(tmpDir):
        shutil.rmtree(tmpDir)
